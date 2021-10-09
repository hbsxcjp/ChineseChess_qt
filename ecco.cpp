#include "ecco.h"
#include "instance.h"
#include "seat.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QThread>

enum RecordIndex {
    SN_I,
    NAME_I,
    PRE_MVSTRS_I,
    MVSTRS_I,
    NUMS_I,
    REGSTR_I
};

#define MOVESTR_LEN 4

using BoutStrs = QMap<QChar, QList<QString>>;

// '-','／': 不前进，加‘|’   '*': 前进，加‘|’    '+': 前进，不加‘|’
constexpr QChar Forward { '+' }, ForwardOr { '*' },
    NotForwardOr { '-' }, NotForwardOr_alias { L'／' }, ExceptNotForwardOr { L'除' }, BlankChar { ' ' };

Ecco::Ecco(const QString& libFileName)
{
    QString libStr { Tools::readTxtFile(libFileName) };
    //setEcco_someField__(libStr);
    //setEcco_regstrField__();
}

Ecco::Ecco(const QString& dbName, const QString& tblName)
{
    //setRegList_db__(dbName, tblName);
    dbName + tblName;
}

bool Ecco::setECCO(PInstance& ins) const
{
    ins->go();
    return true;
}

bool Ecco::setECCO(QList<PInstance> insList) const
{
    for (auto& pins : insList)
        setECCO(pins);

    return true;
}

InitEcco::~InitEcco()
{
    if (dowHtml_)
        delete dowHtml_;
}

void InitEcco::startInitEccoLibToDB(const QString& dbName, const QString& tblName)
{
    dbName_ = dbName;
    tblName_ = tblName;
    Tools::writeTxtFile("eccolib.txt", "", QIODevice::WriteOnly);
    if (!dowHtml_)
        dowHtml_ = new DownHtml("GB18030");
    qDebug() << "InitEcco::initEccoLibToDB()" << this->thread() << dowHtml_->thread();

    if (bool(con_))
        disconnect(con_);
    con_ = connect(dowHtml_, &DownHtml::readyRead, this, &InitEcco::handleEccoLibDownStr);
    QString qstr {}, url_c { "https://www.xqbase.com/ecco/ecco_" };
    for (auto& c : QString { "abcde" })
        dowHtml_->startDowning(url_c + c + ".htm");
}

void InitEcco::startDowningInstanceToDB(const QString& dbName, const QString& tblName, int first, int last)
{
    dbName_ = dbName;
    tblName_ = tblName;
    Tools::writeTxtFile("instance.txt", "", QIODevice::WriteOnly);
    if (!dowHtml_)
        dowHtml_ = new DownHtml("GB18030");
    qDebug() << "InitEcco::downInstanceToDB()" << this->thread() << dowHtml_->thread();

    if (bool(con_))
        disconnect(con_);
    con_ = connect(dowHtml_, &DownHtml::readyRead, this, &InitEcco::handleInstanceDownStr);

    QString url_d { "https://www.xqbase.com/xqbase/?gameid=" };
    for (int id = first; id <= last; ++id)
        dowHtml_->startDowning(url_d + QString::number(id));
}

void InitEcco::handleEccoLibDownStr(const QString& downedStr)
{
    qDebug() << "InitEcco::handleEccoLibDownStr()";
    // 解析字符串，清洁内容, 存入数据库
    QString cleanDownStr { downedStr };
    cleanDownStr.remove(QRegularExpression("</?(?:div|font|img|strong|center|meta|dl|dt|table|tr|td|em|p|li|dir"
                                           "|html|head|body|title|a|b|tbody|script|br|span)[^>]*>",
        QRegularExpression::UseUnicodePropertiesOption));
    cleanDownStr.replace(QRegularExpression(R"((\n|\r)\s+)", QRegularExpression::UseUnicodePropertiesOption), " ");

    Tools::writeTxtFile("eccolib.txt", cleanDownStr, QIODevice::Append);

    emit eccoLibFinished();
}

