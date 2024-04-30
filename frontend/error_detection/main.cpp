#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <vector>
#include "highlight.h"

const std::string prefix = "<!DOCTYPE html>\n\
<html lang=\"eng\">\n\
<head>\n\
    <meta charset=\"UTF-8\">\n\
    <title>test</title>\n\
    <style>\n\
        body {\n\
            font-family: Arial, sans-serif;\n\
        }\n\
        pre {\n\
            background-color: #d4ca9e;\n\
            padding: 10px;\n\
            border-radius: 8px;\n\
            border: 2px solid #000000;\n\
            border-color: #000000;\n\
            display: block;\n\
        }\n\
        .highlighted {\n\
            background-color: rgb(192, 86, 86);\n\
        }\n\
        .button-highlighted {\n\
            background-color: #1dc3ec;\n\
            color: black;\n\
        }\n\
    </style>\n\
</head>\n\
<body>\n\
    <h2>Error classes</h2>\n\
    <p>Choose top k classes to highlight.</p>\n\
";

const std::string suffix = "    <script>\n\
        document.addEventListener('DOMContentLoaded', function() {\n\
            const buttons = document.querySelectorAll('.highlight-button');\n\
        \n\
            buttons.forEach(button => {\n\
                button.addEventListener('click', function() {\n\
                    const alreadyActive = this.classList.contains('button-highlighted');\n\
                   \n\
                    buttons.forEach(btn => {\n\
                        btn.classList.remove('button-highlighted');\n\
                        btn.disabled = false;\n\
                    });\n\
        \n\
                    const targets = this.dataset.target.split(',');\n\
        \n\
                    targets.forEach(targetId => {\n\
                        const targetElement = document.getElementById(targetId.trim());\n\
                        if (targetElement) {\n\
                            targetElement.classList.toggle('highlighted');\n\
                        }\n\
                    });\n\
        \n\
                    if (alreadyActive) {\n\
                        this.classList.remove('button-highlighted');\n\
                        this.disabled = false; \n\
                    } else {\n\
                        this.classList.add('button-highlighted');\n\
                        this.disabled = false; \n\
                       \n\
                        buttons.forEach(btn => {\n\
                            if (btn !== this) {\n\
                                btn.disabled = true;\n\
                            }\n\
                        });\n\
                    }\n\
                });\n\
            });\n\
        });\n\
    </script>\n\
</body>\n\
</html>\n\
";

const std::string errorSample1 = "<span id=\"";
const std::string errorSample2 = "\" class=\"highlightable\">";
const std::string errorSample3 = "</span>";

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Wrong cmd parameters: test file name and list of error classes are required!\n";
        return 1;
    }
    // source file
    std::ifstream solution(argv[1]);
    std::stringstream buffer;
    std::string bufferStr;
    buffer << solution.rdbuf();
    bufferStr = buffer.str();
    solution.close();
    // file with clusters
    std::ifstream errors(argv[2]);

    uint32_t startToken1, endToken1, startToken2, endToken2, cluster;
    size_t k = 1;
    // mapping cluster to corresponding tokens
    std::map<uint32_t, std::pair<size_t, std::set<uint32_t>>> clusterContexts;
    // clusters -> tokens sorted by pos
    std::set<std::pair<uint32_t, size_t>> tokens;
    // clusters -> 1, 2, 3...
    std::map<uint32_t, std::set<uint32_t>> clusterString;
    std::map<uint32_t, std::string> strings;

    while (errors >> startToken1 >> endToken1 >> startToken2 >> endToken2 >> cluster) {
        if (clusterContexts.find(cluster) == clusterContexts.end()) {
            clusterContexts[cluster].first = k;
            ++k;
        }
        clusterContexts[cluster].second.insert(startToken1);
        clusterContexts[cluster].second.insert(startToken2);
        tokens.insert({startToken1, endToken1 - startToken1});
        tokens.insert({startToken2, endToken2 - startToken2});
    }

    for (auto &[cl, ind] : clusterContexts) {
        clusterString[ind.first] = std::move(ind.second);
    }
    std::set<uint32_t> allClusters;
    for (auto &[cl, ind] : clusterString) {
        allClusters.insert(ind.begin(), ind.end());
        std::string allFirst;
        for (auto &i : allClusters) {
            allFirst += std::to_string(i) + ",";
        }
        strings[cl] = allFirst;
    }

    // buttons fiels
    std::stringstream buttons;
    buttons << "<div>\n";
    for (auto &i : strings) {
        buttons << "<button class=\"highlight-button\" data-target=\"" << i.second << "\">" << i.first << "</button>\n";
    }
    buttons << "</div>\n";

    // text field
    std::stringstream text;
    text << "<pre id=\"code\">\n";
    uint32_t prev = 0;

    for (auto &[pos, n] : tokens) {
        text << bufferStr.substr(prev, pos - prev) << errorSample1 << pos << errorSample2 << bufferStr.substr(pos, n)
             << errorSample3;
        prev = pos + n;
    }
    text << bufferStr.substr(prev, bufferStr.size() - prev);
    text << "</pre>\n";

    std::stringstream ans;
    // include "<" in headers
    postprocessHTML headers(text.str());
    ans << prefix << buttons.str() << headers.getStr() << suffix;
    std::cout << ans.str();
}
