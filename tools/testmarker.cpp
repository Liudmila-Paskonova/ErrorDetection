#include <support/Database/Metadata.h>
#include <support/TreeSitter/TreeSitter.h>
#include <support/ArgParser/ArgParser.h>
#include <visualizer/Marker.h>
#include <print>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <set>

argparser::ArgTuple cmdLineArgs = {
    argparser::Argument<std::string>("--problem_name", "-prob", "p02743"),
    argparser::Argument<size_t>("--pairs_count", "-npairs", 10),
    argparser::Argument<std::string>("--raw_dataset", "-dir",
                                     "/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/C++"),
    argparser::Argument<std::string>("--path_to_metadata", "-metadata",
                                     "/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/C++/metadata_cpp.db"),
    argparser::Argument<std::string>("--problem_statements", "-stmts",
                                     "/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/texts"),
    argparser::Argument<std::string>("--dataset_language", "-lang", "cpp", {"c", "cpp"}),
    argparser::Argument<std::string>("--output_file", "-out", "/home/liudmila/cmc/Coursework_final/src/output.txt")};

struct Parameters : public argparser::ParametersBase {
    std::string prob;
    size_t npairs;
    std::string dir;
    std::string metadata;
    std::string lang;
    std::string out;
    std::string stmts;

    Parameters(std::vector<argparser::Data> &m) : ParametersBase(m)
    {
        getParam("-prob", prob);
        getParam("-npairs", npairs);
        getParam("-dir", dir);
        getParam("-stmts", stmts);
        getParam("-metadata", metadata);
        getParam("-lang", lang);
        getParam("-out", out);
    }
};

namespace fs = std::filesystem;

int
main(int argc, char *argv[])
{

    try {
        auto parser = argparser::make_parser<Parameters>(cmdLineArgs);
        auto params = parser.parse(argc, argv);

        db::Database db(params.metadata, "metadata_cpp");
        auto subPairs = db.getPairs(params.prob, params.npairs, params.lang);
        std::vector<std::set<size_t>> errorLines;

        fs::path dataDir = params.dir;
        fs::path stmtDir = params.stmts;
        fs::path stmt = stmtDir / (params.prob + ".txt");
        fs::path probDir = dataDir / params.prob;

        std::ifstream stmtFile(stmt);
        std::stringstream stmtBuf;
        stmtBuf << stmtFile.rdbuf();
        stmtFile.close();

        for (const auto &[ok, pt] : subPairs) {
            std::ifstream okFile(probDir / ok);
            std::stringstream bufferOk;
            bufferOk << okFile.rdbuf();
            okFile.close();
            std::ifstream ptFile(probDir / pt);
            std::stringstream bufferPt;
            bufferPt << ptFile.rdbuf();
            ptFile.close();

            std::string okBody = bufferOk.str(), ptBody = bufferPt.str();

            treesitter::TreeSitter okTree(okBody, params.lang), ptTree(ptBody, params.lang);

            auto ptRt = treesitter::root2leafPaths(ptTree.getRoot(), false);
            std::unordered_map<std::string, std::vector<treesitter::TreeSitterNode>> nodes;
            for (auto &vec : ptRt) {
                std::string newId;
                for (auto &v : vec) {
                    newId += std::format("{:0>3}", v.getID());
                }
                newId += vec.back().getValue(ptBody);
                nodes[newId].push_back(vec.back());
            }

            auto okRt = treesitter::root2leafPaths(okTree.getRoot(), false);

            for (auto &vec : okRt) {
                std::string newId;
                for (auto &v : vec) {
                    newId += std::format("{:0>3}", v.getID());
                }
                newId += vec.back().getValue(okBody);
                auto it = nodes.find(newId);
                if (it != nodes.end()) {
                    nodes.erase(it);
                }
            }
            std::set<size_t> lines;

            for (auto &[path, node] : nodes) {
                for (auto &n : node) {
                    lines.insert(n.getStartPoint().first);
                }
            }

            errorLines.push_back(lines);
        }
        QApplication app(argc, argv);

        marker::Marker window(stmtBuf.str(), probDir.string(), subPairs, errorLines, params.out);

        window.show();

        return app.exec();

    } catch (const std::string &err) {
        std::println("{}", err);
    }
}
