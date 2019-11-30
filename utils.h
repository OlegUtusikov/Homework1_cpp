#pragma once

#include <vector>
#include <string>

#include <QString>


class Utils
{
public:
    static std::vector<std::pair<int, QString>> contains(QString const& str,
                                                         int size);
    static int getLine(QString const& str, int ind);
};
