#include "board.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <QTextStream>
#include <functional>

Board::Board()
    : bottomColor_ { PieceColor::RED }
{
    // "KAABBNNRRCCPPPPPkaabbnnrrccppppp"
    int kindNum[PieceManager::pieceKindNum] = { 1, 2, 2, 2, 2, 2, 5 };
    for (int c = 0; c < PieceManager::pieceColorNum; ++c)
        for (int k = 0; k < PieceManager::pieceKindNum; ++k) {
            QChar ch = PieceManager::chChars.at(c * PieceManager::pieceKindNum + k);
            int num = kindNum[k];
            switch (k) {
            case 0:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new King(ch));
                break;
            case 1:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new Advisor(ch));
                break;
            case 2:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new Bishop(ch));
                break;
            case 3:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new Knight(ch));
                break;
            case 4:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new Rook(ch));
                break;
            case 5:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new Cannon(ch));
                break;
            default:
                for (int i = 0; i < num; ++i)
                    pieces_.append(new Pawn(ch));
                break;
            }
        }

    QList<QPair<int, int>> allRowcols = SeatManager::getAllRowcols();
    for (auto& rowcol : allRowcols)
        seats_.append(new Seat(rowcol.first, rowcol.second));
}

Board::~Board()
{
    for (auto& seat : seats_)
        delete seat;

    for (auto& piece : pieces_)
        delete piece;
}

const PSeat& Board::getSeat(const int row, const int col) const
{
    return seats_.at(SeatManager::getIndex(row, col));
}

const PSeat& Board::getSeat(const int rowcol) const
{
    return seats_.at(SeatManager::getIndex(rowcol));
}

const PSeat& Board::getSeat(const QPair<int, int>& rowcol) const
{
    return getSeat(rowcol.first, rowcol.second);
}

const PSeat& Board::getOtherSeat(const PSeat& seat, ChangeType ct) const
{
    return getSeat(SeatManager::getChangeRowcol(seat->rowcol(), ct));
}

BoardStatus Board::status(PieceColor color)
{
    bool isTrue = isFail(color);
    if (isTrue)
        return BoardStatus::FAIL;

    isTrue = isKilling(color);
    if (isTrue)
        return BoardStatus::KILLING;

    isTrue = isWillKill(color);
    if (isTrue)
        return BoardStatus::WILLKILL;

    isTrue = isCatch(color);
    if (isTrue)
        return BoardStatus::CATCH;

    return BoardStatus::NODANGE;
}

bool Board::isFail(PieceColor color) const
{
    for (auto& fseat : getLiveSeats__(color))
        if (!fseat->getMoveSeats(*this).empty())
            return false;

    return true;
}

bool Board::isKilled(PieceColor color) const
{
    return isFace__() || isKilling(color);
}

bool Board::isKilling(PieceColor color) const
{
    PieceColor othColor { PieceManager::getOtherColor(color) };
    PSeat kingSeat { getKingSeat__(color) };
    // '获取某方可杀将棋子全部可走的位置
    for (auto& seat : getLiveSeats__(othColor, SeatManager::nullName(), SeatManager::nullCol(), true)) {
        auto mvSeats = seat->piece()->moveSeats(*this, *seat);
        if (!mvSeats.empty() && mvSeats.indexOf(kingSeat) != -1)
            return true;
    }

    return false;
}

bool Board::isWillKill(PieceColor color)
{
    //return isContinuousKilling__();
    return color == PieceColor::RED;
}

bool Board::isCatch(PieceColor color)
{
    /*
    if (isProtectedForce__(board, fseat, tseat))
        return false;

    return true;
    //*/
    return color == PieceColor::RED;
}

bool Board::isClout(PieceColor color)
{
    return isKilling(color) || isWillKill(color) || isCatch(color);
}

bool Board::isIdle(PieceColor color)
{
    return !isClout(color);
}

bool Board::isCanMove(PSeat& fseat, PSeat& tseat)
{
    return fseat->getMoveSeats(*this).indexOf(tseat) != -1;
}

