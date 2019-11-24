#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
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

signals:
    void result_changed();

private:
    void queue_callback();
    void callback();
    void clear_queies();
    void add_query(QString const& path);
    QString get_query();
    void set_str(QString const& str);
    void set_hidden(bool hidden);
    void find(QString const& path);

    void process_dir(QString const& path);
    void process_file(QString const& path);

    void find_in_file(QFile& file);

    void print_str(QFileInfo const& info, QFile& file, int line, QString const& res);

    bool is_hidden(QString const& name);

private:
    const QString UNREAL = "~";
    const int TIME_SLEEP = 10;
    const int MAX_SIZE = 8 * 1024 * 1024;
    QString m_str {""};
    QString lastStr {""};
    std::queue<QString> queries;
    const std::size_t count_of_threads = 4;
    std::vector<std::thread> pool;
    std::condition_variable has_work;
    std::mutex work_mutex;
    std::atomic<bool> quit {false};
    std::atomic<bool> finish {false};

    bool callback_queued {false};
    bool watch_hidden {false};
};

