#include <QDir>

#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::print_error()
{
    QString msg = finder_ptr.get()->get_error();
    ui->FilesOut->clear();
    ui->FilesOut->append("ERROR: " + msg);
}

void MainWindow::print_finish()
{
    ui->FilesOut->append("Completed!!!");
    finder_ptr.reset();
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
    connect(finder_ptr.get(), &finder::error, this, &MainWindow::print_error);
    connect(finder_ptr.get(), &finder::completed, this, &MainWindow::print_finish);
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
        QString str = ui->InputData->toPlainText();
        if (str.isEmpty())
        {
            ui->FilesOut->setText("ERROR: substring is empty.");
            return;
        }
        ui->FilesOut->setText("Finding...");
        this->outInd = 1;
        QString path =  ui->PathData->toPlainText();
        if (path.isEmpty())
        {
            path = QDir::homePath();
        }
        bool hidden = ui->checkBox->isChecked();
        this->find_substring(path, hidden, str);
    });

    connect(ui->StopButton, &QPushButton::clicked, this, [this] {
        if (finder_ptr.get() == nullptr)
        {
            return;
        }
        disconnect(finder_ptr.get(), &finder::result_changed, this, &MainWindow::print_res);
        disconnect(finder_ptr.get(), &finder::error, this, &MainWindow::print_error);
        disconnect(finder_ptr.get(), &finder::completed, this, &MainWindow::print_finish);
        finder_ptr.reset();
        if (new_out)
        {
            ui->FilesOut->clear();
        }
        ui->FilesOut->append("Stopped!");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
