#include "finder.h"

#include <QDir>
#include <queue>

finder::result::result()
    : done_work(true)
{}

finder::finder()
    : m_sub_str("")
    , quit(false)
    , callback_queued(false)
    , cancel(false)
    , thread([this]
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(m);
            has_work.wait(lock,
                          [this]
                          { return !m_sub_str.isEmpty() || quit; });
            if (quit)
            {
                break;
            }
            QString sub_str(m_sub_str);
            cur_res.done_work = false;
            cur_res.files.clear();
            queue_callback();

            lock.unlock();
            find(QDir::homePath(), sub_str);
            lock.lock();
            cur_res.done_work = cancel.load();
            queue_callback();

            if (!cancel.load())
            {
                m_sub_str = "";
            }
            cancel.store(false);
        }
    })
{}

finder::~finder()
{
    cancel.store(true);
    {
        std::unique_lock<std::mutex> lg(m);
        quit = true;
        has_work.notify_all();
    }
    thread.join();
}

void finder::set_substring(const QString &sub_str)
{
    std::unique_lock<std::mutex> lg(m);
    if (!m_sub_str.isEmpty())
    {
        cancel.store(true);
    }
    m_sub_str = sub_str;
    has_work.notify_all();
}

finder::result finder::get_result() const
{
    std::lock_guard<std::mutex> lg(m);
    return cur_res;
}

void finder::find(QString const& path, const QString &sub_str)
{
    if (sub_str.isEmpty())
    {
        return;
    }

    std::queue<QString> q;
    q.push(path);
    while (!q.empty())
    {
        if (cancel.load())
        {
            break;
        }
        QString cur_p = q.front();
        q.pop();
        QDir dir(cur_p);
        std::size_t cnt = 0;
        for (auto const& d : dir.entryInfoList())
        {
            if (d.fileName().at(0) != '.')
            {
                if (d.isDir())
                {
                    q.push(d.absoluteFilePath());
                }
                else if (d.isFile() )
                {
                    std::unique_lock<std::mutex> lock(m);
                    cur_res.files.emplace_back(d.fileName());

                    if (++cnt % 4096 == 0)
                    {
                        queue_callback();
                    }
                }
            }
        }
    }
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
        std::unique_lock<std::mutex> lg(m);
        callback_queued = false;
    }

    emit result_changed();
}