void InitEcco::handleInstanceDownStr(const QString& downedStr)
{
    qDebug() << "InitEcco::handleInstanceDownStr()";
    if (downedStr.isEmpty())
        return;

    // 解析字符串，存入数据库
    QMap<QString, QString> regStr = {
        { "\\<title\\>(.*)\\</title\\>", "TITLE" },
        { "\\>([^\\>]+赛[^\\>]*)\\<", "EVENT" },
        { "\\>黑方 ([^\\<]*)\\<", "BLACK" },
        { "\\>红方 ([^\\<]*)\\<", "RED" },
        { "\\>(\\d+年\\d+月(?:\\d+日)?)(?: ([^\\<]*))?\\<", "DATE" }, // SITE_INDEX 3
        { "\\>([A-E]\\d{2})\\. ([^\\<]*)\\<", "ECCOSN" }, // ECCONAME_INDEX 16
        { "\\<pre\\>\\s*(1\\.[\\s\\S]*?)\\((.+)\\)\\</pre\\>", "MOVESTR" } // RESULT_INDEX 10
    };
    InfoMap info { Instance::getInitInfoMap() };
    info["SOURCE"] = downedStr.left(downedStr.indexOf(QChar('\n')));
    for (auto& key : regStr.keys()) {
        QRegularExpression reg(key, QRegularExpression::UseUnicodePropertiesOption);
        auto match = reg.match(downedStr);
        if (!match.hasMatch())
            continue;

        QString value { match.captured(1) };
        QString infoKey { regStr[key] };
        if (infoKey == "MOVESTR")
            value.replace("\r\n", " ");
        info[infoKey] = value;
        QString infoKey2 { (infoKey == "DATE" ? "SITE"
                                              : (infoKey == "ECCOSN"           ? "ECCONAME"
                                                      : (infoKey == "MOVESTR") ? "RESULT"
                                                                               : "")) };
        if (!infoKey2.isEmpty()) {
            info[infoKey2] = match.captured(2);
        }
    }

    //Tools::writeTxtFile("test.txt", downedStr, QIODevice::Append);
    QString qstr {};
    QTextStream stream(&qstr);
    for (auto& key : info.keys())
        stream << key << ':' << info[key] << '\n';
    stream << '\n';
    Tools::writeTxtFile("instance.txt", qstr, QIODevice::Append);

    emit instanceFinished();
}

// 将着法描述组合成着法字符串列表（“此前...”，“...不分先后”）
static void insertBoutStr__(BoutStrs& boutStrs, QChar boutNo, int color, QString mvstrs,
    QRegularExpression reg_m, QRegularExpression reg_bp)
{
    if (boutStrs.find(boutNo) == boutStrs.end())
        boutStrs.insert(boutNo, QList<QString> { {}, {} });

    QString mvstr_result {};
    // 处理一串着法字符串内存在先后顺序的着法
    auto match = reg_bp.match(mvstrs);
    for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
        QString mvstr { match.captured(i) };
        if (mvstr.isEmpty())
            continue;

        // C83 可选着法"车二进六"不能加入顺序着法，因为需要回退，以便解决与后续“炮８进２”的冲突
        if (mvstr == "车二进六"
            && mvstrs.contains("红此前可走马二进三、马八进七、车一平二、车二进六、兵三进一和兵七进一"))
            continue;

        // B22~B24, D42,D52~D55 "马八进七"需要前进，又需要加"|"
        mvstr_result.append((mvstr == "马八进七" ? ForwardOr : Forward) + mvstr);
        mvstrs.replace(match.capturedStart(i), mvstr.length(), QString(mvstr.length(), BlankChar));
    }

    // 处理连续描述的着法
    auto matchIter = reg_m.globalMatch(mvstrs);
    while (matchIter.hasNext()) {
        auto match = matchIter.next();
        mvstr_result.append(NotForwardOr + match.captured(1));
    }
    // B30 "除...以外的着法"
    if (mvstrs.contains(ExceptNotForwardOr))
        mvstr_result.append(ExceptNotForwardOr);
    boutStrs[boutNo][color] = mvstr_result;
}

