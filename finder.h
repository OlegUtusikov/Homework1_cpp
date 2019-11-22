#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <QFile>

class finder : public  QObject
{
    Q_OBJECT

public:
    finder();
    ~finder();

    void run(QString const& str, QString const& path, bool hidden);
    void stop();

    std::vector<QString> getResult() const;
    QString getLastRes() const;

signals:
    void result_changed();

private:
    void queue_callback();
    void callback();
    void clearQueies();
    void addQuery(QString const& path);
    QString getQuery();
    void setStr(QString const& str);
    void setHidden(bool hidden);
    void find(QString const& path);
    void processDir(QString const& path);
    void processFile(QString const& path);

    void processBigFile(QFile& file);
    void processSmallFile(QFile& file);

    bool isHidden(QString const& name);

private:
    const QString UNREAL = "|";
    const int TIME_SLEEP = 5;
    const int MAX_SIZE = 4 * 1024 * 1024;
    QString mStr;
    std::vector<QString> result;
    QString lastStr {""};
    std::queue<QString> queries;
    const std::size_t countOfThreads = 8;
    std::vector<std::thread> pool;
    std::condition_variable hasWork;
    std::mutex workMutex;
    std::atomic<bool> quit {false};
    std::atomic<bool> finish {false};

    bool callback_queued {false};
    bool watchHidden {false};
};

