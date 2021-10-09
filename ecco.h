#ifndef ECCO_H
#define ECCO_H

#include "downhtml.h"
#include "instance.h"
#include "piece.h"
#include "tools.h"
#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QString>

using PInstance = Instance*;
class Ecco {
public:
    Ecco(const QString& libFileName);
    Ecco(const QString& dbName, const QString& tblName);

    // 设置棋谱对象的开局名称
    bool setECCO(PInstance& ins) const;
    bool setECCO(QList<PInstance> insList) const;

private:
    QList<QRegularExpression> regList_ {};
};

class InitEcco : public QObject {
    Q_OBJECT;

public:
    //InitEcco();
    ~InitEcco();

    void startInitEccoLibToDB(const QString& dbName, const QString& tblName);
    void setEccoLibField();

    void startDowningInstanceToDB(const QString& dbName, const QString& tblName, int first, int last);

signals:
    void eccoLibFinished();
    void instanceFinished();

private slots:
    void handleEccoLibDownStr(const QString& downedStr);
    void handleInstanceDownStr(const QString& downedStr);

private:
    //void setEccoRegstrField__();

    // 获取棋谱对象链表
    static QList<PInstance> getInsList_dir__(const QString& dirName);
    static QList<PInstance> getInsList_webfile__(const QString& insFileName);
    static QList<PInstance> getInsList_db__(const QString& dbName, const QString& man_tblName);

    // 存储对象的info数据至数据库（返回对象个数）
    static int storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName);

    QString dbName_ {}, tblName_ {};
    QMetaObject::Connection con_ {};
    DownHtml* dowHtml_ {};
};

#endif // ECCO_H
