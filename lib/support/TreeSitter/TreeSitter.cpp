#include <support/TreeSitter/TreeSitter.h>
#include <stdint.h>

treesitter::TreeSitterNode &
treesitter::TreeSitterNode::operator=(const TreeSitterNode &other)
{
    node = other.node;
    nodeNumAsChild = other.nodeNumAsChild;
    return *this;
}

bool
treesitter::TreeSitterNode::operator==(const TreeSitterNode &b)
{
    return ts_node_eq(this->node, b.node);
}

std::string
treesitter::TreeSitterNode::getType() const
{
    return std::string(ts_node_grammar_type(node));
}

std::string
treesitter::TreeSitterNode::getValue(const std::string_view &src) const
{
    if (isTerminal()) {
        uint32_t wordBegin = getStartByte();
        uint32_t wordEnd = getEndByte();
        return std::string(src.substr(wordBegin, wordEnd - wordBegin));
    } else {
        return {};
    }
}

uint32_t
treesitter::TreeSitterNode::getStartByte() const
{
    return ts_node_start_byte(node);
}

uint32_t
treesitter::TreeSitterNode::getEndByte() const
{
    return ts_node_end_byte(node);
}

std::pair<uint32_t, uint32_t>
treesitter::TreeSitterNode::getStartPoint() const
{
    auto p = ts_node_start_point(node);
    return {p.row, p.column};
}

std::pair<uint32_t, uint32_t>
treesitter::TreeSitterNode::getEndPoint() const
{
    auto p = ts_node_end_point(node);
    return {p.row, p.column};
}

uint32_t
treesitter::TreeSitterNode::getChildCount() const
{
    return ts_node_child_count(node);
}

uint32_t
treesitter::TreeSitterNode::getNodeNum() const
{
    return nodeNumAsChild;
}

uint16_t
treesitter::TreeSitterNode::getID() const
{
    return ts_node_grammar_symbol(node);
}

treesitter::TreeSitterNode
treesitter::TreeSitterNode::getChild(uint32_t i) const
{
    return TreeSitterNode(ts_node_child(node, i), i);
}

bool
treesitter::TreeSitterNode::isTerminal() const
{
    if (!ts_node_is_null(node) && ts_node_child_count(node) == 0) {
        return true;
    } else {
        return false;
    }
}

bool
treesitter::TreeSitterNode::isNamedTerminal() const
{
    if (isTerminal() && !ts_node_is_extra(node) && ts_node_is_named(node)) {
        return true;
    } else {
        return false;
    }
}

bool
treesitter::TreeSitterNode::isFork() const
{
    if (ts_node_child_count(node) >= 2) {
        return true;
    } else {
        return false;
    }
}

treesitter::TreeSitter::TreeSitter(const std::string &buf, const std::string &lang, int opt)
{
    src = buf;
    option = opt;
    parser = ts_parser_new();
    const TSLanguage *lang_parser;
    if (lang == "c") {
        lang_parser = tree_sitter_c();
    } else if (lang == "cpp") {
        lang_parser = tree_sitter_cpp();
    };
    ts_parser_set_language(parser, lang_parser);
    tree = ts_parser_parse_string(parser, NULL, buf.c_str(), strlen(buf.c_str()));
}

treesitter::TreeSitter::TreeSitter(const TreeSitter &other)
{
    parser = other.parser;
    tree = other.tree;
    src = other.src;
}

treesitter::TreeSitterNode
treesitter::TreeSitter::getRoot() const
{
    return TreeSitterNode(ts_tree_root_node(tree));
}

void
treesitter::TreeSitter::getGraph(const char *file)
{
    int imgFd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    ts_tree_print_dot_graph(tree, imgFd);
}

std::string
treesitter::TreeSitter::getToken(const TreeSitterNode &token)
{
    std::string ans;
    int p = 1;
    std::string t = token.getType();
    if (tokenOptions.find(t) != tokenOptions.end()) {
        p = tokenOptions[t][option];
    }
    switch (p) {
    case 0:
        ans = token.getValue(src);
        break;
    case 1:
        ans = t;
        break;
    case 2:
        ans = std::to_string(std::hash<std::string>{}(token.getValue(src)));
    default:
        break;
    }
    return ans;
}

std::string
treesitter::TreeSitter::getPath(const std::string &s)
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

std::string
treesitter::TreeSitter::getFormat(const TreeSitterNode &token1, const std::string &pathUp, const std::string &pathDown,
                                  const TreeSitterNode &token2)
{
    std::string t1, p, t2;
    t1 = getToken(token1);
    t2 = getToken(token2);
    p = pathUp + pathDown;
    p = getPath(p);
    return t1 + "," + p + "," + t2;
}

uint16_t
treesitter::TreeSitter::maybeChangeID(const TreeSitterNode &node)
{
    uint16_t ans = node.getID();
    std::string t = node.getType();
    if (expressions.find(t) != expressions.end()) {
        ans = node.getChild(expressions[t]).getID();
    }
    return ans;
}

treesitter::TreeSitter::~TreeSitter()
{
    ts_parser_delete(parser);
    ts_tree_delete(tree);
}

std::vector<std::vector<treesitter::TreeSitterNode>>
treesitter::root2leafPaths(TreeSitterNode root, bool reverseArr = false)
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
        if (root.isFork() || root.isTerminal()) {
            path.push_back(root);
        }
        // found leaf
        if (root.isTerminal()) {
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
