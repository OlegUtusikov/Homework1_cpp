#include "utils.h"

std::vector<std::pair<int, QString>> Utils::contains(QString const& str,
                                                     int size)
{
    std::vector<int> res(static_cast<std::size_t>(str.size()));
    res[0] = 0;
    std::vector<std::pair<int, QString>> ans;

    for (std::size_t i = 1; i < res.size(); ++i)
    {
        int j = res[static_cast<std::size_t>(i - 1)];
        while (j > 0 && str.at(static_cast<int>(i)) != str.at(j))
        {
            j = res[static_cast<std::size_t>(j - 1)];
        }

        if (str.at(static_cast<int>(i)) == str.at(j))
        {
            ++j;
        }

        res[static_cast<std::size_t>(i)] = j;

        if (static_cast<int>(i) > size && j == size)
        {
            const int LEN = 64;
            int l = std::max(size + 2, static_cast<int>(i) - size - LEN);
            if (l == size + 2)
            {
                ans.push_back({i, str.mid(l, size + LEN)});
            }
            else
            {
                ans.push_back({i, str.mid(l, size + 2 * LEN)});
            }
        }
    }

    return ans;
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
