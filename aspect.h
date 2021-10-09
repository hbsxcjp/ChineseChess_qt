#ifndef ASPECT_H
#define ASPECT_H

#include "instance.h"
#include "piece.h"
#include <QDataStream>
#include <QHash>
#include <QList>
#include <QString>

using RC_RecordHash = QHash<int, Record>;
using RC_RecordHashPair = QPair<RC_RecordHash, RC_RecordHash>;
using FenRC_RecordPairHash = QHash<QString, RC_RecordHashPair>;

class Aspect {
public:
    int getFenCount() { return fen_rc_rec_.count(); }
    int getMoveCount_dif();
    int getMoveCount_total();

    RC_RecordHash& getRC_RecordHash(const QString& fen, PieceColor color);
    Record& getRecord(const QString& fen, PieceColor color, int rowcols);
    void putMoveRec(MoveRec& moveRec);

    // 存储文件到局面记录库
    void readRecFormatFile(const QString& fileName);
    void readRecFormatDir(const QString& dirName);

    void readAspectFile(const QString& fileName);
    void writeAspectFile(const QString& fileName);

    void readAspectFile_bin(const QString& fileName);
    void writeAspectFile_bin(const QString& fileName);

    QString toString();

private:
    RC_RecordHashPair& getRc_RecordHashPair__(const QString& fen);

    // 哈希表，存储局面(fen存入时已统一成红底)及红黑着法
    FenRC_RecordPairHash fen_rc_rec_ {};
};

bool testAspect();

#endif // ASPECT_H
