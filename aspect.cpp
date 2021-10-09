#include "aspect.h"
#include "board.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

static const QString ASPLIB_MARK { "AspectLib" };

int Aspect::getMoveCount_dif()
{
    int count { 0 };
    for (const auto& pair : fen_rc_rec_.values())
        count += pair.first.count() + pair.second.count();
    return count;
}

int Aspect::getMoveCount_total()
{
    int count { 0 };
    for (const auto& pair : fen_rc_rec_.values())
        for (const auto& hash : { pair.first, pair.second })
            for (const auto& rec : hash.values())
                count += rec.count;

    return count;
}

RC_RecordHash& Aspect::getRC_RecordHash(const QString& fen, PieceColor color)
{
    // fen统一成红底
    bool redBottom = PieceManager::redIsBottom(fen);
    auto& pair = getRc_RecordHashPair__(redBottom ? fen : Board::changeFEN(fen, ChangeType::EXCHANGE));
    if (!redBottom)
        color = PieceManager::getOtherColor(color);
    return color == PieceColor::RED ? pair.first : pair.second;
}

Record& Aspect::getRecord(const QString& fen, PieceColor color, int rowcols)
{
    auto& hash = getRC_RecordHash(fen, color);
    auto iter = hash.find(rowcols);
    if (iter != hash.end())
        return iter.value();

    return hash.insert(rowcols, Record {}).value();
}

void Aspect::putMoveRec(MoveRec& moveRec)
{
    // fen统一成红底
    if (!PieceManager::redIsBottom(moveRec.fen)) {
        moveRec.fen = Board::changeFEN(moveRec.fen, ChangeType::EXCHANGE);
        moveRec.color = PieceManager::getOtherColor(moveRec.color);
        //moveRec.fen = Board::changeFEN(moveRec.fen, ChangeType::ROTATE);
        //moveRec.rowcols = Instance::changeRowcols(moveRec.rowcols, ChangeType::ROTATE);
    }
    getRecord(moveRec.fen, moveRec.color, moveRec.rowcols) = moveRec.record;
}

void Aspect::readRecFormatFile(const QString& fileName)
{
    Instance ins(fileName);
    for (auto& moveRec : ins.getMoveReces())
        putMoveRec(moveRec);
}

static void readRecFormatFile__(const QString& fileName, void* asp)
{
    ((Aspect*)asp)->readRecFormatFile(fileName);
}

void Aspect::readRecFormatDir(const QString& dirName)
{
    Tools::operateDir(dirName, readRecFormatFile__, this, true);
}

void Aspect::readAspectFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return;

    QTextStream stream(&file);
    QString mark {};
    int count {};
    stream >> mark >> count;
    assert(mark == ASPLIB_MARK);

    QString fen {};
    int rowcols {}, rec_count {}, weight {},
        killing {}, willKill {}, isCatch {}, isFail {};
    while (!stream.atEnd()) {
        stream >> fen;
        for (int i = 0; i < 2; ++i) {
            stream >> count;
            for (int j = 0; j < count; ++j) {
                stream >> rowcols >> rec_count
                    >> weight >> killing >> willKill >> isCatch >> isFail;
                MoveRec moveRec(fen, i == 0 ? PieceColor::RED : PieceColor::BLACK, rowcols,
                    Record(weight, killing, willKill, isCatch, isFail));
                putMoveRec(moveRec);
            }
        }
    }

    file.close();
}

void Aspect::writeAspectFile(const QString& fileName)
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return;

    QTextStream stream(&file);
    stream << ASPLIB_MARK << ' ' << fen_rc_rec_.count() << '\n';
    for (auto iter = fen_rc_rec_.constKeyValueBegin(); iter != fen_rc_rec_.constKeyValueEnd(); ++iter) {
        stream << (*iter).first << ' '; // fen
        const auto& pair = (*iter).second;
        for (const auto& mrhash : { pair.first, pair.second }) {
            stream << mrhash.count() << ' '; // count
            for (auto recIter = mrhash.constKeyValueBegin();
                 recIter != mrhash.constKeyValueEnd(); ++recIter) {
                stream << (*recIter).first << ' '; // rowcols
                const auto& rec = (*recIter).second; // record
                stream << rec.count << ' ' << rec.weight << ' ' << (int)rec.killing
                       << ' ' << (int)rec.willKill << ' ' << (int)rec.isCatch << ' ' << (int)rec.isFail << ' ';
            }
        }
        stream << '\n';
    }

    file.close();
}

void Aspect::readAspectFile_bin(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return;

    QDataStream stream(&file);
    stream >> fen_rc_rec_;
    file.close();
}

void Aspect::writeAspectFile_bin(const QString& fileName)
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return;

    QDataStream stream(&file);
    stream << fen_rc_rec_;
    file.close();
}

QString Aspect::toString()
{
    QString qstr {};
    QTextStream stream(&qstr);
    stream << "Fen: " << getFenCount() << QString { "个. Move_diff: " } << getMoveCount_dif()
           << QString { "个. Move_total: " } << getMoveCount_total() << QString { "个.\n" };
    return qstr;
}

RC_RecordHashPair& Aspect::getRc_RecordHashPair__(const QString& fen)
{
    auto pairIter = fen_rc_rec_.find(fen);
    if (pairIter != fen_rc_rec_.end())
        return pairIter.value();

    QString fen_sym { Board::changeFEN(fen, ChangeType::SYMMETRY) };
    pairIter = fen_rc_rec_.find(fen_sym);
    if (pairIter != fen_rc_rec_.end()) {
        // 找到左右对称的局面
        auto& pair = pairIter.value();
        RC_RecordHash rc_RecHash[2] {}, old_rc_RecHash[] = { pair.first, pair.second };
        for (int i = 0; i < 2; ++i)
            // 着法转换
            for (auto iter = old_rc_RecHash[i].begin(); iter != old_rc_RecHash[i].end(); ++iter)
                rc_RecHash[i].insert(Instance::changeRowcols(iter.key(), ChangeType::SYMMETRY), iter.value());

        // 将原对称局面下的着法替换为现局面下的着法
        fen_rc_rec_.erase(pairIter);
        return fen_rc_rec_.insert(fen, RC_RecordHashPair { rc_RecHash[0], rc_RecHash[1] }).value();
    }

    // 插入空着法列表
    return fen_rc_rec_.insert(fen, RC_RecordHashPair {}).value();
}

bool testAspect()
{
    QString aspFileName { "asp.txt" },
        aspFileName1 { "out_" + aspFileName },
        aspFileName2 { "bin_" + aspFileName };
    Aspect asp {};
    asp.readRecFormatFile("01.xqf");
    /*
    QList<QString> dirfroms {
        "chessManual/示例文件.xqf",
        //"chessManual/象棋杀着大全.xqf",
        //"chessManual/疑难文件.xqf",
        //"chessManual/中国象棋棋谱大全.xqf"
    };
    for (auto& dirName : dirfroms)
        asp.readRecFormatDir(dirName);
    //*/
    //asp.writeAspectFile(aspFileName);
    //asp.readAspectFile(aspFileName);

    //asp.writeAspectFile(aspFileName1);
    asp.writeAspectFile_bin(aspFileName2);

    Aspect asp2 {};
    asp2.readAspectFile_bin(aspFileName2);
    asp2.writeAspectFile(aspFileName1);

    Tools::writeTxtFile(aspFileName1, asp.toString(), QIODevice::Append);
    Tools::writeTxtFile(aspFileName1, asp2.toString(), QIODevice::Append);
    return true;
}
