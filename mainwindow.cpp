#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <QDir>
#include <iostream>

void MainWindow::print_res()
{
    if (new_out)
    {
        ui->FilesOut->clear();
        new_out = false;
    }
    QString lastStr = finder_ptr.get()->get_last_res();
    ui->FilesOut->append(QString::number(outInd) + ") " + lastStr);
    ++outInd;
}

void MainWindow::init_and_connect()
{

}

void MainWindow::find_substring(const QString &path, bool hidden, const QString &str)
{
    if (finder_ptr.get() != nullptr)
    {
        disconnect(finder_ptr.get(), &finder::result_changed, this, &MainWindow::print_res);
        finder_ptr.reset();
    }
    finder_ptr.reset(new finder);
    connect(finder_ptr.get(), &finder::result_changed, this, &MainWindow::print_res);
    finder_ptr->run(str, path, hidden);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->FindButton, &QPushButton::clicked, this, [this] {
        ui->FilesOut->clear();
        new_out = true;
        ui->FilesOut->setText("Finding...");
        this->outInd = 1;
        QString path =  ui->PathData->toPlainText();
        if (path.isEmpty())
        {
            path = QDir::homePath();
        }
        std::cout << path.toStdString() << std::endl;
        QString str = ui->InputData->toPlainText();
        bool hidden = ui->checkBox->isChecked();
        this->find_substring(path, hidden, str);
    });

    connect(ui->StopButton, &QPushButton::clicked, this, [this] {
        if (finder_ptr.get() == nullptr)
        {
            return;
        }
        disconnect(finder_ptr.get(), &finder::result_changed, this, &MainWindow::print_res);
        finder_ptr.reset();
        ui->FilesOut->append("Stopped!");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
