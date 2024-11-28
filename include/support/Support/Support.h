#ifndef SUPPORT_SUPPORT_SUPPORT_H
#define SUPPORT_SUPPORT_SUPPORT_H

#include <vector>
#include <filesystem>
#include <random>
#include <any>
#include <set>

namespace support
{
std::vector<std::filesystem::path> getNRandomFiles(const std::filesystem::path &dir, size_t n);

/// Type base
template <size_t idx, typename... Ts> struct Type {
};

template <size_t idx, typename T, typename... Ts>
    requires(idx == 0)
struct Type<idx, T, Ts...> {
    using type = T;
};

template <size_t idx, typename T, typename... Ts>
    requires(idx > 0)
struct Type<idx, T, Ts...> {
    using type = Type<idx - 1, Ts...>::type;
};

/// Class storing a particular cmdline parameter as Any type
class Data
{
  public:
    std::string longArg;
    std::string shortArg;
    std::any value;

    Data(const std::string &lArg, const std::string &shArg, const std::any &defVal)
        : longArg(lArg), shortArg(shArg), value(defVal)
    {
    }
    bool operator==(const std::string &str) const;
};

/// Class used to store arguments in the arguments'list
template <typename T>
    requires(std::is_arithmetic<T>() == true || std::same_as<T, std::string>)
class Argument
{
  public:
    std::string longArg;
    std::string shortArg;
    T defaultValue;
    // possible values if provided
    std::set<T> container;

    Argument(const char *lArg, const char *shArg, const T &defVal, const std::set<T> &values = {})
        : longArg(lArg), shortArg(shArg), defaultValue(defVal), container(values)
    {
        if (!longArg.starts_with("--")) {
            throw std::format("Long arguments should start with -- : {}", longArg);
        }
        if (!shortArg.starts_with("-")) {
            throw std::format("Short arguments should start with - : {}", shortArg);
        }
        if (longArg.empty() || shortArg.empty()) {
            throw std::format("{} argument is empty", longArg.empty() ? "Long" : "Short");
        }
    }

    std::optional<std::any>
    find(const std::string &name, const std::string &param = {}) const
    {
        // if there's no such key, pack is empty
        if (longArg != name && shortArg != name) {
            return {};
        }
        // if no parameter provided and container is empty, value is boolean
        if (param.empty() && container.empty() && std::same_as<T, bool>) {
            return true;
        }
        // if no parameter provided but container is not empty, then return err
        if (param.empty() && !container.empty()) {
            return {};
        }

        std::istringstream istream(param);
        T newValue;
        istream >> newValue;

        // if there're no restrictions on parameter's value, return this value
        if (container.empty()) {
            return newValue;
        }

        for (const auto &elem : container) {
            if (elem == newValue) {
                return newValue;
            }
        }

        // if param value is impossible
        return {};
    }
};

template <typename C> struct is_instantiation_of_argument : std::false_type {
};

template <typename T> struct is_instantiation_of_argument<Argument<T>> : std::true_type {
};

template <typename T>
concept IsArgument = is_instantiation_of_argument<T>::value;

/// Class that stores one particular value in a tuple
template <size_t idx, typename D>
    requires IsArgument<D>
class TupleNode
{
    D data;

  public:
    TupleNode(D const &data_) : data(data_) {}
    TupleNode(D &&data_) : data(std::move(data_)) {}

    // get a reference to the stored data
    constexpr D &
    get()
    {
        return data;
    }
};

/// Base for the recursion
template <size_t idx, typename... Ts> struct TupleEntity {
    // the end of the tuple is reached => return empty value
    std::optional<std::any>
    find(const std::string &name, const std::string &param = {})
    {
        return {};
    }

    // all nodes' data has already been collected
    void
    elem(std::vector<Data> &res)
    {
        return;
    }
};

/// Partial specialization should inherit from the corresponding TupleNode && next TupleEntity
template <size_t idx, typename T, typename... Ts>
class TupleEntity<idx, T, Ts...> : public TupleNode<idx, typename std::remove_reference<T>::type>,
                                   public TupleEntity<idx + 1, Ts...>
{
    using NodeType = typename std::remove_reference<T>::type;

  public:
    template <typename X, typename... Xs>
    TupleEntity(X &&arg, Xs &&...args)
        : TupleNode<idx, NodeType>(std::forward<X>(arg)), TupleEntity<idx + 1, Ts...>(std::forward<Xs>(args)...)
    {
    }
    // Check if the current node contains the value being searched
    std::optional<std::any>
    find(const std::string &name, const std::string &param = {})
    {
        auto newVal = static_cast<TupleNode<idx, NodeType> &>(*this).get().find(name, param);
        if (newVal) {
            return newVal;
        } else {
            return static_cast<TupleEntity<idx + 1, Ts...> &>(*this).find(name, param);
        }
    }

    // collect a Data object
    constexpr void
    elem(std::vector<Data> &res)
    {
        auto node = static_cast<TupleNode<idx, typename std::remove_reference<T>::type> &>(*this).get();

        res.push_back({node.longArg, node.shortArg, node.defaultValue});

        static_cast<TupleEntity<idx + 1, Ts...> &>(*this).elem(res);
    }
};

/// Main tuple object
template <typename... Ts> class ArgTuple : public TupleEntity<0, Ts...>
{
  public:
    template <typename... Xs> ArgTuple(Xs &&...args) : TupleEntity<0, Ts...>(std::forward<Xs>(args)...) {}

    static constexpr size_t
    size()
    {
        return sizeof...(Ts);
    }

    // Get the value for the given index
    template <size_t idx>
    auto &
    getVal()
    {
        return static_cast<TupleNode<idx, typename Type<idx, Ts...>::type> &>(*this).get();
    }

    // Given an argument's name and, probably, a parameter, return a value
    std::optional<std::any>
    find(const std::string &name, const std::string &param = {})
    {
        return static_cast<TupleEntity<0, Ts...> &>(*this).find(name, param);
    }

    // Collect all the tuple into a vector
    std::vector<Data>
    getTuple()
    {
        std::vector<Data> res;
        // Print the tuple and go to next element
        static_cast<TupleEntity<0, Ts...> &>(*this).elem(res);
        return res;
    }
};

/// explicit template deduction (CTAD)
template <typename... Xs> ArgTuple(Xs... args) -> ArgTuple<Xs...>;

template <typename C> struct is_instantiation_of_tuple : std::false_type {
};

template <typename... Ts> struct is_instantiation_of_tuple<ArgTuple<Ts...>> : std::true_type {
};

template <typename... Ts>
concept IsTuple = is_instantiation_of_tuple<Ts...>::value;
}; // namespace support
#endif
