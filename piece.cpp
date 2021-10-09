#include "piece.h"
#include "seat.h"
#include <QTextStream>

Piece::Piece(QChar ch)
    : ch_ { ch }
{
}

QChar Piece::ch() const { return ch_; }

QChar Piece::name() const { return PieceManager::getName(ch_); }

PieceColor Piece::color() const { return PieceManager::getColor(ch_); }

const QString Piece::toString() const
{
    QString qstr;
    qstr.append(color() == PieceColor::RED ? L'红' : L'黑')
        .append(PieceManager::getPrintName(*this))
        .append(ch());
    return qstr;
}

QList<PSeat> Piece::moveSeats(const Board& board, Seat& fseat) const
{
    return moveSeats__(board, fseat);
}

QList<PSeat> Piece::putSeats__(const Board& board) const
{
    return SeatManager::getAllSeats(board);
}

QList<PSeat> King::putSeats__(const Board& board) const
{
    return SeatManager::getKingSeats(board, *this);
}

QList<PSeat> King::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getKingMoveSeats(board, fseat);
}

QList<PSeat> Advisor::putSeats__(const Board& board) const
{
    return SeatManager::getAdvisorSeats(board, *this);
}

QList<PSeat> Advisor::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getAdvisorMoveSeats(board, fseat);
}

QList<PSeat> Bishop::putSeats__(const Board& board) const
{
    return SeatManager::getBishopSeats(board, *this);
}

QList<PSeat> Bishop::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getBishopMoveSeats(board, fseat);
}

QList<PSeat> Knight::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getKnightMoveSeats(board, fseat);
}

QList<PSeat> Rook::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getRookMoveSeats(board, fseat);
}

QList<PSeat> Cannon::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getCannonMoveSeats(board, fseat);
}

QList<PSeat> Pawn::putSeats__(const Board& board) const
{
    return SeatManager::getPawnSeats(board, *this);
}

QList<PSeat> Pawn::moveSeats__(const Board& board, Seat& fseat) const
{
    return SeatManager::getPawnMoveSeats(board, fseat);
}

PieceColor PieceManager::getOtherColor(PieceColor color)
{
    return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
}

const QString PieceManager::getZhChars()
{
    return (preChars_ + nameChars_ + movChars_
        + numChars_[PieceColor::RED] + numChars_[PieceColor::BLACK]);
}

const QString PieceManager::getICCSChars() { return ICCS_ColChars_ + ICCS_RowChars_; }

const QString PieceManager::getFENStr() { return FENStr_; }

bool PieceManager::redIsBottom(const QString& fen)
{
    return fen.indexOf(chChars[0]) < SeatManager::seatsNum() / 2;
}

int PieceManager::getRowFromICCSChar(QChar ch) { return ICCS_RowChars_.indexOf(ch); }

int PieceManager::getColFromICCSChar(QChar ch) { return ICCS_ColChars_.indexOf(ch); }

QChar PieceManager::getOtherChar(QChar ch)
{
    return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
}

QChar PieceManager::getColICCSChar(int col) { return ICCS_ColChars_.at(col); }

QChar PieceManager::getName(QChar ch)
{
    int chIndex_nameIndex[][2] {
        { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
        { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
    };
    return nameChars_.at(chIndex_nameIndex[chChars.indexOf(ch)][1]);
}

QChar PieceManager::getPrintName(const Piece& piece)
{
    const QMap<QChar, QChar> rcpName { { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' } };
    QChar name { piece.name() },
        bname { rcpName.value(name, PieceManager::nullChar()) };
    return (piece.color() == PieceColor::BLACK && bname != PieceManager::nullChar()) ? bname : name;
}

PieceColor PieceManager::getColor(QChar ch)
{
    return ch.isLower() ? PieceColor::BLACK : PieceColor::RED;
}

PieceColor PieceManager::getColorFromZh(QChar numZh)
{
    return numChars_[PieceColor::RED].indexOf(numZh) >= 0 ? PieceColor::RED : PieceColor::BLACK;
}

int PieceManager::getIndex(const int seatsLen, const bool isBottom, QChar preChar)
{
    int index = getPreChars__(seatsLen).indexOf(preChar);
    return isBottom ? seatsLen - 1 - index : index;
}

QChar PieceManager::getIndexChar(const int seatsLen, const bool isBottom, const int index)
{
    return getPreChars__(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

QChar PieceManager::nullChar() { return nullChar_; }

QChar PieceManager::redKingChar() { return chChars[0]; };

int PieceManager::getMovNum(bool isBottom, QChar movChar)
{
    return (movChars_.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
}

QChar PieceManager::getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
{
    return movChars_.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
}

int PieceManager::getNum(PieceColor color, QChar numChar)
{
    return numChars_[color].indexOf(numChar) + 1;
}

QChar PieceManager::getNumChar(PieceColor color, int num)
{
    return numChars_[color].at(num - 1);
}

int PieceManager::getCol(bool isBottom, const int num)
{
    return isBottom ? SeatManager::ColNum() - num : num - 1;
}

QChar PieceManager::getColChar(const PieceColor color, bool isBottom, const int col)
{
    return numChars_[color].at(isBottom ? SeatManager::ColNum() - col - 1 : col);
}

bool PieceManager::isKing(QChar name)
{
    return nameChars_.leftRef(2).indexOf(name) >= 0;
}

bool PieceManager::isAdvBish(QChar name)
{
    return nameChars_.midRef(2, 4).indexOf(name) >= 0;
}

bool PieceManager::isStronge(QChar name)
{
    return nameChars_.midRef(6, 5).indexOf(name) >= 0;
}

bool PieceManager::isLineMove(QChar name)
{
    return isKing(name) || nameChars_.midRef(7, 4).indexOf(name) >= 0;
}

bool PieceManager::isPawn(QChar name)
{
    return nameChars_.rightRef(2).indexOf(name) >= 0;
}

bool PieceManager::isPiece(QChar name)
{
    return nameChars_.indexOf(name) >= 0;
}

const QString PieceManager::getPreChars__(int length)
{
    return (length == 2 ? QString(preChars_).remove(1, 1) // "前后"
                        : (length == 3 ? preChars_ // "前中后"
                                       : numChars_[PieceColor::RED].left(5))); // "一二三四五";
}

const QString PieceManager::chChars { "KABNRCPkabnrcp" };
const QString PieceManager::preChars_ { "前中后" };
const QString PieceManager::nameChars_ { "帅将仕士相象马车炮兵卒" };
const QString PieceManager::movChars_ { "退平进" };
const QMap<PieceColor, QString> PieceManager::numChars_ {
    { PieceColor::RED, "一二三四五六七八九" },
    { PieceColor::BLACK, "１２３４５６７８９" }
};
const QString PieceManager::ICCS_ColChars_ { "abcdefghi" };
const QString PieceManager::ICCS_RowChars_ { "0123456789" };
const QString PieceManager::FENStr_ { "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
const QChar PieceManager::nullChar_ { '_' };
