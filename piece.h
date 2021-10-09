#ifndef PIECE_H
#define PIECE_H

//=================================================================
//棋子相关的类型
//=================================================================

#include <QList>
#include <QMap>
#include <QString>

class Piece;
using PPiece = Piece*;

class Seat;
using PSeat = Seat*;

class Board;

enum class PieceColor {
    RED,
    BLACK
};

using SeatCoord = QPair<int, int>;

// 棋子类
class Piece {
public:
    Piece(QChar ch);

    QChar ch() const;
    QChar name() const;
    PieceColor color() const;
    const QString toString() const;

    // 棋子可置入位置
    QList<PSeat> putSeats(const Board& board) const { return putSeats__(board); }

    // 棋子从某位置可移至位置
    QList<PSeat> moveSeats(const Board& board, Seat& fseat) const;

    virtual ~Piece() = default;

private:
    virtual QList<PSeat> putSeats__(const Board& board) const;
    virtual QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const = 0;

    QChar ch_;
};

class King : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> putSeats__(const Board& board) const;
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class Advisor : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> putSeats__(const Board& board) const;
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class Bishop : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> putSeats__(const Board& board) const;
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class Knight : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class Rook : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class Cannon : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class Pawn : public Piece {
public:
    using Piece::Piece;

private:
    QList<PSeat> putSeats__(const Board& board) const;
    QList<PSeat> moveSeats__(const Board& board, Seat& fseat) const;
};

class PieceManager {
public:
    static PieceColor getOtherColor(PieceColor color);
    static const QString getZhChars();
    static const QString getICCSChars();
    static const QString getFENStr();
    static bool redIsBottom(const QString& fen);
    static int getRowFromICCSChar(QChar ch);
    static int getColFromICCSChar(QChar ch);
    static QChar getOtherChar(QChar ch);
    static QChar getColICCSChar(int col);

    static QChar getName(QChar ch);
    static QChar getPrintName(const Piece& piece);
    static PieceColor getColor(QChar ch);
    static PieceColor getColorFromZh(QChar numZh);
    static int getIndex(int seatsLen, bool isBottom, QChar preChar);
    static QChar getIndexChar(int seatsLen, bool isBottom, int index);
    static QChar nullChar();
    static QChar redKingChar();
    static int getMovNum(bool isBottom, QChar movChar);
    static QChar getMovChar(bool isSameRow, bool isBottom, bool isLowToUp);
    static int getNum(PieceColor color, QChar numChar);
    static QChar getNumChar(PieceColor color, int num);
    static int getCol(bool isBottom, int num);
    static QChar getColChar(PieceColor color, bool isBottom, int col);
    static bool isKing(QChar name);
    static bool isAdvBish(QChar name);
    static bool isStronge(QChar name);
    static bool isLineMove(QChar name);
    static bool isPawn(QChar name);
    static bool isPiece(QChar name);

    static const QString chChars;

    // 棋子颜色、种类、每方、全部个数
    static constexpr int pieceColorNum { 2 }, pieceKindNum { 7 },
        pieceSideNum { 16 }, pieceNum { 32 };

private:
    static const QString getPreChars__(int length);

    static const QString preChars_;
    static const QString nameChars_;
    static const QString movChars_;
    static const QMap<PieceColor, QString> numChars_;
    static const QString ICCS_ColChars_;
    static const QString ICCS_RowChars_;
    static const QString FENStr_;
    static const QChar nullChar_;
};

#endif // PIECE_H
