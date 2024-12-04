/*#########################################################################################################//
Tool for dataset preprocessing

Supposing the following structure of a raw dataset,
  |- dataset_directory
  |----|- prob1_name
  |    |---- sub1_name
  |    |---- sub2_name
  |      ...
  |---- prob2_name
  |    |---- sub1_name
  |    |---- sub2_name
  |      ...
this tool will create a new dataset:
  |- train_split_directory
  |----|- train
  |    |---- sub1_name
  |    |---- sub2_name
  |----|- val
  |    |---- sub1_name
  |    |---- sub2_name

  --number_of_problems      |-nprobs    |= number of problems prob1_name, prob2_name... extracted
  --number_of_submissions   |-nsubs     |= number of submissions extracted from each problem folder
  --dataset_directory       |-datadir   |= folder with original files
  --train_split_directory   |- traindir |= folder for train and validation datasets
  --split_train_val         |- split    |= train-val split, %, e.g. 75% means train:val = 3:1 segmentation
//#########################################################################################################*/

#include <support/Support/Support.h>
#include <support/ArgParser/ArgParser.h>
#include <filesystem>
#include <algorithm>
#include <random>
#include <vector>
#include <numeric>
#include <set>
#include <print>
#include <format>

struct Parameters : public argparser::Arguments {
    size_t numProbs;
    size_t numSubs;
    size_t split;
    std::string dataDir;
    std::string outDir;

    Parameters()
    {
        using namespace argparser;

        addParam<"-nprobs", "--number_of_problems">(numProbs, NaturalRangeArgument<>(UINT32_MAX, {0, UINT32_MAX}));
        addParam<"-nsubs", "--number_of_submissions">(numSubs, NaturalRangeArgument<>(UINT32_MAX, {0, UINT32_MAX}));
        addParam<"-split", "--split_train_val">(split, NaturalRangeArgument<>(75, {0, 100}));
        addParam<"-datadir", "--dataset_directory">(dataDir, DirectoryArgument<std::string>("/home"));
        addParam<"-outdir", "--train_split_directory">(outDir, DirectoryArgument<std::string>("/home"));
    }
};

namespace fs = std::filesystem;

int
main(int argc, char *argv[])
{
    try {
        Parameters p;
        p.parse(argc, argv);

        if (!fs::exists(p.dataDir)) {
            throw "No such data dir!";
        }
        if (!fs::exists(p.outDir)) {
            throw "No such output dir!";
        }
        fs::path dataDir = p.dataDir;
        fs::path outDir = p.outDir;

        fs::path trainDir = outDir / "train";
        if (!fs::exists(trainDir)) {
            fs::create_directory(trainDir);
        }
        fs::path valDir = outDir / "val";
        if (!fs::exists(valDir)) {
            fs::create_directory(valDir);
        }

        auto cmp = [](const std::pair<unsigned long long, std::string> &a,
                      const std::pair<unsigned long long, std::string> &b) { return a.first > b.first; };

        // find the nprobs problems with the most files
        std::set<std::pair<unsigned long long, std::string>, decltype(cmp)> problemsRank;
        for (const auto &prob : fs::directory_iterator(dataDir)) {
            if (prob.is_directory()) {
                auto name = prob.path().filename();
                unsigned long long count = 0;
                for (const auto &sub : fs::directory_iterator(prob.path())) {
                    ++count;
                }

                if (problemsRank.size() < p.numProbs || (std::prev(problemsRank.end())->first < count)) {
                    problemsRank.insert({count, name});
                    if (problemsRank.size() > p.numProbs) {
                        problemsRank.erase(std::prev(problemsRank.end()));
                    }
                }
            }
        }
        unsigned long long filesTotal = 0;
        // copy selected files to the outdir
        for (const auto &[count, probName] : problemsRank) {
            auto probPath = dataDir / probName;
            auto vec = support::getNRandomFiles(probPath, p.numSubs);
            for (const auto &v : vec) {
                fs::copy(v, outDir, fs::copy_options::overwrite_existing);
            }
            filesTotal += vec.size();
        }
        // get number of files in a train folder
        auto trainNum = p.split * (filesTotal / 100);

        auto trainVec = support::getNRandomFiles(outDir, trainNum);

        // move files to train and validation folders
        for (const auto &v : trainVec) {
            fs::copy(v, trainDir, fs::copy_options::overwrite_existing);
            fs::remove(v);
        }
        for (const auto &sub : fs::directory_iterator(outDir)) {
            if (sub.is_regular_file()) {
                fs::copy(sub.path(), valDir, fs::copy_options::overwrite_existing);
                fs::remove(sub.path());
            }
        }

    } catch (const std::string &s) {
        std::println("{}", s);
        exit(1);
    }
}
