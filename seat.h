#ifndef SEAT_H
#define SEAT_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

enum class PieceColor;
enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY,
    NOCHANGE
};

class Piece;
using PPiece = Piece*;

class Seat;
using PSeat = Seat*;

class Pieces;
class Board;

class Seat {

public:
    Seat(int row, int col);

    int row() const { return row_; }
    int col() const { return col_; }
    int rowcol() const { return row_ * 10 + col_; }
    const PPiece& piece() const { return piece_; }

    // 在行棋规则下本位置可走的位置，且排除目标同色、将帅对面、被将军的情形
    const QList<PSeat> getMoveSeats(const Board& board);
    void put(PPiece piece = nullptr) { piece_ = piece; }
    PPiece movTo(Seat& tseat, PPiece fillPiece = nullptr);

    const QString toString() const;

private:
    const int row_, col_;
    PPiece piece_ {};
};

class SeatManager {
public:
    static int rowNum() { return RowNum_; };
    static int ColNum() { return ColNum_; };
    static int seatsNum() { return RowNum_ * ColNum_; };
    static bool isBottom(int row) { return row < RowLowUpIndex_; };
    static int getIndex(int row, int col) { return row * ColNum_ + col; }
    static int getIndex(int rowcol) { return rowcol / 10 * ColNum_ + rowcol % 10; }
    static int getChangeRow(int row, ChangeType ct);
    static int getChangeCol(int col, ChangeType ct);
    static int getChangeRowcol(int rowcol, ChangeType ct);

    static const QList<QPair<int, int>> getAllRowcols();

    static const QList<PSeat> getAllSeats(const Board& board);
    static const QList<PSeat> getKingSeats(const Board& board, const Piece& piece);
    static const QList<PSeat> getAdvisorSeats(const Board& board, const Piece& piece);
    static const QList<PSeat> getBishopSeats(const Board& board, const Piece& piece);
    static const QList<PSeat> getPawnSeats(const Board& board, const Piece& piece);

    static const QList<PSeat> getKingMoveSeats(const Board& board, Seat& fseat);
    static const QList<PSeat> getAdvisorMoveSeats(const Board& board, Seat& fseat);
    static const QList<PSeat> getBishopMoveSeats(const Board& board, Seat& fseat);
    static const QList<PSeat> getKnightMoveSeats(const Board& board, Seat& fseat);
    static const QList<PSeat> getRookMoveSeats(const Board& board, Seat& fseat);
    static const QList<PSeat> getCannonMoveSeats(const Board& board, Seat& fseat);
    static const QList<PSeat> getPawnMoveSeats(const Board& board, Seat& fseat);

    static const QString getSeatsStr(const QList<PSeat>& seats);

    static int nullCol() { return -1; };
    static QChar nullName() { return '0'; };

private:
    static const QList<QPair<int, int>> getKingRowcols__(bool isBottom);
    static const QList<QPair<int, int>> getAdvisorRowcols__(bool isBottom);
    static const QList<QPair<int, int>> getBishopRowcols__(bool isBottom);
    static const QList<QPair<int, int>> getPawnRowcols__(bool isBottom);

    static const QList<PSeat> getSeats__(const Board& board, QList<QPair<int, int>> rowcols);
    static const QList<QPair<PSeat, PSeat>> getBishopObs_MoveSeats__(const Board& board, Seat& fseat);
    static const QList<QPair<PSeat, PSeat>> getKnightObs_MoveSeats__(const Board& board, Seat& fseat);
    static const QList<PSeat> getNonObstacleSeats__(const QList<QPair<PSeat, PSeat>>& obs_MoveSeats);
    static const QList<QList<PSeat>> getRookCannonMoveSeat_Lines__(const Board& board, Seat& fseat);

    static const int RowNum_ { 10 }, ColNum_ { 9 },
        RowLowIndex_ { 0 }, RowLowMidIndex_ { 2 }, RowLowUpIndex_ { 4 },
        RowUpLowIndex_ { 5 }, RowUpMidIndex_ { 7 }, RowUpIndex_ { 9 },
        ColLowIndex_ { 0 }, ColMidLowIndex_ { 3 }, ColMidUpIndex_ { 5 }, ColUpIndex_ { 8 };
};

#endif // SEAT_H
