#ifndef DOWNHTML_H
#define DOWNHTML_H

#include <QTextCodec>
#include <QThread>
#include <QUrl>
#include <QtDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class DownHtml : public QObject {
    Q_OBJECT
public:
    DownHtml(const char* codeName);
    ~DownHtml();

    void startDowning(const QString& url);

Q_SIGNALS:
    void goDowning(const QString& url);
    void readyRead(const QString& downedStr);

private slots:
    void doDowning(const QString& url);
    //void on_finished(QNetworkReply* reply);
    void on_finished();

private:
    QTextCodec* codec_;
    QThread* thread_;
    QNetworkAccessManager* manager_;
};

#endif // DOWNHTML_H