void Board::reset(const QString& fen)
{
    auto pieceChars = FENTopieChars(fen);
    assert(seats_.size() == pieceChars.length());
    auto boardPieces = getBoardPieces__(pieceChars);
    reset__(boardPieces);
}

void Board::changeSide(ChangeType ct)
{
    QList<PPiece> boardPieces {};
    for (const auto& seat : seats_)
        boardPieces.append(ct == ChangeType::EXCHANGE
                ? (seat->piece() ? getOtherPiece__(seat->piece()) : seat->piece())
                : getOtherSeat(seat, ct)->piece());
    reset__(boardPieces);
}

QPair<PSeat, PSeat> Board::fromToSeats(const QString& zhStr, bool tolerateError) const
{
    assert(zhStr.size() == 4);
    PSeat fseat {}, tseat {};
    QList<PSeat> seats {};

    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color { PieceManager::getColorFromZh(zhStr.back()) };
    bool isBottom { isBottomSide(color) };
    int index {}, movDir { PieceManager::getMovNum(isBottom, zhStr.at(2)) };
    QChar name { zhStr.front() };

    if (PieceManager::isPiece(name)) { // 首字符为棋子名
        seats = getLiveSeats__(color, name,
            PieceManager::getCol(isBottom, PieceManager::getNum(color, zhStr.at(1))));

        if (tolerateError && seats.size() == 0)
            return QPair<PSeat, PSeat>(fseat, tseat);
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = (PieceManager::isPawn(name)
                ? getSortPawnLiveSeats__(isBottom, color, name)
                : getLiveSeats__(color, name));
        if (tolerateError && seats.size() == 0)
            return QPair<PSeat, PSeat>(fseat, tseat);
        assert(seats.size() > 0);
        index = PieceManager::getIndex(seats.size(), isBottom, zhStr.front());
    }

    assert(index <= seats.length() - 1);
    fseat = seats.at(index);
    int num { PieceManager::getNum(color, zhStr.back()) },
        toCol { PieceManager::getCol(isBottom, num) };
    if (PieceManager::isLineMove(name)) {
        int trow { fseat->row() + movDir * num };
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(trow, fseat->col());
    } else { // 斜线走子：仕、相、马
        int colAway { abs(toCol - fseat->col()) }, //  相距1或2列
            trow { fseat->row() + movDir * (PieceManager::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)) };
        tseat = getSeat(trow, toCol);
    }
    //assert(zhStr == getZh(fseat, tseat));

    return QPair<PSeat, PSeat>(fseat, tseat);
}

//(fseat, tseat)->中文纵线着法
QString Board::pgn_zhStr(const PSeat& fseat, const PSeat& tseat) const
{
    QString qstr {};
    const PPiece& fromPiece { fseat->piece() };
    assert(fromPiece);

    const PieceColor color { fromPiece->color() };
    const QChar name { fromPiece->name() };
    const int fromRow { fseat->row() },
        fromCol { fseat->col() },
        toRow { tseat->row() },
        toCol { tseat->col() };
    bool isSameRow { fromRow == toRow },
        isBottom { isBottomSide(color) };
    auto seats = getLiveSeats__(color, name, fromCol);

    if (seats.size() > 1 && PieceManager::isStronge(name)) {
        if (PieceManager::isPawn(name))
            seats = getSortPawnLiveSeats__(isBottom, color, name);
        qstr.append(PieceManager::getIndexChar(seats.size(), isBottom, seats.indexOf(fseat)))
            .append(name);
    } else { //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        qstr.append(name)
            .append(PieceManager::getColChar(color, isBottom, fromCol));
    }
    qstr.append(PieceManager::getMovChar(isSameRow, isBottom, toRow > fromRow))
        .append(PieceManager::isLineMove(name) && !isSameRow
                ? PieceManager::getNumChar(color, abs(fromRow - toRow))
                : PieceManager::getColChar(color, isBottom, toCol));

    auto mvSeats = fromToSeats(qstr);
    assert(fseat == mvSeats.first && tseat == mvSeats.second);

    return qstr;
}

