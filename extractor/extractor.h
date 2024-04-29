#include <iostream>
#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>
#include <map>
#include "threadPool.h"
#include "treeSitter.h"

class Extractor
{
  private:
    // these expressions' IDs should be referred to the corresponding operations
    std::map<std::string, uint32_t> expressions = {{"unary_expression", 0}, {"binary_expression", 1}};
    /*
    Token options
    0: Similar to JavaExtractor <token1, path, token2>
        - tokens - names,types and constants - are named (value)
        - strings are unnamed, "string" (type)
        - paths are hashed (std::hash) (ID)
    1: Possible improvement <token1, path, token2>
        - tokens - names - are unnamed (type)
        - types and constants are named (value)
        - strings are std::hash(value)
        - paths are hashed with 3-formatted indices of internal nodes (ID)
    */
    std::map<std::string, std::vector<int>> tokenOptions = {
        // getValue(), getType()
        {"identifier", {0, 1}}, // name (e.g. x, myFunc)
        // getValue(), getValue()
        {"primitive_type", {0, 0}}, // type (e.g. int, double)
        // getValue(), getValue()
        {"number_literal", {0, 0}}, // constant (e.g. 0, 1.43)
        // getType(), std::hash(getValue()) = 2
        {"string_content", {1, 2}} // string (e.g. "Hello", " ")
    };

    int option;
    size_t maxLen;
    size_t maxWidth;
    bool exportVectors;

    // format token depending on its type and option
    std::string
    getToken(const TreeSitterNode &token)
    {
        std::string ans;
        int p = 1;
        std::string t = token.getType();
        if (tokenOptions.find(t) != tokenOptions.end()) {
            p = tokenOptions[t][option];
        }
        switch (p) {
        case 0:
            ans = token.getValue();
            break;
        case 1:
            ans = t;
            break;
        case 2:
            ans = std::to_string(std::hash<std::string>{}(token.getValue()));
        default:
            break;
        }
        return ans;
    }

    // format path depending on the option
    std::string
    getPath(const std::string &s)
    {
        std::string ans;
        switch (option) {
        case 0:
            ans = std::to_string(std::hash<std::string>{}(s));
            break;
        case 1:
            ans = s;
            break;
        default:

            break;
        }
        return ans;
    }

    // get the representation for the extractor
    std::string
    getFormat(const TreeSitterNode &token1, const std::string &pathUp, const std::string &pathDown,
              const TreeSitterNode &token2)
    {
        std::string t1, p, t2;
        t1 = getToken(token1);
        t2 = getToken(token2);
        p = pathUp + pathDown;
        p = getPath(p);
        return t1 + "," + p + "," + t2;
    }

    // replace binary/unary ops with corresponding operation
    uint16_t
    maybeChangeID(const TreeSitterNode &node)
    {
        uint16_t ans = node.getID();
        std::string t = node.getType();
        if (expressions.find(t) != expressions.end()) {
            ans = node.getChild(expressions[t]).getID();
        }
        return ans;
    }

    // Function that obtains the taskName (file format: /some/path/[taskName][status]unique_id.c)
    // >> file - source file name
    std::tuple<std::string, std::string, std::string>
    getTaskAndStatusNames(const std::string &file)
    {
        auto nSlash = file.rfind('/');
        std::string task = file, status = file;
        // get the raw name (without directories)
        if (nSlash != file.npos) {
            task = file.substr(nSlash + 2);
        }
        auto nCloseBr1 = task.find(']');
        status = task.substr(nCloseBr1 + 2);
        task = task.substr(0, nCloseBr1);

        auto nCloseBr2 = status.find(']');
        auto name = status.substr(nCloseBr2 + 1);
        status = status.substr(0, nCloseBr2);

        auto dot = name.find('.');
        name = name.substr(0, dot);

        return {task, status, name};
    }

    // Function that extracts all triplets (<token><path><token>)
    // >> file - source file name
    std::string
    extract(const std::string &file)
    {
        std::stringstream answer;
        auto tup = getTaskAndStatusNames(file);
        answer << std::get<0>(tup);
        if (exportVectors) {
            answer << "|" << std::get<1>(tup) << "|" << std::get<2>(tup);
        }
        std::ifstream f(file);
        std::stringstream buffer;
        buffer << f.rdbuf();
        f.close();
        std::string strBuf = buffer.str();

        TreeSitter t(strBuf);

        auto root = t.getRoot();
        // Get all possible root-2-leaves pathes

        auto root2leaves = root2leafPaths(root, true);
        for (auto &root2leaf : root2leaves) {
            auto firstTerm = root2leaf[0];
            auto nodeChildNum = root2leaf[0].getNodeNum();
            // common part of each path is stored here
            std::string pathUp;
            // get all pathes from firstTerm leaf:
            for (size_t rl = 1; rl < root2leaf.size(); ++rl) {
                std::map<size_t, size_t> widthCounter;
                auto topR2Lnode = root2leaf[rl];
                // format to have all ids 3-digit (they're [1,350])
                pathUp += std::format("{:0>3}", maybeChangeID(topR2Lnode));
                if (rl >= maxLen) {
                    break;
                }
                // for each next (after previous topR2Lnode) child of topR2Lnode...
                for (size_t j = nodeChildNum + 1; j < topR2Lnode.getChildCount(); j++) {
                    // ... get all root2leaf paths...
                    auto v = root2leafPaths(topR2Lnode.getChild(j));

                    for (auto &ex : v) {
                        if (rl + ex.size() > maxLen) {
                            continue;
                        }
                        if (widthCounter[rl + ex.size()] > maxWidth) {
                            continue;
                        }
                        widthCounter[rl + ex.size()]++;

                        std::string pathDown;
                        for (size_t ind = 0; ind < ex.size() - 1; ind++) {
                            pathDown += std::format("{:0>3}", maybeChangeID(ex[ind]));
                        }
                        answer << " " << getFormat(firstTerm, pathUp, pathDown, ex[ex.size() - 1]);
                    }
                }
                nodeChildNum = root2leaf[rl].getNodeNum();
            }
        }

        answer << "\n";
        return answer.str();
    };

