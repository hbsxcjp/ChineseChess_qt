#include "seat.h"
#include "board.h"
#include "piece.h"
#include <QTextStream>

Seat::Seat(int row, int col)
    : row_ { row }
    , col_ { col }
{
}

const QList<PSeat> Seat::getMoveSeats(const Board& board)
{
    const PPiece& fpiece = piece();
    assert(fpiece);
    PieceColor color { fpiece->color() };
    QList<PSeat> seats;
    for (auto& tseat : fpiece->moveSeats(board, *this)) {
        auto& tpiece = tseat->piece();
        // 排除目标位置颜色相同
        if (tpiece && tpiece->color() == color)
            continue;

        // 移动棋子后，检测是否会被对方将军
        auto eatPiece = movTo(*tseat);
        if (!board.isKilled(color))
            seats.append(tseat);
        tseat->movTo(*this, eatPiece);
    }

    return seats;
}

PPiece Seat::movTo(Seat& tseat, PPiece fillPiece)
{
    auto eatPiece = tseat.piece();
    tseat.put(piece_);
    put(fillPiece);
    return eatPiece;
}

const QString Seat::toString() const
{
    QString qstr {};
    qstr.append(QString::number(row_)).append(QString::number(col_));
    qstr.append(piece_ ? PieceManager::getPrintName(*piece_) : PieceManager::nullChar());
    return qstr;
}

const QList<PSeat> SeatManager::getAllSeats(const Board& board)
{
    return getSeats__(board, getAllRowcols());
}

const QList<PSeat> SeatManager::getKingSeats(const Board& board, const Piece& piece)
{
    return getSeats__(board, getKingRowcols__(board.isBottomSide(piece.color())));
}

const QList<PSeat> SeatManager::getAdvisorSeats(const Board& board, const Piece& piece)
{
    return getSeats__(board, getAdvisorRowcols__(board.isBottomSide(piece.color())));
}

const QList<PSeat> SeatManager::getBishopSeats(const Board& board, const Piece& piece)
{
    return getSeats__(board, getBishopRowcols__(board.isBottomSide(piece.color())));
}

const QList<PSeat> SeatManager::getPawnSeats(const Board& board, const Piece& piece)
{
    return getSeats__(board, getPawnRowcols__(board.isBottomSide(piece.color())));
}

