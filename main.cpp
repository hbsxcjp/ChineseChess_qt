#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>
//#include <locale>

int main(int argc, char* argv[])
{
    //std::locale loc = std::locale::global(std::locale(""));
    //setlocale(LC_ALL, "");
    //QTextCodec* codec = QTextCodec::codecForName("utf-8");
    //QTextCodec::setCodecForLocale(codec);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
