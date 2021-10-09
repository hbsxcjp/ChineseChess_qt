#include "downhtml.h"

DownHtml::DownHtml(const char* codeName)
    : codec_ { QTextCodec::codecForName(codeName) }
    , thread_ { new QThread }
    , manager_ {}
{
    connect(this, &DownHtml::goDowning, this, &DownHtml::doDowning);
    // 在独立的线程异步运行
    this->moveToThread(thread_);
    thread_->start();
}

DownHtml::~DownHtml()
{
    if (manager_)
        manager_->deleteLater();
    delete thread_;
}

void DownHtml::startDowning(const QString& url)
{
    emit goDowning(url);
}

void DownHtml::doDowning(const QString& url)
{
    qDebug() << "DownHtml::doDowning()" << url;
    QUrl qurl;
    if (url.isEmpty() || !(qurl = QUrl::fromUserInput(url)).isValid()) {
        emit readyRead(QString {});
        return;
    }

    // 为支持在独立线程运行，须在此运行线程新建对象，而不能在非本对象运行线程建立（如构造函数建立、以参数方式传入）
    if (!manager_)
        manager_ = new QNetworkAccessManager;
    auto reply = manager_->get(QNetworkRequest(qurl));
    connect(reply, &QNetworkReply::finished, this, &DownHtml::on_finished);

    qDebug() << "DownHtml::doDowning()" << this->thread();
}

void DownHtml::on_finished()
{
    auto reply = static_cast<QNetworkReply*>(sender());
    qDebug() << "DownHtml::on_finished()" << reply->url();
    emit readyRead(reply->url().toString() + '\n' + codec_->toUnicode(reply->readAll()));

    reply->deleteLater();
    reply = Q_NULLPTR;
}
