#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ecco.h"
#include <QFile>
#include <QFontComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QSpinBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionTest_triggered();
    void on_actFileSave_triggered();

    void on_eccoLibFinished();
    void on_instanceFinished();

private:
    // 编写代码创建界面
    void initUI__();
    QLabel* labCurFile;
    QProgressBar* progressBar;
    QSpinBox* spinSize;
    QFontComboBox* fontComboBox;

    // 下载网络文件的函数是异步执行的，需要一个全生命对象
    InitEcco* initEcco {};

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
