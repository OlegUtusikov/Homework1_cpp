#include "utils.h"

std::pair<bool, int> Utils::contains(QString const& str, int size)
{
    std::vector<int> res(static_cast<std::size_t>(str.size()));

    res[0] = 0;
    for (int i = 1; i < static_cast<int>(res.size()); ++i)
    {
        int j = res[static_cast<std::size_t>(i - 1)];
        while (j > 0 && str.at(i) != str.at(j))
        {
            j = res[static_cast<std::size_t>(j - 1)];
        }
        if (str.at(i) == str.at(j))
        {
            ++j;
        }
        res[static_cast<std::size_t>(i)] = j;
        if (j == size)
        {
            return {true, i};
        }
    }
    return {false, -1};
}

int Utils::getLine(QString const& str, int ind)
{
    int res = 0;
    for (int i = 0; i < ind; ++i)
    {
        if (str.at(i) == '\n')
        {
            ++res;
        }
    }
    return res;
}