const QList<PSeat> SeatManager::getKingMoveSeats(const Board& board, Seat& fseat)
{
    bool isBottom { board.isBottomSide(fseat.piece()->color()) };
    int frow { fseat.row() }, fcol { fseat.col() };
    QList<QPair<int, int>> rowcols {
        { frow, fcol - 1 }, { frow, fcol + 1 },
        { frow - 1, fcol }, { frow + 1, fcol }
    };
    int rowLow { isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp { isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    QList<QPair<int, int>> kingRowcols {};
    for (const auto& rowcol : rowcols)
        if (rowcol.first >= rowLow && rowcol.first <= rowUp
            && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_)
            kingRowcols.append(rowcol);
    return getSeats__(board, kingRowcols);
}

const QList<PSeat> SeatManager::getAdvisorMoveSeats(const Board& board, Seat& fseat)
{
    bool isBottom { board.isBottomSide(fseat.piece()->color()) };
    int frow { fseat.row() }, fcol { fseat.col() };
    QList<QPair<int, int>> rowcols {
        { frow - 1, fcol - 1 }, { frow - 1, fcol + 1 },
        { frow + 1, fcol - 1 }, { frow + 1, fcol + 1 }
    };
    int rowLow { isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp { isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    QList<QPair<int, int>> advRowcols {};
    for (const auto& rowcol : rowcols)
        if (rowcol.first >= rowLow && rowcol.first <= rowUp
            && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_)
            advRowcols.append(rowcol);
    return getSeats__(board, advRowcols);
}

const QList<PSeat> SeatManager::getBishopMoveSeats(const Board& board, Seat& fseat)
{
    return getNonObstacleSeats__(getBishopObs_MoveSeats__(board, fseat));
}

const QList<PSeat> SeatManager::getKnightMoveSeats(const Board& board, Seat& fseat)
{
    return getNonObstacleSeats__(getKnightObs_MoveSeats__(board, fseat));
}

const QList<PSeat> SeatManager::getRookMoveSeats(const Board& board, Seat& fseat)
{
    QList<PSeat> moveSeats {};
    for (auto& seats : getRookCannonMoveSeat_Lines__(board, fseat))
        for (auto& seat : seats) {
            moveSeats.append(seat);
            if (seat->piece())
                break;
        }
    return moveSeats;
}

const QList<PSeat> SeatManager::getCannonMoveSeats(const Board& board, Seat& fseat)
{
    QList<PSeat> moveSeats {};
    for (auto& seats : getRookCannonMoveSeat_Lines__(board, fseat)) {
        bool skip = false;
        for (auto& seat : seats) {
            if (!skip) {
                if (!seat->piece())
                    moveSeats.append(seat);
                else
                    skip = true;
            } else if (seat->piece()) {
                moveSeats.append(seat);
                break;
            }
        }
    }
    return moveSeats;
}

const QList<PSeat> SeatManager::getPawnMoveSeats(const Board& board, Seat& fseat)
{
    bool isBottom { board.isBottomSide(fseat.piece()->color()) };
    int frow { fseat.row() }, fcol { fseat.col() };
    QList<PSeat> seats {};
    int row {}, col {};
    if ((isBottom && (row = frow + 1) <= RowUpIndex_)
        || (!isBottom && (row = frow - 1) >= RowLowIndex_))
        seats.append(board.getSeat(row, fcol));
    if (isBottom == (frow > RowLowUpIndex_)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex_)
            seats.append(board.getSeat(frow, col));
        if ((col = fcol + 1) <= ColUpIndex_)
            seats.append(board.getSeat(frow, col));
    }
    return seats;
}

const QString SeatManager::getSeatsStr(const QList<PSeat>& seats)
{
    QString qstr { QString::number(seats.length()) + "个 " };
    for (const auto& seat : seats)
        qstr.append(seat->toString()).append(' ');
    return qstr;
}

int SeatManager::getChangeRow(int row, ChangeType ct)
{
    if (ct == ChangeType::ROTATE)
        return RowNum_ - row - 1;

    return row;
}

int SeatManager::getChangeCol(int col, ChangeType ct)
{
    if (ct == ChangeType::ROTATE || ct == ChangeType::SYMMETRY)
        return ColNum_ - col - 1;

    return col;
}

int SeatManager::getChangeRowcol(int rowcol, ChangeType ct)
{
    if (ct == ChangeType::ROTATE)
        return (RowNum_ - rowcol / 10 - 1) * 10 + (ColNum_ - rowcol % 10 - 1);

    if (ct == ChangeType::SYMMETRY)
        return rowcol + ColNum_ - rowcol % 10 * 2 - 1;

    return rowcol;
}

const QList<QPair<int, int>> SeatManager::getAllRowcols()
{
    QList<QPair<int, int>> rowcols {};
    for (int row = 0; row < RowNum_; ++row)
        for (int col = 0; col < ColNum_; ++col)
            rowcols.append({ row, col });
    return rowcols;
}

const QList<QPair<int, int>> SeatManager::getKingRowcols__(bool isBottom)
{
    QList<QPair<int, int>> rowcols {};
    int rowLow { isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp { isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            rowcols.append({ row, col });
    return rowcols;
}

const QList<QPair<int, int>> SeatManager::getAdvisorRowcols__(bool isBottom)
{
    QList<QPair<int, int>> rowcols {};
    int rowLow { isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp { isBottom ? RowLowMidIndex_ : RowUpIndex_ }, rmd { isBottom ? 1 : 0 }; // 行列和的奇偶值
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            if ((col + row) % 2 == rmd)
                rowcols.append({ row, col });
    return rowcols;
}

const QList<QPair<int, int>> SeatManager::getBishopRowcols__(bool isBottom)
{
    QList<QPair<int, int>> rowcols {};
    int rowLow { isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp { isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2) {
            int gap { row - col };
            if ((isBottom && (gap == 2 || gap == -2 || gap == -6))
                || (!isBottom && (gap == 7 || gap == 3 || gap == -1)))
                rowcols.append({ row, col });
        }
    return rowcols;
}

const QList<QPair<int, int>> SeatManager::getPawnRowcols__(bool isBottom)
{
    QList<QPair<int, int>> rowcols {};
    int lfrow { isBottom ? RowLowUpIndex_ - 1 : RowUpLowIndex_ },
        ufrow { isBottom ? RowLowUpIndex_ : RowUpLowIndex_ + 1 },
        ltrow { isBottom ? RowUpLowIndex_ : RowLowIndex_ },
        utrow { isBottom ? RowUpIndex_ : RowLowUpIndex_ };
    for (int row = lfrow; row <= ufrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2)
            rowcols.append({ row, col });
    for (int row = ltrow; row <= utrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; ++col)
            rowcols.append({ row, col });
    return rowcols;
}

const QList<PSeat> SeatManager::getSeats__(const Board& board, QList<QPair<int, int>> rowcols)
{
    QList<PSeat> seats {};
    for (auto& rowcol : rowcols)
        seats.append(board.getSeat(rowcol));
    return seats;
}

const QList<QPair<PSeat, PSeat>> SeatManager::getBishopObs_MoveSeats__(const Board& board, Seat& fseat)
{
    bool isBottom { board.isBottomSide(fseat.piece()->color()) };
    int frow { fseat.row() }, fcol { fseat.col() };
    QList<QPair<PSeat, PSeat>> obs_MoveSeats {};
    QList<QPair<QPair<int, int>, QPair<int, int>>> obs_MoveRowcols {
        { { frow - 1, fcol - 1 }, { frow - 2, fcol - 2 } },
        { { frow - 1, fcol + 1 }, { frow - 2, fcol + 2 } },
        { { frow + 1, fcol - 1 }, { frow + 2, fcol - 2 } },
        { { frow + 1, fcol + 1 }, { frow + 2, fcol + 2 } }
    };
    int rowLow { isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp { isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    for (const auto& obs_Moverowcol : obs_MoveRowcols)
        if (obs_Moverowcol.second.first >= rowLow
            && obs_Moverowcol.second.first <= rowUp
            && obs_Moverowcol.second.second >= ColLowIndex_
            && obs_Moverowcol.second.second <= ColUpIndex_)
            obs_MoveSeats.push_back(
                { board.getSeat(obs_Moverowcol.first),
                    board.getSeat(obs_Moverowcol.second) });
    return obs_MoveSeats;
}

const QList<QPair<PSeat, PSeat>> SeatManager::getKnightObs_MoveSeats__(const Board& board, Seat& fseat)
{
    int frow { fseat.row() }, fcol { fseat.col() };
    QList<QPair<PSeat, PSeat>> obs_MoveSeats {};
    QList<QPair<QPair<int, int>, QPair<int, int>>> obs_MoveRowcols {
        { { frow - 1, fcol }, { frow - 2, fcol - 1 } },
        { { frow - 1, fcol }, { frow - 2, fcol + 1 } },
        { { frow, fcol - 1 }, { frow - 1, fcol - 2 } },
        { { frow, fcol + 1 }, { frow - 1, fcol + 2 } },
        { { frow, fcol - 1 }, { frow + 1, fcol - 2 } },
        { { frow, fcol + 1 }, { frow + 1, fcol + 2 } },
        { { frow + 1, fcol }, { frow + 2, fcol - 1 } },
        { { frow + 1, fcol }, { frow + 2, fcol + 1 } }
    };
    for (const auto& obs_Moverowcol : obs_MoveRowcols)
        if (obs_Moverowcol.second.first >= RowLowIndex_
            && obs_Moverowcol.second.first <= RowUpIndex_
            && obs_Moverowcol.second.second >= ColLowIndex_
            && obs_Moverowcol.second.second <= ColUpIndex_)
            obs_MoveSeats.push_back(
                { board.getSeat(obs_Moverowcol.first),
                    board.getSeat(obs_Moverowcol.second) });
    return obs_MoveSeats;
}

const QList<PSeat> SeatManager::getNonObstacleSeats__(const QList<QPair<PSeat, PSeat>>& obs_MoveSeats)
{
    QList<PSeat> seats {};
    for (const auto& obs_MoveSeat : obs_MoveSeats)
        if (!obs_MoveSeat.first->piece())
            seats.append(obs_MoveSeat.second);
    return seats;
}

const QList<QList<PSeat>> SeatManager::getRookCannonMoveSeat_Lines__(const Board& board, Seat& fseat)
{
    int frow { fseat.row() }, fcol { fseat.col() };
    QList<QList<PSeat>> seat_Lines { {}, {}, {}, {} };
    for (int col = fcol - 1; col >= ColLowIndex_; --col)
        seat_Lines[0].append(board.getSeat(frow, col));
    for (int col = fcol + 1; col <= ColUpIndex_; ++col)
        seat_Lines[1].append(board.getSeat(frow, col));
    for (int row = frow - 1; row >= RowLowIndex_; --row)
        seat_Lines[2].append(board.getSeat(row, fcol));
    for (int row = frow + 1; row <= RowUpIndex_; ++row)
        seat_Lines[3].append(board.getSeat(row, fcol));
    return seat_Lines;
}
