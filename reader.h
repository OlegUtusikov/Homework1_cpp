#include <QFile>
#include <memory>
#include <vector>
#include <unordered_set>
#include <QDir>

#pragma once

class Reader
{
public:
    const QPair<int, int> WRONG_POS {0, 0};

    Reader();
    QPair<int, int> find(QString const& str);
    void set_path (QString const& path);
    ~Reader();

private:
    const std::size_t SIZE_BUFFER = 8 * 1024;
    const char NEW_LINE = '\n';
    QString m_path {""};
    std::vector<char> buffer;
};