// 获取回合着法字符串数组
static void setBoutStrs__(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isBefore,
    QRegularExpression reg_m, QRegularExpression reg_bm, QRegularExpression reg_bp)
{
    auto matchIter = reg_bm.globalMatch(mvstrs);
    while (matchIter.hasNext()) {
        auto match = matchIter.next();
        QChar boutNo = mvstrs[match.capturedStart(1)];
        if (isBefore && boutNo.isLower())
            boutNo = boutNo.toUpper();

        for (int i = 2; i <= match.lastCapturedIndex(); ++i) {
            QString qstr { match.captured(i) };
            if (qstr.isEmpty())
                continue;

            int color = i / 5;
            qstr.replace(NotForwardOr_alias, NotForwardOr);
            if ((i == 2 || i == 5) && qstr != "……") { // 回合的着法
                // D41 该部分字段的着法与前置部分重复
                if (color == 1 && !isBefore && sn == "D41")
                    continue;
                // B32 第一、二着黑棋的顺序有误
                if (color == 1 && boutNo < '3' && sn == "B32")
                    qstr = (boutNo == '1') ? "炮８平６" : "马８进７";
                // B21 棋子文字错误
                else if (qstr == "象七进九")
                    qstr[0] = L'相';

                if (boutStrs.find(boutNo) != boutStrs.end()) {
                    auto& mvstr = boutStrs[boutNo][color];
                    if (!mvstr.contains(qstr)) {
                        if (!mvstr.isEmpty())
                            mvstr[0] = NotForwardOr;
                        mvstr.append((mvstr.isEmpty() ? Forward : NotForwardOr) + qstr);
                    }
                } else {
                    QList<QString> mvstrs { {}, {} };
                    mvstrs[color] = Forward + qstr;
                    boutStrs.insert(boutNo, mvstrs);
                }
            } else if (i == 3 || i == 6) { // "不分先后" i=3, 6
                for (auto bNo : boutStrs.keys()) {
                    if (bNo > boutNo)
                        break;

                    for (auto& mvstrs : boutStrs[bNo])
                        if (mvstrs.length() == MOVESTR_LEN + 1)
                            mvstrs[0] = ForwardOr;
                }
            } else if (i == 4 || i == 7) { // "此前..."
                QChar insertBoutNo = boutNo.unicode() - 'n' + 'g'; // 往前7个字符
                if (sn == "D41" && !isBefore)
                    color = 0; // D41 转移至前面颜色的字段
                if (sn == "D29") {
                    // D29 先处理红棋着法 (此前红可走马八进七，黑可走马２进３、车９平４)
                    insertBoutStr__(boutStrs, insertBoutNo, 0, "此前红可走马八进七", reg_m, reg_bp);
                    insertBoutStr__(boutStrs, insertBoutNo, 1, "黑可走马２进３、车９平４", reg_m, reg_bp);
                }

                // 处理"此前..."着法描述字符串
                insertBoutStr__(boutStrs, insertBoutNo, color, qstr, reg_m, reg_bp);
            }
        }
    }
}

static QString getIccses__(const QString& mvstrs, Instance& ins)
{
    QString iccses {};
    int count { 0 }, groupCount { 1 }; // 分组数
    for (int i = 0; i < mvstrs.length() - 1; i += MOVESTR_LEN + 1) {
        QString mvstr { mvstrs.mid(i, MOVESTR_LEN + 1) };
        // '-','／': 不前进，加‘|’   '*': 前进，加‘|’    '+': 前进，不加‘|’
        bool isOther = mvstr[0] == NotForwardOr,
             split = isOther || mvstr[0] == ForwardOr,
             // 添加着法，并前进至此着法
            succes = ins.appendMove_zh_tolerateError(mvstr.mid(1), isOther);
        if (succes) {
            ++count;
            if (split && i > 0) {
                if (groupCount > 0)
                    iccses.append('|');
                ++groupCount;
            }
            iccses.append(ins.getCurMoveIccs());
            if (isOther)
                ins.backOther();
        } else {
            --groupCount;
            Tools::writeTxtFile("boutStrs.txt", "\t\t失败:" + mvstr + '\n', QIODevice::Append);
        }
    }
    if (mvstrs.back() == ExceptNotForwardOr)
        iccses = "(?:(?!" + iccses + ").*)"; // 否定顺序环视 见《精通正则表达式》P66.
    else {
        if (count > 1)
            iccses = "(?:" + iccses + ")";
        if (groupCount > 1)
            iccses.append((mvstrs[0] == NotForwardOr ? "{1," : "{") + QString::number(groupCount) + "}");
    }

    Tools::writeTxtFile("boutStrs.txt", '\t' + mvstrs + ' ' + iccses + "\n", QIODevice::Append);
    //fwprintf(test_out, L"\n\tmvstrs:%ls %ls{%d}", mvstrs, iccses, groupCount);
    return iccses;
}

