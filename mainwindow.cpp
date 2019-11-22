#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <QDir>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->FindButton, &QPushButton::clicked, this, [this] {
        ui->FilesOut->clear();
        outInd = 1;
        QString path =  ui->PathData->toPlainText();
        if (path.isEmpty())
        {
            path = QDir::homePath();
        }
        finder.run(ui->InputData->toPlainText(), path, ui->checkBox->isEnabled());
    });

    connect(ui->StopButton, &QPushButton::clicked, this, [this] {
        finder.stop();
    });


    connect(&finder, &finder::result_changed, this, [this] {
        QString lastStr = finder.getLastRes();
        ui->FilesOut->append(QString::number(outInd) + ") " + lastStr);
        ++outInd;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