    // Function-wrapper for multithreading
    // >> dirNames - array of files' paths
    void
    task(std::vector<std::string> &dirNames)
    {
        std::string batched;
        for (auto &i : dirNames) {
            batched += extract(i);
        }
        std::cout << batched;
    }

  public:
    Extractor(int opt = 0, size_t len = UINT32_MAX, size_t width = UINT32_MAX, bool ev = false)
        : option(opt), maxLen(len), maxWidth(width), exportVectors{ev} {};

    // Function that runs extractor
    // >> dir - path to directory or file
    // >> batchSize - size of batch == number of files one thread is going to process
    // >> numThreads - number of threads available
    void
    run(const std::string &dir, size_t batchSize = 1, size_t numThreads = 1)
    {
        ThreadPool pool(numThreads);

        if (dir.empty()) {
            throw "No input files!";
        }
        std::filesystem::path dirPath = dir;
        if (exists(dirPath) && is_directory(dirPath)) {

            // DIRECTORY
            size_t k = 0;
            std::vector<std::string> dirNames;
            for (auto const &dir_entry : std::filesystem::directory_iterator{dirPath}) {
                auto fileName = dir_entry.path().string();
                // check that file has "c" extension
                if (fileName[fileName.size() - 1] != 'c') {
                    continue;
                }
                dirNames.push_back(fileName);
                ++k;
                if (k % batchSize == 0) {
                    pool.push(std::bind_front(&Extractor::task, this, dirNames));
                    dirNames.clear();
                }
            }

            if (!dirNames.empty()) {
                pool.push(std::bind_front(&Extractor::task, this, dirNames));
                dirNames.clear();
            }
        } else if (exists(dirPath) && is_regular_file(dirPath)) {
            // FILE
            std::stringstream answer;

            std::ifstream f(dirPath);
            std::stringstream buffer;
            buffer << f.rdbuf();
            f.close();
            std::string strBuf = buffer.str();

            TreeSitter t(strBuf);

            auto root = t.getRoot();
            // Get all possible root-2-leaves pathes

            auto root2leaves = root2leafPaths(root, true);
            for (auto &root2leaf : root2leaves) {
                auto firstTerm = root2leaf[0];
                auto nodeChildNum = root2leaf[0].getNodeNum();
                // common part of each path is stored here
                std::string pathUp;
                // get all pathes from firstTerm leaf:
                for (size_t rl = 1; rl < root2leaf.size(); ++rl) {
                    std::map<size_t, size_t> widthCounter;
                    auto topR2Lnode = root2leaf[rl];
                    // format to have all ids 3-digit (they're [1,350])
                    pathUp += std::format("{:0>3}", maybeChangeID(topR2Lnode));
                    if (rl >= maxLen) {
                        break;
                    }
                    // for each next (after previous topR2Lnode) child of topR2Lnode...
                    for (size_t j = nodeChildNum + 1; j < topR2Lnode.getChildCount(); j++) {
                        // ... get all root2leaf paths...
                        auto v = root2leafPaths(topR2Lnode.getChild(j));

                        for (auto &ex : v) {
                            if (rl + ex.size() > maxLen) {
                                continue;
                            }
                            if (widthCounter[rl + ex.size()] > maxWidth) {
                                continue;
                            }
                            widthCounter[rl + ex.size()]++;

                            std::string pathDown;
                            for (size_t ind = 0; ind < ex.size() - 1; ind++) {
                                pathDown += std::format("{:0>3}", maybeChangeID(ex[ind]));
                            }
                            auto lastTerm = ex[ex.size() - 1];
                            answer << firstTerm.getStartByte() << "|" << firstTerm.getEndByte() << "|"
                                   << lastTerm.getStartByte() << "|" << lastTerm.getEndByte() << " "
                                   << getFormat(firstTerm, pathUp, pathDown, ex[ex.size() - 1]) << "\n";
                        }
                    }
                    nodeChildNum = root2leaf[rl].getNodeNum();
                }
            }
            std::cout << answer.str();

        } else {
            throw "Wrong cmd params!";
        }
    }
};