// 只考虑一种基本情形. 某局面配对开局正则表达式时，转换成红底且左右对称两种情形进行匹配
static QString getRegStr__(const BoutStrs& boutStrs)
{
    Instance ins {};
    // C11~C14=>"车一平四／车一平六" D40~D43=>"车８进５", 做备份以便后续单独处理
    QList<QString> regStr { {}, {} }, mvstrs_bak { "-车一平四-车一平六", "-车８进５" };
    for (int color = 0; color < PieceManager::pieceColorNum; ++color) {
        bool handleMvstrs_bak = false;
        for (auto boutNo : boutStrs.keys()) {
            auto& mvstrs = boutStrs[boutNo][color];
            if (mvstrs.isEmpty())
                continue;

            // 处理“此前...”着法(每次单独处理) C11~C14=>"车一平四" D40~D43=>"车８进５"
            if (handleMvstrs_bak) {
                regStr[color].append(getIccses__(mvstrs_bak[color], ins));
                //if (boutNo < 'n') // C11 bout==62时不回退
                //  ins.backToPre(); // 回退至前着，避免重复走最后一着
            } else if (mvstrs.contains(mvstrs_bak[color]))
                handleMvstrs_bak = true;

            regStr[color].append(getIccses__(mvstrs, ins));
        }
    }

    return "(?:^" + regStr[0] + ".*?&" + regStr[1] + ')';
}