static constexpr QChar FENSplitChar_ { '/' };

QString Board::pieCharsToFEN(const QString& pieceChars)
{
    assert(pieceChars.length() == SeatManager::seatsNum());
    QString fen {};
    for (int index = 0; index < pieceChars.length(); index += 9) {
        QString line { pieceChars.mid(index, 9) }, qstr {};
        int num { 0 };
        for (auto ch : line) {
            if (ch != PieceManager::nullChar()) {
                if (num != 0) {
                    qstr.append(QString::number(num));
                    num = 0;
                }
                qstr.append(ch);
            } else
                ++num;
        }
        if (num != 0)
            qstr.append(QString::number(num));
        fen.prepend(qstr).prepend(FENSplitChar_);
    }
    fen.remove(0, 1);

    //assert(FENTopieChars(fen) == pieceChars);
    return fen;
}

QString Board::FENTopieChars(const QString& fen)
{
    QString pieceChars {};
    QStringList strList { fen.split(FENSplitChar_) };
    for (auto& line : strList) {
        QString qstr {};
        for (auto ch : line)
            if (ch.isDigit())
                qstr.append(QString(ch.digitValue(), PieceManager::nullChar()));
            else
                qstr.append(ch);
        pieceChars.prepend(qstr);
    }

    assert(fen == pieCharsToFEN(pieceChars));
    return pieceChars;
}

QString Board::changeFEN(const QString& fen, ChangeType ct)
{
    if (ct == ChangeType::NOCHANGE)
        return fen;

    QString fen_ct {};
    if (ct == ChangeType::EXCHANGE) {
        for (int i = 0; i < fen.length(); ++i) {
            auto ch = fen[i];
            fen_ct.append(PieceManager::getOtherChar(ch));
        }
    } else if (ct == ChangeType::ROTATE) {
        for (int i = fen.length() - 1; i >= 0; --i)
            fen_ct.append(fen[i]);
    } else if (ct == ChangeType::SYMMETRY) {
        QStringList strList = fen.split(FENSplitChar_);
        for (auto& line : strList) {
            for (int i = line.length() - 1; i >= 0; --i)
                fen_ct.append(line[i]);
            fen_ct.append(FENSplitChar_);
        }
        fen_ct.remove(fen_ct.length() - 1, 1); // 去掉最后一个多余的分隔符
    }
    return fen_ct;
}

QString Board::getFEN() const
{
    QString qstr {};
    for (const auto& seat : seats_) {
        const auto& piece = seat->piece();
        qstr.append(piece ? piece->ch() : PieceManager::nullChar());
    }
    return pieCharsToFEN(qstr);
}

QString Board::getFEN_ct(ChangeType ct) const
{
    return changeFEN(getFEN(), ct);
}

QString Board::toString() const
{ // 文本空棋盘
    QString textBlankBoard { "┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
                             "┃　│　│　│╲│╱│　│　│　┃\n"
                             "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                             "┃　│　│　│╱│╲│　│　│　┃\n"
                             "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                             "┃　│　│　│　│　│　│　│　┃\n"
                             "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                             "┃　│　│　│　│　│　│　│　┃\n"
                             "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
                             "┃　　　　　　　　　　　　　　　┃\n"
                             "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
                             "┃　│　│　│　│　│　│　│　┃\n"
                             "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                             "┃　│　│　│　│　│　│　│　┃\n"
                             "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                             "┃　│　│　│╲│╱│　│　│　┃\n"
                             "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                             "┃　│　│　│╱│╲│　│　│　┃\n"
                             "┗━┷━┷━┷━┷━┷━┷━┷━┛\n" }; // 边框粗线

    for (auto color : { PieceColor::BLACK, PieceColor::RED })
        for (auto& seat : getLiveSeats__(color))
            textBlankBoard[(SeatManager::ColNum() - seat->row())
                    * 2 * (SeatManager::ColNum() * 2)
                + seat->col() * 2]
                = PieceManager::getPrintName(*seat->piece());
    return textBlankBoard;
}

