#include <support/TreeSitter/TreeSitter.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <optional>

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
    std::ifstream file(buf);
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    src = ss.str();
    option = opt;
    parser = ts_parser_new();
    const TSLanguage *lang_parser;
    if (lang == "c") {
        lang_parser = tree_sitter_c();
    } else if (lang == "cpp") {
        lang_parser = tree_sitter_cpp();
    };
    ts_parser_set_language(parser, lang_parser);
    tree = ts_parser_parse_string(parser, NULL, src.c_str(), strlen(src.c_str()));
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

treesitter::TreeSitter::~TreeSitter()
{
    ts_parser_delete(parser);
    ts_tree_delete(tree);
}

std::string
treesitter::TreeSitter::getContext() const
{
    return src;
}

std::vector<std::vector<treesitter::TreeSitterNode>>
treesitter::root2leafPaths(TreeSitterNode root, bool reverseArr)
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

std::vector<std::vector<treesitter::TSNode>>
treesitter::Traversal::getAllNode2TerminalPaths(const TSNode &node, bool reverseArr)
{
    // vector of all possible r2l paths
    std::vector<std::vector<TSNode>> res;

    // a root2terminal path
    std::vector<TSNode> stack;

    TSTreeCursor cursor = ts_tree_cursor_new(node);
    TSNode curNode = ts_tree_cursor_current_node(&cursor);
    stack.push_back(curNode);

    int isNew = true;

    while (1) {
        if (isNew && ts_tree_cursor_goto_first_child(&cursor)) {
            // to child (down)
            curNode = ts_tree_cursor_current_node(&cursor);
            stack.push_back(curNode);
        } else {
            // terminal || !isNew
            if (isNew) {
                // terminal -> add a new root-terminal path
                auto vec = stack;
                if (reverseArr) {
                    std::reverse(vec.begin(), vec.end());
                }
                res.push_back(vec);
            }
            if (ts_tree_cursor_goto_next_sibling(&cursor)) {
                // to sibling terminal (up-down)
                stack.pop_back();
                isNew = true;
                curNode = ts_tree_cursor_current_node(&cursor);
                stack.push_back(curNode);
            } else if (ts_tree_cursor_goto_parent(&cursor)) {
                // to parent (up)
                isNew = false;
                stack.pop_back();
            } else {
                stack.clear();
                break;
            }
        }
    }

    ts_tree_cursor_delete(&cursor);
    return res;
}

std::vector<std::vector<treesitter::TSNode>>
treesitter::Traversal::root2terminal(const TSNode &root)
{
    auto vec = getAllNode2TerminalPaths(root);
    return vec;
}

std::vector<std::vector<treesitter::TSNode>>
treesitter::Traversal::terminal2terminal(const TSNode &root)
{
    auto vec = getAllNode2TerminalPaths(root);
    /// @todo
    return vec;
}

std::optional<std::vector<treesitter::TokenizedToken>>
treesitter::Tokenizer::defaultTokenization(const std::vector<TSNode> &nodes, const std::string &src,
                                           std::unordered_map<size_t, std::string> &vocab)
{
    // remove comments
    if (ts_node_is_extra(nodes.back())) {
        return std::nullopt;
    }
    // remove too short branches (e.g. #define ..., #include ...)
    if (nodes.size() < 5) {
        return std::nullopt;
    }
    std::vector<TokenizedToken> res;
    for (auto &node : nodes) {
        std::string id = std::format("{:0>3}", ts_node_grammar_symbol(node));
        size_t name;
        if (!ts_node_is_null(node) && ts_node_child_count(node) == 0) {
            // terminal
            std::string tempName;
            if (ts_node_is_named(node) && std::string_view(ts_node_grammar_type(node)) != "identifier") {
                // named terminal => exists in the grammar
                size_t bytes = ts_node_end_byte(node) - ts_node_start_byte(node);
                tempName = src.substr(ts_node_start_byte(node), bytes);
            } else {
                // identifiers + unnamed
                tempName = ts_node_grammar_type(node);
            }
            // add a terminal to vocabulary
            name = std::hash<std::string>{}(tempName);
            vocab[name] = "[" + tempName + "]";
        } else {
            // non-terminal
            name = 0;
        }
        res.push_back(TokenizedToken(id, name));
    }

    return res;
}

std::string
treesitter::Split::toBranch(const std::vector<TokenizedToken> &pathContext)
{
    std::string res;
    for (const auto &token : pathContext) {
        res += token.id;
    }
    auto hashed = pathContext.back().name;
    res += "_" + std::to_string(hashed);

    return res;
}

treesitter::Tree::Tree(const std::string &fileName, const std::string &lang, const std::string &traversalParam,
                       const std::string &tokenizationParam, const std::string &splitParam)
    : traversal(traversalPolicy[traversalParam]), tokenizer(tokenizationRules[tokenizationParam]),
      split(splitStrategy[splitParam])
{
    std::ifstream file(fileName);
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    src = ss.str();
    parser = ts_parser_new();

    // ts_parser_set_language(parser, languages[lang]());
    const TSLanguage *lang_parser = tree_sitter_cpp();
    ts_parser_set_language(parser, lang_parser);
    tree = ts_parser_parse_string(parser, NULL, src.c_str(), strlen(src.c_str()));
    root = ts_tree_root_node(tree);
}

std::vector<std::string>
treesitter::Tree::process()
{
    // traverse the tree
    auto pathVectors = traversal(root);
    std::vector<std::vector<TokenizedToken>> tokens;
    for (const auto &path : pathVectors) {
        // get a tokenized path-context
        auto temp = tokenizer(path, src, vocab);
        if (temp.has_value()) {
            tokens.push_back(temp.value());
        }
    }
    std::vector<std::string> res;
    for (auto &token : tokens) {
        // get path-context's final representation
        res.push_back(split(token));
    }
    return res;
}

treesitter::Tree::~Tree()
{
    ts_parser_delete(parser);
    ts_tree_delete(tree);
}
