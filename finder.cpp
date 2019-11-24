#include "finder.h"
#include "utils.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
#include <chrono>

finder::finder()
{
    for (std::size_t i = 0; i < count_of_threads; ++i)
    {
        pool.emplace_back(std::thread([this] {
            while (true)
            {
                std::unique_lock<std::mutex> lock(work_mutex);
                has_work.wait(lock, [this] { return quit || !queries.empty(); });

                if (quit || finish.load())
                {
                    break;
                }

                lock.unlock();
                QString curPath = get_query();

                if (curPath.isEmpty()) {
                    continue;
                }

                QFileInfo info(curPath);
                if (!info.exists())
                {
                    {
                        std::lock_guard<std::mutex> lock(work_mutex);
                        QString tmp = "";
                        tmp.append("Invalid path : '")
                           .append(curPath)
                           .append("'");
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));
                    queue_callback();
                    continue;
                }
                if (!is_hidden(info.fileName()))
                {
                    if (info.isDir())
                    {
                        process_dir(curPath);
                    }
                    else if (info.isFile() && !info.isExecutable())
                    {
                        process_file(curPath);
                    }
                }
            }
        }));
    }
}

void finder::process_dir(const QString &path)
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
        add_query(curInfo.absoluteFilePath());
    }
}

void finder::process_file(const QString &path)
{
    if (finish.load())
    {
        return;
    }

    QFile file(path);
    find_in_file(file);
}

void finder::print_str(const QFileInfo &info, QFile &file, int line, QString const& res)
{
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        lastStr.clear();
        lastStr.append(info.absoluteFilePath().append(" : ")
                                                .append(QString::number(line))
                                                .append("\n"))
                                                .append("---------------------------\n")
                                                .append(res).append("\n\n");
    }

    queue_callback();
    file.close();
}

void finder::find_in_file(QFile &file)
{
    QFileInfo info(file);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        const int SIZE = std::max(MAX_SIZE, 2 * m_str.size());
        QString prev = "";
        int line = 0;
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
                QString n_buf = prev.append(buf.left(m_str.size()));
                nStr.append(m_str).append(UNREAL).append(n_buf);
                auto const& res = Utils::contains(nStr, m_str.size());
                if (res.first != -1)
                {
                    int ind = res.first - 2 * m_str.size() - UNREAL.size() + 1;
                    print_str(info, file, line + Utils::getLine(n_buf, ind) + 1, res.second);
                    return;
                }
                line += Utils::getLine(prev, prev.size()) + 1;
            }
            nStr.clear();
            nStr.append(m_str).append(UNREAL).append(buf);
            auto const& res = Utils::contains(nStr, m_str.size());
            if (res.first != -1)
            {
                int ind = res.first - 2 * m_str.size() - UNREAL.size() + 1;
                print_str(info, file, line + Utils::getLine(buf, ind) + 1, res.second);
                return;
            }
            line += Utils::getLine(buf, buf.size() - prev.size()) + 1;
            prev = buf.rightRef(m_str.size()).toString();
        }
    }
}

finder::~finder()
{
    quit.store(true);
    finish.store(true);
    has_work.notify_all();

    for (auto& pItem : pool)
    {
        if (pItem.joinable())
        {
            pItem.join();
        }
    }
}

void finder::add_query(const QString &path)
{
    if (finish.load())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(work_mutex);
    queries.push(path);
    has_work.notify_all();
}

QString finder::get_query()
{
    if (finish.load())
    {
        return "";
    }

    QString res = "";
    std::lock_guard<std::mutex> lock(work_mutex);
    if (!queries.empty())
    {
        res = queries.front();
        queries.pop();
    }
    return res;
}

void finder::clear_queies()
{
    std::lock_guard<std::mutex> lock(work_mutex);
    while (!queries.empty())
    {
        queries.pop();
    }
}

void finder::set_str(const QString &str)
{
    std::lock_guard<std::mutex> lock(work_mutex);
    m_str = str;
}

void finder::set_hidden(bool hidden)
{
    std::lock_guard<std::mutex> lock(work_mutex);
    watch_hidden = hidden;
}

void finder::run(const QString &str, const QString &path, bool hidden)
{
    set_str(str);
    set_hidden(hidden);
    add_query(path);
    has_work.notify_all();
}

void finder::queue_callback()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));

    if (finish.load() || callback_queued)
    {
        return;
    }

    callback_queued = true;
    QMetaObject::invokeMethod(this, &finder::callback, Qt::QueuedConnection);
}

void finder::callback()
{
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        callback_queued = false;
    }

    emit result_changed();
}

bool finder::is_hidden(QString const& name)
{
    return watch_hidden || name.isEmpty() ? false : name[0] == '.';
}

QString finder::get_last_res() const
{
    return lastStr;
}
