#pragma once

#include <vector>
#include <QString>

class Utils
{
public:
    static  std::pair<bool, int> contains(QString const& str, int size);
    static int getLine(QString const& str, int ind);
};