static void setEccoRegstrField__(QMap<QString, QStringList>& records)
{
    QString ZhWChars { PieceManager::getZhChars() },
        // 着法中文字符组
        mvstr { "([" + ZhWChars + "]{4})" },
        // 着法，含多选的复式着法
        mvStrs { "(?:[" + ZhWChars + "]{4}(?:／[" + ZhWChars + "]{4})*)" },
        // 捕捉一步着法: 1.着法，2.可能的“此前...”着法，3. 可能的“／\\n...$”着法
        rich_mvStr { "(" + mvStrs + "|……)(?:[，、；\\s　和\\(\\)以／]|$)(\\(?不.先后[\\)，])?([^" + ZhWChars + "]*?此前[^\\)]+?\\))?" },
        // 捕捉一个回合着法：1.序号，2.一步着法的首着，3-5.着法或“此前”，4-6.着法或“此前”或“／\\n...$”
        bout_rich_mvStr { "([\\da-z]).[ \\n\\r\\s]?" + rich_mvStr + "(?:" + rich_mvStr + ")?" };

    QRegularExpression regs[] = {
        QRegularExpression(mvstr, QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression(bout_rich_mvStr, QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression("红方：(.+)\\s+黑方：(.+)", QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression("(炮二平五)?.(马二进三).*(车一平二).(车二进六)?"
                           "|(马８进７).+(车９平８)"
                           "|此前可走(马二进三)、(马八进七)、(兵三进一)和(兵七进一)",
            QRegularExpression::UseUnicodePropertiesOption)
    };

    int no {};
    Tools::writeTxtFile("boutStrs.txt", "", QIODevice::WriteOnly);
    for (auto& record : records) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.isEmpty())
            continue;

        BoutStrs boutStrs {};
        QString pre_mvstrs { record[PRE_MVSTRS_I] };
        if (!pre_mvstrs.isEmpty()) {
            auto match = regs[2].match(pre_mvstrs);
            if (match.hasMatch()) {
                // 处理前置着法描述字符串————"局面开始：红方：黑方："
                insertBoutStr__(boutStrs, '1', 0, match.captured(1), regs[0], regs[3]);
                insertBoutStr__(boutStrs, '1', 1, match.captured(2), regs[0], regs[3]);
            } else
                setBoutStrs__(boutStrs, sn, pre_mvstrs, true, regs[0], regs[1], regs[3]);
        }

        setBoutStrs__(boutStrs, sn, mvstrs, false, regs[0], regs[1], regs[3]);

        //*
        QString qstr {};
        QTextStream stream(&qstr);
        stream << "\nNo." << no++ << "\n";
        for (auto& str : record)
            stream << str << ' ';
        stream << "\n";
        for (auto boutNo : boutStrs.keys()) {
            stream << boutNo << '.';
            for (auto& mvstrs : boutStrs[boutNo]) {
                stream << mvstrs << ' ';
            }
            stream << '\n';
        }
        Tools::writeTxtFile("boutStrs.txt", qstr, QIODevice::Append);
        //*/

        record[REGSTR_I] = getRegStr__(boutStrs);
        Tools::writeTxtFile("boutStrs.txt", record[REGSTR_I] + "\n", QIODevice::Append);
    }
}

void InitEcco::setEccoLibField()
{
    QMap<QString, QStringList> records {};

    // 根据正则表达式分解字符串，获取字段内容
    QList<QString> regStr = {
        // field: sn name nums
        R"(([A-E])．(\S+?)\((共[\s\S]+?局)\))",
        // field: sn name nums mvstr B2. C0. D1. D2. D3. D4. \\s不包含"　"(全角空格)
        R"(([A-E]\d)(?:．|\. )(?:空|([\S^\r]+?)\((共[\s\S]+?局)\)\s+([\s\S]*?))(?=[\s　]*[A-E]\d0．))",
        // field: sn name mvstr nums
        R"(([A-E]\d{2})．(\S+)[\s　]+(?:(?![A-E]\d|上一)([\s\S]*?)[\s　]*(无|共[\s\S]+?局)[\s\S]*?(?=上|[A-E]\d{0,2}．))?)",
        // field: sn mvstr C20 C30 C61 C72局面字符串
        R"(([A-E]\d)\d局面 =([\s\S]*?)(?=[\s　]*[A-E]\d{2}．))"
    };

    QString cleanDownStr { Tools::readTxtFile("eccolib.txt") };
    for (int g = 0; g < regStr.length(); ++g) {
        QRegularExpression reg(regStr[g], QRegularExpression::UseUnicodePropertiesOption);
        auto matchIter = reg.globalMatch(cleanDownStr);
        while (matchIter.hasNext()) {
            auto match = matchIter.next();
            auto capTexts = match.capturedTexts();
            if (g < 3) {
                QStringList record { "", "", "", "", "", "" };
                for (int f = 0; f < capTexts.length() - 1; ++f) {
                    int index = ((g == 2) ? (f > 1 ? f + 1 : f)
                                          : (f > 1 ? (f == 2 ? NUMS_I : MVSTRS_I) : f));
                    record[index] = capTexts[f + 1];
                }
                records[capTexts[1]] = record;
            } else
                // C20 C30 C61 C72局面字符串存至g=1数组, 以便设置前置着法字符串
                records[capTexts[1]][MVSTRS_I] = capTexts[2];
        }
    }

    // 设置pre_mvstrs字段, 前置省略内容 有40+75=115项
    //int no = 1;
    for (auto& record : records) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.length() < 3)
            continue;

        QString sn_pre {};
        // 三级局面的 C2 C3_C4 C61 C72局面 有40项
        if (mvstrs[0] == L'从') {
            //continue;
            sn_pre = mvstrs.mid(1, 2);
            // 前置省略的着法 有75项
        } else if (mvstrs[0] != L'1') {
            //continue;
            // 截断为两个字符长度
            sn_pre = sn.left(2);
            if (sn_pre[0] == L'C')
                sn_pre[1] = L'0'; // C0/C1/C5/C6/C7/C8/C9 => C0
            else if (sn_pre == "D5")
                sn_pre = "D51"; // D5 => D51
        } else
            continue;

        record[PRE_MVSTRS_I] = records[sn_pre][MVSTRS_I];
        //qDebug() << no++ << sn << record[PRE_MVSTRS_I] << record[MVSTRS_I];
    }

    //*
    QString qstr {};
    QTextStream stream(&qstr);
    int no2 = 0;
    for (auto& record : records) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.isEmpty())
            continue;

        stream << no2++ << '.';
        for (auto& str : record)
            if (!str.isEmpty())
                stream << str << '\n';
        stream << "\n";
    }
    // 已与原始网页，核对完全一致！
    Tools::writeTxtFile("records.txt", qstr, QIODevice::WriteOnly);
    //*/

    qDebug() << "setEccoLibField: " << records.size();

    setEccoRegstrField__(records);
}

QList<PInstance> InitEcco::getInsList_dir__(const QString& dirName)
{
    QList<PInstance> insList;
    dirName.at(0);
    return insList;
}

QList<PInstance> InitEcco::getInsList_webfile__(const QString& insFileName)
{
    QList<PInstance> insList;
    insFileName.at(0);
    return insList;
}

QList<PInstance> InitEcco::getInsList_db__(const QString& dbName, const QString& man_tblName)
{
    QList<PInstance> insList;
    dbName.at(0);
    man_tblName.at(0);
    return insList;
}

int InitEcco::storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName)
{
    insList.end();
    dbName.at(0);
    tblName.at(0);
    return 0;
}
