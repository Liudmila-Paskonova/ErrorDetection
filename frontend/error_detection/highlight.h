#include <cstdint>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>

class postprocessHTML
{
    std::string src;
    std::vector<uint32_t> indices;

  public:
    postprocessHTML(const std::string &s) : src(s)
    {
        const std::string literals = "abcdefghijklmnopqrstuvwxyz/";
        std::vector<std::pair<std::string, uint32_t>> stack;
        // position of the first '<'
        auto maybeHeader = src.find('<', 0);
        while (maybeHeader < src.length()) {
            auto k = std::min(src.find_first_not_of(literals, maybeHeader + 1),
                              std::min(src.find('>', maybeHeader + 1), src.find('<', maybeHeader + 1)));
            std::string maybeTag = src.substr(maybeHeader + 1, k - maybeHeader - 1);
            if (maybeTag.empty()) {
                // '< '
                indices.push_back(maybeHeader);
            } else if (maybeTag[0] == '/') {
                //</header>
                maybeTag.erase(0, 1);
                while (!stack.empty() && stack.back().first != maybeTag) {
                    indices.push_back(stack.back().second);
                    stack.pop_back();
                }
                if (!stack.empty() && stack.back().first == maybeTag) {
                    stack.pop_back();
                }
            } else {
                stack.push_back({maybeTag, maybeHeader});
            }

            maybeHeader = src.find('<', maybeHeader + 1);
        }
        while (!stack.empty()) {
            indices.push_back(stack.back().second);
            stack.pop_back();
        }
        std::sort(indices.begin(), indices.end());
    }
    // make < visible in html
    std::string
    getStr()
    {
        std::string res;
        uint32_t pos = 0;
        for (auto &i : indices) {
            res += src.substr(pos, i - pos) + "&lt;";
            pos = i + 1;
        }
        res += src.substr(pos, src.size() - pos);
        return res;
    }
};
