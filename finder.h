#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include <QObject>
#include <QString>
#include <QFile>
#include <QFileInfo>

class finder : public  QObject
{
    Q_OBJECT

public:
    finder();
    ~finder();

    void run(QString const& str, QString const& path, bool hidden);

    QString get_last_res() const;
    QString get_error() const;

signals:
    void result_changed();
    void error();
    void completed();

private:
    void queue_callback();
    void error_callback();
    void finish_callback();

    void callback_q();
    void callback_e();
    void callback_f();
    void clear_queies();
    void add_query(QString const& path);
    QString get_query();
    void set_str(QString const& str);
    void set_hidden(bool hidden);
    void find(QString const& path);

    void process_dir(QString const& path);
    void process_file(QString const& path);

    void find_in_file(QFile& file);

    void print_str(QFileInfo const& info);

    bool is_hidden(QString const& name);

private:

    const QString UNREAL = "`/";
    const int TIME_SLEEP = 20;
    const int MAX_SIZE = 4 * 1024 * 1024;
    QString m_str {""};
    QString lastStr {""};
    QString errorMsg {""};
    std::unordered_map<std::string, std::vector<std::pair<QString, int>>> res_map;
    std::queue<QString> queries;
    const std::size_t count_of_threads = 2;
    std::atomic<int> active_threads {0};
    std::vector<std::thread> pool;
    std::condition_variable has_work;
    std::mutex work_mutex;
    std::atomic<bool> quit {false};
    std::atomic<bool> finish {false};

    bool callback_queued {false};
    bool watch_hidden {false};
};

