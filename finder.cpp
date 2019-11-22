#include "finder.h"
#include "utils.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
#include <chrono>

finder::finder()
{
    for (std::size_t i = 0; i < countOfThreads; ++i)
    {
        pool.emplace_back(std::thread([this] {
            while (true)
            {
                std::unique_lock<std::mutex> lock(workMutex);
                hasWork.wait(lock, [this] { return quit || !queries.empty(); });

                if (quit)
                {
                    break;
                }

                if (finish.load()) {
                    continue;
                }

                lock.unlock();
                QString curPath = getQuery();

                if (curPath.isEmpty()) {
                    continue;
                }

                QFileInfo info(curPath);
                if (!info.exists())
                {
                    {
                        std::lock_guard<std::mutex> lock(workMutex);
                        QString tmp = "";
                        result.push_back(tmp.append("Invalid path : '")
                                            .append(curPath)
                                            .append("'"));
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    queue_callback();
                    continue;
                }
                if (info.isDir())
                {
                    processDir(curPath);
                }
                else if (info.isFile() && !info.isExecutable())
                {
                    processFile(curPath);
                }
            }
        }));
    }
}

void finder::processDir(const QString &path)
{
    if (finish.load())
    {
        return;
    }

    QDir dir(path);
    for (QFileInfo const& curInfo : dir.entryInfoList(QDir::Readable |
                                                      QDir::NoDotAndDotDot |
                                                      QDir::Dirs |
                                                      QDir::Files |
                                                      QDir::NoSymLinks))
    {
        if (finish.load())
        {
            break;
        }

        if (!isHidden(curInfo.absoluteFilePath()))
        {
            addQuery(curInfo.absoluteFilePath());
        }
    }
}

void finder::processFile(const QString &path)
{
    if (finish.load())
    {
        return;
    }

    QFile file(path);
    if (file.size() > MAX_SIZE)
    {
        processBigFile(file);
    }
    else
    {
        processSmallFile(file);
    }
}

void finder::processSmallFile(QFile &file)
{
    QFileInfo info(file);

    auto finishSearch = [this, &file, &info] (int line) {
        {
            std::lock_guard<std::mutex> lock(workMutex);
            lastStr.clear();
            lastStr.append(info.absoluteFilePath().append("[ Line = ")
                                                    .append(QString::number(line))
                                                    .append(" ]"));
            result.push_back(lastStr);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));
        queue_callback();
        file.close();
    };

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString data = "";
        data.append(mStr).append(UNREAL).append(stream.readAll());
        auto const& res = Utils::contains(data, mStr.size());
        if (res.first)
        {
            int ind = res.second - 2 * static_cast<int>(mStr.size());
            finishSearch(Utils::getLine(data, ind) + 1);
            return;
        }
    }
}

void finder::processBigFile(QFile &file)
{
    QFileInfo info(file);

    auto finishSearch = [this, &file, &info] (int line) {
        {
            std::lock_guard<std::mutex> lock(workMutex);
            lastStr.clear();
            lastStr.append(info.absoluteFilePath().append("[ Line = ")
                                                    .append(QString::number(line))
                                                    .append(" ]"));
            result.push_back(lastStr);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));
        queue_callback();
        file.close();
    };

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        const int SIZE = std::max(MAX_SIZE, 2 * mStr.size());
        QString prev = "";
        int line = 0;
        int index = 0;
        while (!stream.atEnd())
        {
            if (finish.load()) {
                file.close();
                break;
            }
            QString buf = stream.read(SIZE);
            QString nStr;
            if (!prev.isEmpty())
            {
                nStr.clear();
                nStr.append(mStr).append(UNREAL).append(prev).append(buf.leftRef(mStr.size()).toString());
                auto const& res = Utils::contains(nStr, mStr.size());
                if (res.first)
                {
                    int ind = res.second - 2 * mStr.size();
                    finishSearch(line + Utils::getLine(nStr, ind) + 1);
                    return;
                }
                index += prev.size();
                line += Utils::getLine(prev, prev.size());
            }
            nStr.clear();
            nStr.append(mStr).append(UNREAL).append(buf);
            auto const& res = Utils::contains(nStr, mStr.size());
            if (res.first)
            {
                int ind = res.second - 2 * mStr.size();
                finishSearch(line + Utils::getLine(nStr, ind) + 1);
                return;
            }
            index += buf.size();
            line += Utils::getLine(buf, buf.size() - prev.size());
            prev = buf.rightRef(mStr.size()).toString();
        }
    }
}

finder::~finder()
{
    finish.store(true);
    {
        std::lock_guard<std::mutex> lock(workMutex);
        quit = true;
        hasWork.notify_all();
    }

    for (auto& pItem : pool)
    {
        if (pItem.joinable())
        {
            pItem.join();
        }
    }
}

void finder::addQuery(const QString &path)
{
    std::lock_guard<std::mutex> lock(workMutex);
    queries.push(path);
    hasWork.notify_all();
}

QString finder::getQuery()
{
    std::lock_guard<std::mutex> lock(workMutex);
    QString res = "";
    if (!queries.empty())
    {
        res = queries.front();
        queries.pop();
    }
    return res;
}

void finder::clearQueies()
{
    std::lock_guard<std::mutex> lock(workMutex);
    while (!queries.empty())
    {
        queries.pop();
    }
}

void finder::setStr(const QString &str)
{
    std::lock_guard<std::mutex> lock(workMutex);
    mStr = str;
}

void finder::setHidden(bool hidden)
{
    std::lock_guard<std::mutex> lock(workMutex);
    watchHidden = hidden;
}

void finder::run(const QString &str, const QString &path, bool hidden)
{
    stop();
    finish.store(false);
    clearQueies();
    addQuery(path);
    setStr(str);
    setHidden(hidden);
    result.clear();
    hasWork.notify_all();
}

void finder::stop()
{
    finish.store(true);
    clearQueies();
    setStr("");
}

void finder::queue_callback()
{
    if (callback_queued)
    {
        return;
    }

    callback_queued = true;
    QMetaObject::invokeMethod(this, &finder::callback, Qt::QueuedConnection);
}

void finder::callback()
{
    {
        std::lock_guard<std::mutex> lock(workMutex);
        callback_queued = false;
    }

    emit result_changed();
}

std::vector<QString> finder::getResult() const
{
    return result;
}

bool finder::isHidden(QString const& name)
{
    return watchHidden ? false : name[0] == '.';
}

QString finder::getLastRes() const
{
    return lastStr;
}