QString Board::toFullString() const
{
    // 棋盘上下边标识字符串
    const QString PRESTR[] = {
        "　　　　　　　黑　方　　　　　　　\n１　２　３　４　５　６　７　８　９\n",
        "　　　　　　　红　方　　　　　　　\n一　二　三　四　五　六　七　八　九\n"
    };
    const QString SUFSTR[] = {
        "九　八　七　六　五　四　三　二　一\n　　　　　　　红　方　　　　　　　\n",
        "９　８　７　６　５　４　３　２　１\n　　　　　　　黑　方　　　　　　　\n"
    };

    int index = bottomColor_ == PieceColor::RED ? 0 : 1;
    return PRESTR[index] + toString() + SUFSTR[index];
}

bool Board::test()
{
    QString qstr0 { "红帅K 红仕A 红仕A 红相B 红相B 红马N 红马N "
                    "红车R 红车R 红炮C 红炮C 红兵P 红兵P 红兵P 红兵P 红兵P "
                    "黑将k 黑士a 黑士a 黑象b 黑象b 黑馬n 黑馬n "
                    "黑車r 黑車r 黑砲c 黑砲c 黑卒p 黑卒p 黑卒p 黑卒p 黑卒p " },
        qstr1 {};
    for (auto& piece : pieces_)
        qstr1.append(piece->toString()).append(' ');
    assert(qstr0 == qstr1);

    QList<QString> fen {
        PieceManager::getFENStr(),
        "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
    },
        pieceChars { "RNBAKABNR__________C_____C_P_P_P_P_P__________________p_p_p_p_p_c_____c__________rnbakabnr",
            "___c_________K____B____N__B___________________________________p______R______ak__r_____a___", "", "" },
        boardString { "　　　　　　　黑　方　　　　　　　\n"
                      "１　２　３　４　５　６　７　８　９\n"
                      "車━馬━象━士━将━士━象━馬━車\n"
                      "┃　│　│　│╲│╱│　│　│　┃\n"
                      "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                      "┃　│　│　│╱│╲│　│　│　┃\n"
                      "┠─砲─┼─┼─┼─┼─┼─砲─┨\n"
                      "┃　│　│　│　│　│　│　│　┃\n"
                      "卒─┼─卒─┼─卒─┼─卒─┼─卒\n"
                      "┃　│　│　│　│　│　│　│　┃\n"
                      "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
                      "┃　　　　　　　　　　　　　　　┃\n"
                      "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
                      "┃　│　│　│　│　│　│　│　┃\n"
                      "兵─┼─兵─┼─兵─┼─兵─┼─兵\n"
                      "┃　│　│　│　│　│　│　│　┃\n"
                      "┠─炮─┼─┼─┼─┼─┼─炮─┨\n"
                      "┃　│　│　│╲│╱│　│　│　┃\n"
                      "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                      "┃　│　│　│╱│╲│　│　│　┃\n"
                      "车━马━相━仕━帅━仕━相━马━车\n"
                      "九　八　七　六　五　四　三　二　一\n"
                      "　　　　　　　红　方　　　　　　　\n"
                      "04帅",
            "　　　　　　　黑　方　　　　　　　\n"
            "１　２　３　４　５　６　７　８　９\n"
            "┏━┯━┯━┯━┯━士━┯━┯━┓\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─士─将─┼─┼─車\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┠─╬─┼─┼─┼─┼─车─╬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─卒\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "相─╬─┼─┼─┼─马─┼─╬─相\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─帅─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┗━┷━┷━砲━┷━┷━┷━┷━┛\n"
            "九　八　七　六　五　四　三　二　一\n"
            "　　　　　　　红　方　　　　　　　\n"
            "14帅" },
        boardChangeStr = { "　　　　　　　红　方　　　　　　　\n"
                           "一　二　三　四　五　六　七　八　九\n"
                           "┏━┯━┯━┯━┯━仕━┯━┯━┓\n"
                           "┃　│　│　│╲│╱│　│　│　┃\n"
                           "┠─┼─┼─┼─仕─帅─┼─┼─车\n"
                           "┃　│　│　│╱│╲│　│　│　┃\n"
                           "┠─╬─┼─┼─┼─┼─車─╬─┨\n"
                           "┃　│　│　│　│　│　│　│　┃\n"
                           "┠─┼─╬─┼─╬─┼─╬─┼─兵\n"
                           "┃　│　│　│　│　│　│　│　┃\n"
                           "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
                           "┃　　　　　　　　　　　　　　　┃\n"
                           "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
                           "┃　│　│　│　│　│　│　│　┃\n"
                           "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                           "┃　│　│　│　│　│　│　│　┃\n"
                           "象─╬─┼─┼─┼─馬─┼─╬─象\n"
                           "┃　│　│　│╲│╱│　│　│　┃\n"
                           "┠─┼─┼─┼─将─┼─┼─┼─┨\n"
                           "┃　│　│　│╱│╲│　│　│　┃\n"
                           "┗━┷━┷━炮━┷━┷━┷━┷━┛\n"
                           "９　８　７　６　５　４　３　２　１\n"
                           "　　　　　　　黑　方　　　　　　　\n"
                           "14将",
            "　　　　　　　黑　方　　　　　　　\n"
            "１　２　３　４　５　６　７　８　９\n"
            "┏━┯━┯━┯━┯━炮━┯━┯━┓\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─将─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "象─╬─┼─馬─┼─┼─┼─╬─象\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "兵─┼─╬─┼─╬─┼─╬─┼─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─╬─車─┼─┼─┼─┼─╬─┨\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "车─┼─┼─帅─仕─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┗━┷━┷━仕━┷━┷━┷━┷━┛\n"
            "九　八　七　六　五　四　三　二　一\n"
            "　　　　　　　红　方　　　　　　　\n"
            "13帅",
            "　　　　　　　黑　方　　　　　　　\n"
            "１　２　３　４　５　６　７　８　９\n"
            "┏━┯━┯━炮━┯━┯━┯━┯━┓\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─将─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "象─╬─┼─┼─┼─馬─┼─╬─象\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─兵\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─╬─┼─┼─┼─┼─車─╬─┨\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─仕─帅─┼─┼─车\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┗━┷━┷━┷━┷━仕━┷━┷━┛\n"
            "九　八　七　六　五　四　三　二　一\n"
            "　　　　　　　红　方　　　　　　　\n"
            "15帅" };

    QMap<PieceColor, QString> seatStr[] {
        { { PieceColor::RED, "16个 00车 01马 02相 03仕 04帅 05仕 06相 07马 08车 21炮 27炮 30兵 32兵 34兵 36兵 38兵 " },
            { PieceColor::BLACK, "16个 60卒 62卒 64卒 66卒 68卒 71砲 77砲 90車 91馬 92象 93士 94将 95士 96象 97馬 98車 " } },
        { { PieceColor::RED, "5个 14帅 20相 25马 28相 76车 " },
            { PieceColor::BLACK, "6个 03砲 68卒 84士 85将 88車 95士 " } }
    },
        moveSeatStr[] { { { PieceColor::RED, "00车=> 2个 10_ 20_ \n"
                                             "01马=> 2个 20_ 22_ \n"
                                             "02相=> 2个 20_ 24_ \n"
                                             "03仕=> 1个 14_ \n"
                                             "04帅=> 1个 14_ \n"
                                             "05仕=> 1个 14_ \n"
                                             "06相=> 2个 24_ 28_ \n"
                                             "07马=> 2个 26_ 28_ \n"
                                             "08车=> 2个 18_ 28_ \n"
                                             "21炮=> 12个 20_ 22_ 23_ 24_ 25_ 26_ 11_ 31_ 41_ 51_ 61_ 91馬 \n"
                                             "27炮=> 12个 26_ 25_ 24_ 23_ 22_ 28_ 17_ 37_ 47_ 57_ 67_ 97馬 \n"
                                             "30兵=> 1个 40_ \n"
                                             "32兵=> 1个 42_ \n"
                                             "34兵=> 1个 44_ \n"
                                             "36兵=> 1个 46_ \n"
                                             "38兵=> 1个 48_ \n" },
                            { PieceColor::BLACK, "60卒=> 1个 50_ \n"
                                                 "62卒=> 1个 52_ \n"
                                                 "64卒=> 1个 54_ \n"
                                                 "66卒=> 1个 56_ \n"
                                                 "68卒=> 1个 58_ \n"
                                                 "71砲=> 12个 70_ 72_ 73_ 74_ 75_ 76_ 61_ 51_ 41_ 31_ 01马 81_ \n"
                                                 "77砲=> 12个 76_ 75_ 74_ 73_ 72_ 78_ 67_ 57_ 47_ 37_ 07马 87_ \n"
                                                 "90車=> 2个 80_ 70_ \n"
                                                 "91馬=> 2个 70_ 72_ \n"
                                                 "92象=> 2个 70_ 74_ \n"
                                                 "93士=> 1个 84_ \n"
                                                 "94将=> 1个 84_ \n"
                                                 "95士=> 1个 84_ \n"
                                                 "96象=> 2个 74_ 78_ \n"
                                                 "97馬=> 2个 76_ 78_ \n"
                                                 "98車=> 2个 88_ 78_ \n" } },
            { { PieceColor::RED, "14帅=> 4个 13_ 15_ 04_ 24_ \n"
                                 "20相=> 2个 02_ 42_ \n"
                                 "25马=> 8个 04_ 06_ 13_ 17_ 33_ 37_ 44_ 46_ \n"
                                 "28相=> 2个 06_ 46_ \n"
                                 "76车=> 17个 75_ 74_ 73_ 72_ 71_ 70_ 77_ 78_ 66_ 56_ 46_ 36_ 26_ 16_ 06_ 86_ 96_ \n" },
                { PieceColor::BLACK, "03砲=> 17个 02_ 01_ 00_ 04_ 05_ 06_ 07_ 08_ 13_ 23_ 33_ 43_ 53_ 63_ 73_ 83_ 93_ \n"
                                     "68卒=> 1个 58_ \n"
                                     "84士=> 3个 73_ 75_ 93_ \n"
                                     "85将=> 0个 \n"
                                     "88車=> 4个 87_ 86_ 78_ 98_ \n"
                                     "95士=> 0个 \n" } } };
    for (int i = 0; i < fen.length(); ++i) {
        assert(pieceChars[i] == FENTopieChars(fen[i]));
        assert(fen[i] == pieCharsToFEN(pieceChars[i]));

        reset(fen[i]);
        assert(fen[i] == getFEN());
        assert(boardString[i] == toFullString() + getKingSeat__(bottomColor_)->toString());

        for (auto color : { PieceColor::RED, PieceColor::BLACK }) {
            assert(seatStr[i][color] == SeatManager::getSeatsStr(getLiveSeats__(color)));
            //Tools::writeTxtFile("test.txt", SeatManager::getSeatsStr(getLiveSeats__(color)) + '\n', QIODevice::Append);

            //*
            QString qstr {};
            QTextStream stream(&qstr);
            for (auto& fseat : getLiveSeats__(color))
                //qstr.append(fseat->toString() + "=> " + SeatManager::getSeatsStr(fseat->getMoveSeats(*this)) + '\n');
                stream << fseat->toString() << "=> " << SeatManager::getSeatsStr(fseat->getMoveSeats(*this)) << '\n';

            assert(moveSeatStr[i][color] == qstr);
            //Tools::writeTxtFile("test.txt", qstr + '\n', QIODevice::Append);
            //*/
        }
    }

    // 测试第2个FEN的更改棋局函数
    QList<ChangeType> chg = { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY };
    reset(fen[1]);
    for (int i = 0; i < chg.length(); ++i) {
        changeSide(chg[i]);
        QString qstr { toFullString() + getKingSeat__(bottomColor_)->toString() };
        /*
        if (boardChangeStr[i] == qstr) {
            Tools::writeTxtFile("test.txt", boardChangeStr[i] + '\n', QIODevice::Append);
            Tools::writeTxtFile("test.txt", qstr + '\n', QIODevice::Append);
        }
        //*/

        assert(boardChangeStr[i] == qstr);
    }

    return true;
}

