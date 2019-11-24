#pragma once

#include <vector>
#include <QString>

class Utils
{
public:
    static  std::pair<int, QString> contains(QString const& str, int size);
    static int getLine(QString const& str, int ind);
};
