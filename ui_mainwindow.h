/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actFileNew;
    QAction *actFileOpen;
    QAction *actFileExit;
    QAction *actFileSave;
    QAction *actionTest;
    QWidget *centralwidget;
    QPlainTextEdit *plainTextEdit;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuPlay;
    QMenu *menuHelp;
    QMenu *menuCM;
    QStatusBar *statusbar;
    QToolBar *fileToolBar;
    QToolBar *cmToolBar;
    QToolBar *playToolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 700);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font.setPointSize(9);
        MainWindow->setFont(font);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icon/res/reading_60px.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        MainWindow->setIconSize(QSize(36, 36));
        actFileNew = new QAction(MainWindow);
        actFileNew->setObjectName(QString::fromUtf8("actFileNew"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icon/res/file_32px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actFileNew->setIcon(icon1);
        actFileNew->setFont(font);
        actFileOpen = new QAction(MainWindow);
        actFileOpen->setObjectName(QString::fromUtf8("actFileOpen"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icon/res/opened_folder_32px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actFileOpen->setIcon(icon2);
        actFileOpen->setFont(font);
        actFileExit = new QAction(MainWindow);
        actFileExit->setObjectName(QString::fromUtf8("actFileExit"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icon/res/logout_rounded_down_32px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actFileExit->setIcon(icon3);
        actFileExit->setFont(font);
        actFileSave = new QAction(MainWindow);
        actFileSave->setObjectName(QString::fromUtf8("actFileSave"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icon/res/save_32px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actFileSave->setIcon(icon4);
        actFileSave->setFont(font);
        actionTest = new QAction(MainWindow);
        actionTest->setObjectName(QString::fromUtf8("actionTest"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        plainTextEdit = new QPlainTextEdit(centralwidget);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setGeometry(QRect(0, 0, 800, 600));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(plainTextEdit->sizePolicy().hasHeightForWidth());
        plainTextEdit->setSizePolicy(sizePolicy);
        QFont font1;
        font1.setFamily(QString::fromUtf8("\346\226\260\345\256\213\344\275\223"));
        font1.setPointSize(11);
        plainTextEdit->setFont(font1);
        plainTextEdit->setAutoFillBackground(false);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuFile->setFont(font);
        menuPlay = new QMenu(menubar);
        menuPlay->setObjectName(QString::fromUtf8("menuPlay"));
        menuPlay->setFont(font);
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuHelp->setFont(font);
        menuCM = new QMenu(menubar);
        menuCM->setObjectName(QString::fromUtf8("menuCM"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        fileToolBar = new QToolBar(MainWindow);
        fileToolBar->setObjectName(QString::fromUtf8("fileToolBar"));
        fileToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        MainWindow->addToolBar(Qt::TopToolBarArea, fileToolBar);
        cmToolBar = new QToolBar(MainWindow);
        cmToolBar->setObjectName(QString::fromUtf8("cmToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, cmToolBar);
        playToolBar = new QToolBar(MainWindow);
        playToolBar->setObjectName(QString::fromUtf8("playToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, playToolBar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuCM->menuAction());
        menubar->addAction(menuPlay->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actFileNew);
        menuFile->addAction(actFileOpen);
        menuFile->addAction(actFileSave);
        menuFile->addSeparator();
        menuFile->addAction(actFileExit);
        menuPlay->addAction(actionTest);
        fileToolBar->addAction(actFileNew);
        fileToolBar->addAction(actFileOpen);
        fileToolBar->addAction(actFileSave);

        retranslateUi(MainWindow);
        QObject::connect(actFileExit, SIGNAL(triggered()), MainWindow, SLOT(close()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\345\255\246\350\261\241\346\243\213", nullptr));
        actFileNew->setText(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272", nullptr));
#if QT_CONFIG(tooltip)
        actFileNew->setToolTip(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272\344\270\200\344\270\252\346\243\213\345\261\200", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actFileNew->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actFileOpen->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200", nullptr));
#if QT_CONFIG(tooltip)
        actFileOpen->setToolTip(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\344\270\200\344\270\252\346\243\213\345\261\200", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actFileOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actFileExit->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272", nullptr));
#if QT_CONFIG(shortcut)
        actFileExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actFileSave->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
#if QT_CONFIG(tooltip)
        actFileSave->setToolTip(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\345\275\223\345\211\215\346\243\213\345\261\200", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actFileSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionTest->setText(QCoreApplication::translate("MainWindow", "Test", nullptr));
#if QT_CONFIG(shortcut)
        actionTest->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
        menuPlay->setTitle(QCoreApplication::translate("MainWindow", "\346\274\224\347\244\272", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "\345\270\256\345\212\251", nullptr));
        menuCM->setTitle(QCoreApplication::translate("MainWindow", "\346\243\213\350\260\261", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