PPiece Board::getOtherPiece__(PPiece piece) const
{
    assert(piece);
    return pieces_.at((pieces_.indexOf(piece) + PieceManager::pieceSideNum) % PieceManager::pieceNum);
}

const QList<PPiece> Board::getBoardPieces__(const QString& pieceChars) const
{
    assert(pieceChars.length() == SeatManager::seatsNum());
    QList<PPiece> pieces {};
    for (auto& ch : pieceChars) {
        if (ch == PieceManager::nullChar())
            pieces.append(nullptr);
        else {
            for (auto& piece : pieces_) {
                if (piece->ch() == ch && pieces.indexOf(piece) == -1) {
                    pieces.append(piece);
                    break;
                }
            }
            assert("没有找到字符所指的棋子对象，出错了！");
        }
    }
    assert(pieces.length() == SeatManager::seatsNum());
    return pieces;
}

void Board::reset__(const QList<PPiece> boardPieces)
{
    int index { 0 };
    for (const auto& seat : seats_)
        seat->put(boardPieces[index++]);

    bottomColor_ = (SeatManager::isBottom(getKingSeat__(PieceColor::RED)->row())
            ? PieceColor::RED
            : PieceColor::BLACK);
}

const PSeat& Board::getKingSeat__(PieceColor color) const
{
    for (const auto& seat : seats_) {
        auto& piece = seat->piece();
        if (piece && PieceManager::isKing(piece->name()) && piece->color() == color)
            return seat;
    }
    assert("没有找到将帅棋子，出错了！");
    return seats_[4];
}

