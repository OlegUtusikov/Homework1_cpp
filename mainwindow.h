#pragma once

#include <memory>
#include <QMainWindow>
#include "finder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void print_res();
    void print_error();
    void print_finish();

private:
    void find_substring(QString const& path, bool hidden, QString const& str);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<finder> finder_ptr;
    std::size_t outInd {1};
    bool new_out {true};
};
