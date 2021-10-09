#ifndef BOARD_H
#define BOARD_H

#include <QList>
#include <QMap>
#include <QString>

class Piece;
using PPiece = Piece*;

class Seat;
using PSeat = Seat*;

enum class PieceColor;
enum class RecFormat;
enum class ChangeType;
enum class BoardStatus {
    FAIL,
    KILLING,
    WILLKILL,
    CATCH,
    NODANGE
};

class Board {
public:
    Board();
    ~Board();

    const PSeat& getSeat(const int row, const int col) const;
    const PSeat& getSeat(const int rowcol) const;
    const PSeat& getSeat(const QPair<int, int>& rowcol) const;
    // 获取相对位置的Seat
    const PSeat& getOtherSeat(const PSeat& seat, ChangeType ct) const;

    bool isBottomSide(PieceColor color) const { return bottomColor_ == color; }

    // 判断某方棋子是否已失败、被将军、被杀、被捉
    BoardStatus status(PieceColor color);

    // 着法走之后，判断是否已输棋 (将死、对面、困毙)
    // 第3条　将死和困毙
    //　　3.1　一方的棋子攻击对方的帅(将) ，并在下一着要把它吃掉，称为“照将”，或简称“将”。“照将”不必声明。
    //　　被“照将”的一方必须立即“应将”，即用自己的着法去化解被“将”的状态。
    //　　如果被“照将”而无法“应将”，就算被“将死”。
    //　　3.2　轮到走棋的一方，无子可走，就算被“困毙”。
    bool isFail(PieceColor color) const;

    // 是否处于将帅对面、被将军状态
    bool isKilled(PieceColor color) const;

    // 28.1　将
    //　　凡走子直接攻击对方帅(将) 者，称为“照将”，简称“将”。
    bool isKilling(PieceColor color) const;

    // 未测试
    // 28.2　杀
    //　　凡走子企图在下一着照将或连续照将，将死对方者，称为“杀着”，简称“杀”。
    bool isWillKill(PieceColor color);

    // 未测试
    // 28.3　捉
    //　　凡走子后能够造成在下一着(包括从下一着开始运用连续照将或连续交换的手段) 吃掉对方某个无根子，称为“捉”。
    // 构成“捉子”，应符合下列条件：
    //　　29.1　捉子的形式可以有：能够直接吃子；能够通过连续照将吃子；能够通过完整的交换过程得子。
    //　　29.2　“捉”产生于刚走的这着棋，上一着尚不存在。
    //　　29.3　直接或间接被捉的，是有子力价值的无根子(含：假根子、少根子)。
    //　　29.4　下一着吃子或得子后，不致被将死。
    bool isCatch(PieceColor color);

    // 28.4　打
    //　　将、杀、捉等攻击手段，统称为“打”。
    bool isClout(PieceColor color);

    // 28.9　闲
    //　　凡走子性质不属于将、杀、捉，统称为“闲”。兑、献、拦、跟，均属“闲”的范畴。
    bool isIdle(PieceColor color);

    // 着法走之前，判断起止位置是可走的位置(遵守“行走规则”，并排除：棋子同色、将帅对面、被将军等的情况)
    bool isCanMove(PSeat& fseat, PSeat& tseat);

    void reset(const QString& fen);
    void changeSide(ChangeType ct);

    QPair<PSeat, PSeat> fromToSeats(const QString& zhStr, bool tolerateError = false) const;
    QString pgn_zhStr(const PSeat& fseat, const PSeat& tseat) const;

    static QString pieCharsToFEN(const QString& pieceChars); // 便利函数，下同
    static QString FENTopieChars(const QString& fen);
    static QString changeFEN(const QString& fen, ChangeType ct);

    QString getFEN() const;
    QString getFEN_ct(ChangeType ct) const;
    QString toString() const;
    QString toFullString() const;
    bool test();

private:
    PPiece getOtherPiece__(PPiece piece) const;
    const QList<PPiece> getBoardPieces__(const QString& pieceChars) const;
    void reset__(const QList<PPiece> boardPieces);

    const PSeat& getKingSeat__(PieceColor color) const;
    const QList<PSeat> getLiveSeats__(PieceColor color) const;
    const QList<PSeat> getLiveSeats__(PieceColor color, QChar name) const;
    const QList<PSeat> getLiveSeats__(PieceColor color, QChar name, int col, bool getStronge = false) const;
    const QList<PSeat> getSortPawnLiveSeats__(bool isBottom, PieceColor color, QChar name) const;

    // 着法走之后，判断是否将帅对面
    bool isFace__() const;

    // 模拟移动棋子，判断取得棋子移动后的棋局状态是否符合指定函数意义
    bool isStatusIfMoved__(PSeat& fseat, PSeat& tseat,
        bool (*isFunc__)(PieceColor color), bool isFromColor);

    // 判断移动是否失败(将帅对面、自身被将则属失败)
    bool moveFailed__(PSeat& fseat, PSeat& tseat);

    // 判断是否照将或连续照将，将死对方者
    bool isContinuousKilling__(PSeat& tseat);

    // 是否子力
    // (25.3　车、马、炮、过河兵(卒)、士、相(象)，均算“子力”。帅(将)、未过河兵(卒)，不算“子力”。“子力”简称“子”
    // 子力价值是衡量子力得失的尺度，也是判断是否“捉子”的依据之一。原则上，一车相当于双马、双炮或一马一炮；
    // 马炮相等；士相(象)相等；过河兵(卒)价值浮动，一兵换取数子或一子换取数兵均不算得子。)
    bool isForce__(PSeat& tseat);

    // 是否有根子
    // (28.16　有根子和无根子
    //　　凡有己方其他棋子(包括暗根)充分保护的子，称为“有根子”；反之，称为“无根子”。
    //　　形式上是根，实际上起不到充分保护作用，称为假根或少根、假根子和少根子按无根子处理。)
    bool isProtectedForce__(PSeat& tseat);

    bool isAcorssRiver__(const PSeat& seat);

    PieceColor bottomColor_;
    QList<PPiece> pieces_ {};
    QList<PSeat> seats_;
};

#endif // BOARD_H
