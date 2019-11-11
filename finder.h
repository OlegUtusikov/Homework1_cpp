#pragma once

#include <QObject>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <QDir>

struct finder : QObject
{
private:
    Q_OBJECT

public:
    struct result
    {
        result();

        std::vector<QString> files;
        bool done_work;
    };

    finder();
    ~finder();

    void set_substring(QString const& sub_str);
    void set_path(QString const& path);
    result get_result() const;

signals:
    void result_changed();

private:
    void find(QString const& path, QString const& sub_str);
    void queue_callback();
    void callback();

private:
    mutable std::mutex m;
    QString m_sub_str;
    bool quit;
    bool callback_queued;
    std::atomic<bool> cancel;
    std::condition_variable has_work;
    result cur_res;
    std::thread thread;
};