const QList<PSeat> Board::getLiveSeats__(PieceColor color) const
{
    return getLiveSeats__(color, SeatManager::nullName(), SeatManager::nullCol());
}

const QList<PSeat> Board::getLiveSeats__(PieceColor color, QChar name) const
{

    return getLiveSeats__(color, name, SeatManager::nullCol());
}

const QList<PSeat> Board::getLiveSeats__(PieceColor color, QChar name, int col, bool getStronge) const
{
    QList<PSeat> seats {};
    for (const auto& seat : seats_) {
        auto& piece = seat->piece();
        if (piece && color == piece->color()
            && (name == SeatManager::nullName() || name == piece->name())
            && (col == SeatManager::nullCol() || col == seat->col())
            && (!getStronge || PieceManager::isStronge(piece->name())))
            seats.append(seat);
    }
    return seats;
}

// '多兵排序'
const QList<PSeat> Board::getSortPawnLiveSeats__(bool isBottom, PieceColor color, QChar name) const
{
    // 最多5个兵, 按列建立字典，按列排序
    QList<PSeat> pawnSeats { getLiveSeats__(color, name) }, seats {};
    QMap<int, QList<PSeat>> colSeats {};

    for (auto& seat : pawnSeats)
        // 底边则列倒序,每列位置顺序
        colSeats[isBottom ? -seat->col() : seat->col()].append(seat);

    // 整合成一个数组
    for (auto col : colSeats.keys())
        if (colSeats[col].size() > 1) //  选取多于一个位置的列
            for (auto& seat : colSeats[col])
                seats.prepend(seat); // 按列存入
    return seats;
}

