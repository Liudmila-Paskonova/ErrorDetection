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

struct Parameters : public argparser::Arguments {
    std::string prob;
    size_t npairs;
    std::string dir;
    std::string metadata;
    std::string lang;
    std::string outdir;
    std::string stmts;

    Parameters()
    {
        using namespace argparser;
        addParam<"-prob", "--problem_name">(prob, CostrainedArgument<std::string>("p02743", {"p02743"}));
        addParam<"-npairs", "--pairs_count">(npairs, NaturalRangeArgument<>(1, {1, 4000}));
        addParam<"-dir", "--raw_dataset">(
            dir, DirectoryArgument<std::string>("/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/C++"));
        addParam<"-stmts", "--problem_statements">(
            stmts, DirectoryArgument<std::string>("/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/texts"));
        addParam<"-metadata", "--path_to_metadata">(
            metadata, FileArgument<std::string>(
                          "/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/C++/metadata_cpp.db"));
        addParam<"-lang", "--dataset_language">(lang, CostrainedArgument<std::string>("cpp", {"c", "cpp"}));
        addParam<"-outdir", "--output_directory">(
            outdir,
            DirectoryArgument<std::string>("/home/liudmila/ssd-drive/Coursework_dataset/Project_CodeNet/labels2"));
    }
};

namespace fs = std::filesystem;

int
main(int argc, char *argv[])
{

    try {
        Parameters params;
        params.parse(argc, argv);

        db::Database db(params.metadata, "metadata_cpp");
        auto subPairs = db.getPairs(params.prob, params.npairs, params.lang);
        std::vector<std::set<size_t>> errorLines;

        fs::path dataDir = params.dir;
        fs::path stmtDir = params.stmts;
        fs::path stmt = stmtDir / (params.prob + ".txt");
        fs::path probDir = dataDir / params.prob;
        fs::path outDir = params.outdir;
        fs::path outFile = outDir / (params.prob + ".txt");

        std::ifstream stmtFile(stmt);
        std::stringstream stmtBuf;
        stmtBuf << stmtFile.rdbuf();
        stmtFile.close();

        for (const auto &[ok, pt] : subPairs) {
            std::string okFileStr = (probDir / ok).string();
            std::string ptFileStr = (probDir / pt).string();

            treesitter::TreeSitter okTree(okFileStr, params.lang), ptTree(ptFileStr, params.lang);

            auto ptRt = root2leafPaths(ptTree.getRoot());
            std::unordered_map<std::string, std::vector<treesitter::TreeSitterNode>> nodes;
            for (auto &vec : ptRt) {
                std::string newId;
                for (auto &v : vec) {
                    newId += std::format("{:0>3}", v.getID());
                }
                newId += vec.back().getValue(ptTree.getContext());
                nodes[newId].push_back(vec.back());
            }

            auto okRt = root2leafPaths(okTree.getRoot());

            for (auto &vec : okRt) {
                std::string newId;
                for (auto &v : vec) {
                    newId += std::format("{:0>3}", v.getID());
                }
                newId += vec.back().getValue(okTree.getContext());
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

        marker::Marker window(stmtBuf.str(), probDir.string(), subPairs, errorLines, outFile.string());

        window.show();

        return app.exec();

    } catch (const std::string &err) {
        std::println("{}", err);
    }
}
