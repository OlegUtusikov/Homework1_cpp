#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->FindButton, &QPushButton::clicked, this, [this] {
        finder.set_substring(ui->InputData->toPlainText());
    });


    connect(&finder, &finder::result_changed, this, [this] {
        finder::result cur_res = finder.get_result();
        ui->FilesOut->clear();
        std::stringstream ss;
        for (std::size_t ind = 0; ind < cur_res.files.size(); ++ind)
        {
             ss << QString::number(ind).append(") ").append(cur_res.files[ind]).append('\n').toStdString();
        }
        if (!cur_res.done_work)
        {
            ss << "\n Findning ... \n";
        }
        ui->FilesOut->setText(ss.str().c_str());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