bool Board::isFace__() const
{
    PSeat kingSeat { getKingSeat__(PieceColor::RED) },
        othKingSeat { getKingSeat__(PieceColor::BLACK) };
    int fcol { kingSeat->col() };
    if (fcol != othKingSeat->col())
        return false;

    bool isBottom { isBottomSide(PieceColor::RED) };
    int lrow { isBottom ? kingSeat->row() : othKingSeat->row() },
        urow { isBottom ? othKingSeat->row() : kingSeat->row() };
    for (int row = lrow + 1; row < urow; ++row)
        if (getSeat(row, fcol)->piece())
            return false;

    return true;
}

bool Board::isStatusIfMoved__(PSeat& fseat, PSeat& tseat, bool (*isFunc__)(PieceColor), bool isFromColor)
{
    PieceColor color = fseat->piece()->color();
    if (!isFromColor)
        color = PieceManager::getOtherColor(color);
    PPiece eatPiece = fseat->movTo(*tseat);
    bool isStatus = isFunc__(color);
    tseat->movTo(*fseat, eatPiece);
    return isStatus;
}

bool Board::moveFailed__(PSeat& fseat, PSeat& tseat)
{
    //return isStatusIfMoved__(fseat, tseat, isKilled, true); // 己方
    return fseat && tseat;
}

