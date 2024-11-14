#include <cstring>
#include <stack>
#include <vector>
#include <string>
#include <stack>
#include <algorithm>
#include <fcntl.h>
#include <cstdint>
#include <map>

namespace treesitter
{
extern "C" {
#include "tree_sitter/api.h"
}

extern "C" const TSLanguage *tree_sitter_c();
extern "C" const TSLanguage *tree_sitter_cpp();

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
    /*
        // get the parent node
        TreeSitterNode
        getParent() const
        {
            TSNode a;
            return TreeSitterNode(ts_node_parent(node), src);
        }
        */

    // check if internal node is leaf
    bool isTerminal() const;

    // check if internal node is named leaf
    bool isNamedTerminal() const;

    // check if internal node has >= 2 children
    bool isFork() const;
};

class TreeSitter
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

    TSParser *parser;
    TSTree *tree;
    std::string src;
    int option;

  public:
    TreeSitter(const std::string &buf, const std::string &lang, int opt);

    TreeSitter(const TreeSitter &other);

    // get root node
    TreeSitterNode getRoot() const;

    // get AST
    void getGraph(const char *file);

    // format token depending on its type and option
    std::string getToken(const TreeSitterNode &token);

    // format path depending on the option
    std::string getPath(const std::string &s);

    // get the representation for the extractor
    std::string getFormat(const TreeSitterNode &token1, const std::string &pathUp, const std::string &pathDown,
                          const TreeSitterNode &token2);

    // replace binary/unary ops with corresponding operation
    uint16_t maybeChangeID(const TreeSitterNode &node);

    // free memory here
    ~TreeSitter();
};

// Function that gets all possible root-to-leaf paths
// >> root - start node of each r2l path
// >> reverseArr - true if we want 2 get l2r path
std::vector<std::vector<TreeSitterNode>> root2leafPaths(TreeSitterNode root, bool reverseArr);
}; // namespace treesitter
