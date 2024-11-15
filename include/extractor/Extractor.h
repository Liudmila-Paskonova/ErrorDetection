#ifndef EXTRACTOR_EXTRACTOR_H
#define EXTRACTOR_EXTRACTOR_H

#include <support/TreeSitter/TreeSitter.h>
#include <support/Database/Metadata.h>
#include <support/ThreadPool/ThreadPool.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>
#include <map>
#include <ranges>
#include <concepts>
#include <chrono>
#include <mutex>

namespace extractor
{
std::mutex mut;

// Function that extracts all triplets (<token><path><token>)
// >> file - source file name
// >> db - path to metadata.db
template <typename Parameters>
void
extract(const std::filesystem::path &file, db::Database &db, const Parameters &params)
{
    std::string fileName = file.filename().stem();
    auto pack = db.getPackage(fileName);

    std::stringstream answer;
    answer << pack.probID;

    std::ifstream f(file);
    std::stringstream buffer;
    buffer << f.rdbuf();
    f.close();
    std::string strBuf = buffer.str();

    treesitter::TreeSitter t(strBuf, params.lang, params.tokens);

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
            pathUp += std::format("{:0>3}", t.maybeChangeID(topR2Lnode));
            if (rl >= params.maxLen) {
                break;
            }
            // for each next (after previous topR2Lnode) child of topR2Lnode...
            for (size_t j = nodeChildNum + 1; j < topR2Lnode.getChildCount(); j++) {
                // ... get all root2leaf paths...
                auto v = root2leafPaths(topR2Lnode.getChild(j), false);

                for (auto &ex : v) {
                    if (rl + ex.size() > params.maxLen) {
                        continue;
                    }
                    if (widthCounter[rl + ex.size()] > params.maxWidth) {
                        continue;
                    }
                    widthCounter[rl + ex.size()]++;

                    std::string pathDown;
                    for (size_t ind = 0; ind < ex.size() - 1; ind++) {
                        pathDown += std::format("{:0>3}", t.maybeChangeID(ex[ind]));
                    }
                    answer << " " << t.getFormat(firstTerm, pathUp, pathDown, ex[ex.size() - 1]);
                }
            }
            nodeChildNum = root2leaf[rl].getNodeNum();
        }
    }

    answer << "\n";
    {
        std::lock_guard tr(mut);
        std::cout << answer.str();
    }
}

template <typename Parameters> class Extractor
{

    Parameters params;

  public:
    Extractor(const Parameters &p) : params(p) {};

    // Function that runs extractor
    // >> dir - path to directory or file
    // >> batchSize - size of batch == number of files one thread is going to process
    // >> numThreads - number of threads available

    void
    run(const std::string &dir, const std::string &metadata)
    {
        std::filesystem::path dirPath = dir;
        if (!std::filesystem::exists(dirPath)) {
            throw "No such data dir!";
        }
        if (!std::filesystem::is_directory(dirPath)) {
            throw "is not a directory!";
        }

        db::Database db(metadata);

        std::vector<std::filesystem::path> filePaths;
        for (auto const &dir_entry : std::filesystem::directory_iterator{dirPath}) {
            filePaths.push_back(dir_entry.path());
        }
        {
            threadpool::ThreadPool pool(params.numThreads);
            for (auto &file : filePaths) {
                auto res = pool.addTask(extractor::extract<Parameters>, std::ref(file), std::ref(db), std::ref(params));
            }
        }
    }
};
} // namespace extractor

#endif
