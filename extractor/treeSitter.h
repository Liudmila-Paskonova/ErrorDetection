#include <cstring>
#include <stack>
#include <vector>
#include <string>
#include <stack>
#include <algorithm>
#include <fcntl.h>

extern "C" {
#include "tree_sitter/api.h"
}

extern "C" const TSLanguage *tree_sitter_c();

class TreeSitterNode
{
  private:
    TSNode node;
    uint32_t nodeNumAsChild;
    std::string_view src;

  public:
    TreeSitterNode(const TSNode &a, const std::string_view &str, uint32_t num = 0)
        : node(a), nodeNumAsChild(num), src(str){};

    TreeSitterNode &
    operator=(const TreeSitterNode &other)
    {
        node = other.node;
        nodeNumAsChild = other.nodeNumAsChild;
        src = other.src;
        return *this;
    }

    friend bool
    operator==(const TreeSitterNode &a, const TreeSitterNode &b)
    {
        return ts_node_eq(a.node, b.node);
    }

    // get the type of the node
    std::string
    getType() const
    {
        return std::string(ts_node_grammar_type(node));
    }

    // get the value of the particular node
    std::string
    getValue() const
    {
        if (isTerminal()) {
            uint32_t wordBegin = getStartByte();
            uint32_t wordEnd = getEndByte();
            return std::string(src.substr(wordBegin, wordEnd - wordBegin));
        } else {
            return {};
        }
    }

    // get the start byte of the node
    uint32_t
    getStartByte() const
    {
        return ts_node_start_byte(node);
    }

    // get the end byte of the node
    uint32_t
    getEndByte() const
    {
        return ts_node_end_byte(node);
    }

    // get number of child nodes
    uint32_t
    getChildCount() const
    {
        return ts_node_child_count(node);
    }

    // get the child number this node's parent has for this node
    uint32_t
    getNodeNum() const
    {
        return nodeNumAsChild;
    }

    // get the node ID
    uint16_t
    getID() const
    {
        return ts_node_grammar_symbol(node);
    }

    // get the particular child node
    TreeSitterNode
    getChild(uint32_t i) const
    {
        return TreeSitterNode(ts_node_child(node, i), src, i);
    }

    // check if internal node is leaf
    bool
    isTerminal() const
    {
        if (!ts_node_is_null(node) && ts_node_child_count(node) == 0) {
            return true;
        } else {
            return false;
        }
    }

    // check if internal node is named leaf
    bool
    isNamedTerminal() const
    {
        if (isTerminal() && !ts_node_is_extra(node) && ts_node_is_named(node)) {
            return true;
        } else {
            return false;
        }
    }

    // check if internal node has >= 2 children
    bool
    isFork() const
    {
        if (ts_node_child_count(node) >= 2) {
            return true;
        } else {
            return false;
        }
    }
};

class TreeSitter
{
  private:
    TSParser *parser;
    TSTree *tree;
    std::string src;

  public:
    TreeSitter(const std::string &buf) : src(buf)
    {
        parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_c());
        tree = ts_parser_parse_string(parser, NULL, buf.c_str(), strlen(buf.c_str()));
    }

    TreeSitter(const TreeSitter &other)
    {
        parser = other.parser;
        tree = other.tree;
        src = other.src;
    }

    // get root node
    TreeSitterNode
    getRoot() const
    {
        return TreeSitterNode(ts_tree_root_node(tree), src);
    }

    // get AST
    void
    getGraph(const char *file)
    {
        int imgFd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        ts_tree_print_dot_graph(tree, imgFd);
    }

    // free memory here
    ~TreeSitter()
    {
        ts_parser_delete(parser);
        ts_tree_delete(tree);
    }
};

// Function that gets all possible root-to-leaf paths
// >> root - start node of each r2l path
// >> reverseArr - true if we want 2 get l2r path
static std::vector<std::vector<TreeSitterNode>>
root2leafPaths(TreeSitterNode root, bool reverseArr = false)
{
    // vector of all possible l2l paths
    std::vector<std::vector<TreeSitterNode>> res;
    // one particular path
    std::vector<TreeSitterNode> path;
    // here we store nodes we want to process with their child indices = numbers that parent nodes have 4 them
    std::stack<std::pair<TreeSitterNode, std::vector<TreeSitterNode>>> s;

    s.push({root, path});

    while (!s.empty()) {
        auto it = s.top();
        s.pop();

        root = it.first;
        path = it.second;

        // don't add nodes with 1 child
        if (root.isFork() || root.isNamedTerminal()) {
            path.push_back(root);
        }
        // found leaf
        if (root.isNamedTerminal()) {
            if (reverseArr) {
                std::reverse(path.begin(), path.end());
            }
            res.push_back(path);
        }
        // if not, look up all children
        for (int i = root.getChildCount() - 1; i >= 0; --i) {
            s.push({root.getChild(i), path});
        }
    }
    return res;
}
