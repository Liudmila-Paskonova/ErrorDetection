#ifndef SUPPORT_TREESITTER_TREESITTER_H
#define SUPPORT_TREESITTER_TREESITTER_H

#include <cstring>
#include <stack>
#include <vector>
#include <string>
#include <stack>
#include <algorithm>
#include <fcntl.h>
#include <cstdint>
#include <map>
#include <functional>
#include <unordered_map>
#include <format>
#include <cstdint>
#include <fstream>
#include <sstream>

namespace treesitter
{
extern "C" {
#include "tree_sitter/api.h"
}

extern "C" const TSLanguage *tree_sitter_c();
extern "C" const TSLanguage *tree_sitter_cpp();

/// Struct that represents one node
struct TokenizedToken {
    /// grammar identifier of a node
    std::string id;
    /// value or grammar type
    std::string name;
};

class TreeSitterNode
{
  private:
    TSNode node;
    uint32_t nodeNumAsChild;

  public:
    TreeSitterNode(const TSNode &a, uint32_t num = 0) : node(a), nodeNumAsChild(num) {};

    TreeSitterNode &operator=(const TreeSitterNode &other);

    bool operator==(const TreeSitterNode &b);

    // get the type of the node
    std::string getType() const;

    // get the value of the particular node
    std::string getValue(const std::string_view &src) const;

    // get the start byte of the node
    uint32_t getStartByte() const;

    // get the end byte of the node
    uint32_t getEndByte() const;

    // get the start point (row, column) of the node
    std::pair<uint32_t, uint32_t> getStartPoint() const;

    // get the end point (row, column) of the node
    std::pair<uint32_t, uint32_t> getEndPoint() const;

    // get number of child nodes
    uint32_t getChildCount() const;

    // get the child number this node's parent has for this node
    uint32_t getNodeNum() const;

    // get the node ID
    uint16_t getID() const;

    // get the particular child node
    TreeSitterNode getChild(uint32_t i) const;

    // check if internal node is leaf
    bool isTerminal() const;

    // check if internal node is named leaf
    bool isNamedTerminal() const;

    // check if internal node has >= 2 children
    bool isFork() const;
};

/// Class that stores traversal policies
class Traversal
{
    /// A function to get all possible node-terminal (or terminal-node) sequences for a node
    /// @param node - a given node
    /// @param reverseArr - reverse the resulting vector if needed
    /// @return vector of nodes' sequences
    static std::vector<std::vector<TSNode>> getAllNode2TerminalPaths(const TSNode &node, bool reverseArr = false);

  public:
    /// A function to get all possible root-terminal sequences
    /// @param root - the root of a tree
    /// @return vector of sequences of nodes
    static std::vector<std::vector<TSNode>> root2terminal(const TSNode &root);

    /// A function to get all possible terminal-terminal sequences
    /// @param root - the root of a tree
    /// @return vector of sequences of nodes
    static std::vector<std::vector<TSNode>> terminal2terminal(const TSNode &root);
};

/// Class that stores tokenization methods
class Tokenizer
{
  public:
    /// A function that collects information (id, name) about a particular node
    /// @param node - a given node
    /// @param src - file' context (required to extract exact values)
    /// @return TokenizedToken
    static TokenizedToken defaultTokenization(const TSNode &node, const std::string &src);
};

/// Class that stores split strategies
class Split
{
  public:
    /// A function that converts a sequence of tokens to a string representing path from root to some token
    /// @param pathContext - a vector of tokens to process
    /// @param vocab - a vocabulary that stores mapping between terminal names and their hashes
    /// @return a string representation of tokens in format idid...id_hash (e.g. 123423678_276187 implies a sequence of
    /// nodes "123", "423", "678" where "678" is a terminal with hash 276187)
    static std::string toBranch(const std::vector<TokenizedToken> &pathContext,
                                std::unordered_map<size_t, std::string> &vocab);
};

/// Mapping between options and language callables
static std::unordered_map<std::string, std::function<const TSLanguage *(void)>> languages = {
    {"c", std::bind(tree_sitter_c)}, {"cpp", std::bind(tree_sitter_cpp)}};

/// Mapping between options and traversal callables
static std::unordered_map<std::string, std::function<std::vector<std::vector<TSNode>>(const TSNode &)>>
    traversalPolicy = {{"root_terminal", std::bind(&Traversal::root2terminal, std::placeholders::_1)},
                       {"terminal_terminal", std::bind(&Traversal::terminal2terminal, std::placeholders::_1)}};

/// Mapping between options and tokenization callables
static std::unordered_map<std::string, std::function<TokenizedToken(const TSNode &, const std::string &)>>
    tokenizationRules = {
        {"masked_identifiers",
         std::bind(&Tokenizer::defaultTokenization, std::placeholders::_1, std::placeholders::_2)},
};

/// Mapping between options and split callables
static std::unordered_map<std::string, std::function<std::string(const std::vector<TokenizedToken> &,
                                                                 std::unordered_map<size_t, std::string> &)>>
    splitStrategy = {
        {"ids_hash", std::bind(&Split::toBranch, std::placeholders::_1, std::placeholders::_2)},
};

/// Class that creates a TSTree from a given file and parses the input options to obtain the requested nodes'
/// representation
class Tree
{
    /// A callable for tree traversal
    std::function<std::vector<std::vector<TSNode>>(const TSNode &)> &traversal;
    /// A callable for nodes' tokenization
    std::function<TokenizedToken(const TSNode &, const std::string &)> &tokenizer;
    /// A callable to split sequences of nodes
    std::function<std::string(const std::vector<TokenizedToken> &, std::unordered_map<size_t, std::string> &)> &split;

    /// TreeSitter parser
    TSParser *parser;
    /// TreeSitter tree
    TSTree *tree;
    /// File's context
    std::string src;
    /// The root of a tree
    TSNode root;

  public:
    /// A vocabulary storing mapping between hashes and the corresponding terminals' names
    std::unordered_map<size_t, std::string> vocab;

    /// Constructor to build a TSTree and set the requested callables
    /// @param fileName - path to input file
    /// @param lang - @param fileName's language
    /// @param traversalParam - traversal option (the way we traverse tree and collect nodes)
    /// @param tokenizationParam - tokenization option (the way we encode nodes)
    /// @param splitParam - split option (the way we construct a path-context from sequence of nodes)
    Tree(const std::string &fileName, const std::string &lang, const std::string &traversalParam,
         const std::string &tokenizationParam, const std::string &splitParam);

    /// A function that applies the chosen callables to process an inner file in the right way
    /// @return a vector of strings representing one line in the resulting file
    std::vector<std::string> process();

    ~Tree();
};

class TreeSitter
{

    TSParser *parser;
    TSTree *tree;
    std::string src;
    int option;

  public:
    TreeSitter(const std::string &buf, const std::string &lang, int opt = 0);

    TreeSitter(const TreeSitter &other);

    // get root node
    TreeSitterNode getRoot() const;

    // get AST
    void getGraph(const char *file);

    std::string getContext() const;

    // free memory here
    ~TreeSitter();
};

// Function that gets all possible root-to-leaf paths
// >> root - start node of each r2l path
// >> reverseArr - true if we want 2 get l2r path
std::vector<std::vector<TreeSitterNode>> root2leafPaths(TreeSitterNode root, bool reverseArr = false);
}; // namespace treesitter
#endif
