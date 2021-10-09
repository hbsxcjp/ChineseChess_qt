#include "mainwindow.h"
#include "aspect.h"
#include "board.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setCentralWidget(ui->mdiArea);
    // this->setWindowState(Qt::WindowMaximized);

    initUI__();
}

MainWindow::~MainWindow()
{
    if (initEcco)
        delete initEcco;

    delete ui;
}

void MainWindow::initUI__()
{
    labCurFile = new QLabel;
    labCurFile->setMidLineWidth(200);
    labCurFile->setText("当前棋局：");
    ui->statusbar->addWidget(labCurFile);

    progressBar = new QProgressBar;
    progressBar->setMaximumWidth(200);
    progressBar->setMinimum(5);
    progressBar->setMaximum(50);
    progressBar->setValue(15);
    ui->statusbar->addWidget(progressBar);

    ui->playToolBar->addSeparator();
    ui->playToolBar->addWidget(new QLabel("字体 "));
    fontComboBox = new QFontComboBox;
    fontComboBox->setMinimumWidth(100);
    ui->playToolBar->addWidget(fontComboBox);

    spinSize = new QSpinBox;
    spinSize->setMinimumWidth(50);
    spinSize->setMinimum(5);
    spinSize->setMaximum(50);
    spinSize->setValue(10);
    ui->playToolBar->addWidget(new QLabel(" 大小 "));
    ui->playToolBar->addWidget(spinSize);
}

QMap<bool, QString> result { { true, "成功." }, { false, "失败！" } };

void MainWindow::on_actionTest_triggered()
{
    qDebug() << "MainWindow::on_actionTest_triggered()";

    ui->plainTextEdit->appendPlainText("Board.test " + result[(Board()).test()]);

    ui->plainTextEdit->appendPlainText("Instance.test " + result[testInstance()]);

    ui->plainTextEdit->appendPlainText("Aspect.test " + result[testAspect()]);

    /*
    //EccoInit::downEccoLibWeb(&tools);

    if (!initEcco)
        initEcco = new InitEcco;
    initEcco->startInitEccoLibToDB(QStringLiteral("chess.db"), QStringLiteral("ecco"));
    //initEcco->startDowningInstanceToDB(QStringLiteral("chess.db"), QStringLiteral("ecco"), 9210, 9215);
    */
}

void MainWindow::on_actFileSave_triggered()
{
    qDebug() << "MainWindow::on_actFileSave_triggered()";
    //ui->plainTextEdit->appendPlainText("Ecco.test " + result[EccoInit::saveEccoLibWeb("eccolib.txt", &tools)]);

    if (!initEcco)
        initEcco = new InitEcco;
    initEcco->setEccoLibField();
}

void MainWindow::on_eccoLibFinished()
{
    qDebug() << "MainWindow::on_eccoLibFinished()";
    //if (initEcco)
    //    delete initEcco;
}

void MainWindow::on_instanceFinished()
{
    qDebug() << "MainWindow::on_instanceFinished()";
    //if (initEcco)
    //    delete initEcco;
}