bool Board::isContinuousKilling__(PSeat& tseat)
{
    PieceColor color = tseat->piece()->color();
    bool killed = false;
    if (isKilling(color)) {
        auto tseats = getLiveSeats__(color);
        for (int i = 0; i < tseats.length(); ++i) {
            /*
            int mtseatCount = canMoveSeats(mtseats, board, tseats[i], true);
            killed = mtseatCount == 0; // 对方无棋子可走，退出递归调用
            for (int j = 0; j < mtseatCount; ++j) {
                Piece eatFPiece = movePiece(tseats[i], mtseats[j], getBlankPiece()); // 走一着

                Seat fseats[SIDEPIECENUM], mfseats[BOARDROW + BOARDCOL];
                int fpieCount = getLiveSeats_bc(fseats, board, color, false);
                for (int k = 0; k < fpieCount; ++k) {
                    int mfseatCount = canMoveSeats(mfseats, board, fseats[k], true);
                    for (int l = 0; l < mfseatCount; ++l) {
                        killed = isContinuousKilling__(board, fseats[k], mfseats[l]); // 递归调用
                        if (killed)
                            break;
                    }
                    if (killed)
                        break;
                }

                movePiece(mtseats[j], tseats[i], eatFPiece);
                if (killed)
                    break;
            }
            if (killed)
                break;
            //*/
        }
    }

    return killed;
}

bool Board::isForce__(PSeat& tseat)
{
    QChar name = tseat->piece()->name();
    // 帅(将)、未过河兵(卒)，不算“子力”
    return !(PieceManager::isKing(name)
        || (PieceManager::isPawn(name) && isAcorssRiver__(tseat)));
}

bool Board::isProtectedForce__(PSeat& tseat)
{
    if (!isForce__(tseat))
        return false;

    PieceColor color = tseat->piece()->color();
    auto tseats = getLiveSeats__(color);
    for (int i = 0; i < tseats.length(); ++i) {
        auto ttseats = tseats[0]->getMoveSeats(*this);
        for (int j = 0; j < ttseats.length(); ++j) {
            /*
            if (mseats[j] == tseat) {
                Piece eatPiece0 = movePiece(fseat, tseat, getBlankPiece()); // 对方走一着
                Piece eatPiece1 = movePiece(fseats[i], tseat, getBlankPiece()); // 己方走一着

                bool willKill = false;
                //bool willKill = isWillKill(board, getColor(getPiece_s(tseat))); // 是否被杀

                movePiece(tseat, fseats[i], eatPiece1);
                movePiece(tseat, fseat, eatPiece0);
                if (!willKill)
                    return true;
            }
            //*/
        }
    }

    return false;
}

bool Board::isAcorssRiver__(const PSeat& seat)
{
    assert(seat && seat->piece());
    // 是否底部颜色 和 是否非底部位置 两者结果相等
    return isBottomSide(seat->piece()->color()) == !SeatManager::isBottom(seat->row());
}
