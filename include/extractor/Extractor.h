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
#include <unordered_map>

namespace extractor
{
/// lock
std::mutex mut;

// Function that extracts all triplets (<token><path><token>)
// >> file - source file name
// >> db - path to metadata.db
template <typename Parameters>
void
extract(const std::filesystem::path &file, const Parameters &params)
{
    treesitter::Tree t(file, params.lang, params.traversal, params.token, params.split);
    auto res = t.process();
    std::string line = file.filename().stem();
    for (const auto &v : res) {
        line += " " + v;
    }
    line += "\n";
    auto id = std::this_thread::get_id();
    std::filesystem outFile = params.outdir / "temp" / "tokens" / (std::to_string(id) + ".txt");
    std::ofstream tempFile(outFile);
    outFile << line;
    outFile.close();
    std::filesystem outVocab = params.outdir / "temp" / "vocabs" / (std::to_string(id) + ".txt");
    std::ofstream tempVocab(outVocab);
    outVocab << line;
    for (auto &[hash, tok] : t.vocab) {
        outVocab << hash << " " << tok << "\n";
    }
    outVocab.close();
}

class Extractor
{

  public:
    // Function that runs extractor
    // >> dir - path to directory or file
    // >> batchSize - size of batch == number of files one thread is going to process
    // >> numThreads - number of threads available

    template <typename Parameters>
    void
    run(const Parameters &params)
    {
        std::filesystem::path dirPath = params.dir;
        std::filesystem::path outDirPath = params.outdir;

        auto dirName = dirPath.filename().stem();
        std::filesystem::path tokensDir = outDirPath / dirName;
        std::filesystem::create_directory(tokensDir);
        std::filesystem::create_directory(tokensDir / "temp" / "tokens");
        std::filesystem::create_directory(tokensDir / "temp" / "vocabs");

        std::vector<std::filesystem::path> filePaths;
        for (auto const &dir_entry : std::filesystem::directory_iterator{dirPath}) {
            filePaths.push_back(dir_entry.path());
        }

        // run threadpool
        {
            threadpool::ThreadPool pool(params.numThreads);
            for (auto &file : filePaths) {
                auto res = pool.addTask(extractor::extract<Parameters>, std::ref(file), std::ref(params));
            }
        }

        // unite all files with path-contexts into one
        std::ofstream outFile(tokensDir / (params.traversal + "|" + params.token + "|" + params.split + "_tokens.txt"));
        for (auto const &dir_entry : std::filesystem::directory_iterator{tokensDir / "temp" / "tokens"}) {
            std::ofstream f(dir_entry.path());
            outFile << f.rdbuf();
            f.close();
        }
        outFile.close();
        std::filesystem::remove_all(tokensDir / "temp" / "tokens");

        // create a vocabulary
        std::unordered_map<size_t, std::string> globalVocab;
        for (auto const &dir_entry : std::filesystem::directory_iterator{tokensDir / "temp" / "vocabs"}) {
            std::ofstream f(dir_entry.path());
            std::string line;
            while (std::getline(f, line)) {
                std::istringstream lineStream(line);
                size_t number;
                std::string value;
                lineStream >> number;
                std::getline(lineStream >> std::ws, value);
                globalVocab[number] = value;
            }

            f.close();
        }
        std::filesystem::remove_all(tokensDir / "temp" / "vocabs");

        std::ofstream outVocab(tokensDir /
                               (params.traversal + "|" + params.token + "|" + params.split + "_mapping.txt"));
        for (auto &[hash, tok] : globalVocab) {
            outVocab << hash << " " << tok << "\n";
        }
        outVocab.close();
    }
};
} // namespace extractor

#endif
