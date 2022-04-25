#include "json_builder.h"

#include <utility>

namespace json {

    namespace detail {

        bool IsNodeOfArray(LastMethod method, const std::vector<Node>& stack) {
            return stack.back().IsArray() && (
                    method == LastMethod::VALUE       ||
                    method == LastMethod::START_ARRAY ||
                    method == LastMethod::END_ARRAY   ||
                    method == LastMethod::END_DICT);
        }
    }

    DictValueContext Builder::Key(std::string key) {
        using namespace std::literals;
        if(last_method_ == LastMethod::KEY){
            throw std::logic_error("Double Key()"s);
        }
        
        if (nodes_stack_.back().IsDict()) {
            key_stack_.push_back(key);
            last_method_ = LastMethod::KEY;
        } else {
            throw std::logic_error("Key() cant be applied"s);
        }

        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        using namespace std::literals;
        if (last_method_ == LastMethod::NONE) {
            nodes_stack_.emplace_back(MakeNode(std::move(value)));
        } else if (nodes_stack_.back().IsDict() && last_method_ == LastMethod::KEY) {
            Dict tmp_dict = nodes_stack_.back().AsDict();
            tmp_dict[key_stack_.back()] = MakeNode(std::move(value));
            key_stack_.pop_back();
            nodes_stack_.pop_back();
            nodes_stack_.emplace_back(tmp_dict);
        } else if (detail::IsNodeOfArray(last_method_, nodes_stack_)) {
            Array tmp_arr = nodes_stack_.back().AsArray();
            tmp_arr.push_back(MakeNode(std::move(value)));
            nodes_stack_.pop_back();
            nodes_stack_.emplace_back(tmp_arr);
        } else {
            throw std::logic_error("Error in Value()"s);
        }
        last_method_ = LastMethod::VALUE;

        return *this;
    }

    DictItemContext Builder::StartDict() {
        using namespace std::literals;
        if (last_method_ != LastMethod::NONE
            && last_method_ != LastMethod::KEY
            && !detail::IsNodeOfArray(last_method_, nodes_stack_))
        {
            throw std::logic_error("Wrong call StartDict()"s);
        }
        nodes_stack_.emplace_back(std::move(Dict{}));
        last_method_ = LastMethod::START_DICT;

        return *this;
    }

    ArrayItemContext Builder::StartArray() {
        using namespace std::literals;
        if (last_method_ != LastMethod::NONE
            && last_method_ != LastMethod::KEY
            && !detail::IsNodeOfArray(last_method_, nodes_stack_))
        {
            throw std::logic_error("Error in StartArray()"s);
        }
        nodes_stack_.emplace_back(std::move(Array{}));
        last_method_ = LastMethod::START_ARRAY;

        return *this;
    }

    Builder& Builder::EndDict() {
        using namespace std::literals;
        if (!nodes_stack_.empty() && nodes_stack_.back().IsDict()) {
            Dict tmp = nodes_stack_.back().AsDict();
            nodes_stack_.pop_back();
            if (nodes_stack_.empty()) {
                root_ = MakeNode(std::move(tmp));
            } else {
                if (nodes_stack_.back().IsDict()) {
                    Dict tmp_dict = nodes_stack_.back().AsDict();
                    tmp_dict[key_stack_.back()] = std::move(tmp);
                    key_stack_.pop_back();
                    nodes_stack_.pop_back();
                    nodes_stack_.emplace_back(std::move(tmp_dict));
                }
                else {
                    Array tmp_arr = nodes_stack_.back().AsArray();
                    tmp_arr.emplace_back(std::move(tmp));
                    nodes_stack_.pop_back();
                    nodes_stack_.emplace_back(std::move(tmp_arr));
                }
            }
        } else {
            throw std::logic_error("Error in EndDict()"s);
        }
        last_method_ = LastMethod::END_DICT;

        return *this;
    }

    Builder& Builder::EndArray() {
        using namespace std::literals;
        if (!nodes_stack_.empty() && nodes_stack_.back().IsArray()) {
            Array tmp = nodes_stack_.back().AsArray();
            nodes_stack_.pop_back();
            if (nodes_stack_.empty()) {
                root_ = MakeNode(std::move(tmp));
            } else {
                if (nodes_stack_.back().IsArray()) {
                    Array tmp_arr = nodes_stack_.back().AsArray();
                    tmp_arr.emplace_back(std::move(tmp));
                    nodes_stack_.pop_back();
                    nodes_stack_.emplace_back(std::move(tmp_arr));
                } else {
                    Dict tmp_dict = nodes_stack_.back().AsDict();
                    tmp_dict[key_stack_.back()] = std::move(tmp);
                    key_stack_.pop_back();
                    nodes_stack_.pop_back();
                    nodes_stack_.emplace_back(std::move(tmp_dict));
                }
            }
        } else {
            throw std::logic_error("Error in EndArray()"s);
        }
        last_method_ = LastMethod::END_ARRAY;

        return *this;
    }

    Node Builder::Build() {
        using namespace std::literals;
        if (!nodes_stack_.empty() && (
            last_method_ == LastMethod::END_ARRAY ||
            last_method_ == LastMethod::END_DICT  ||
            last_method_ == LastMethod::VALUE))
        {
            return nodes_stack_.back();
        }
        else if (nodes_stack_.empty() && (
                 last_method_ == LastMethod::END_ARRAY ||
                 last_method_ == LastMethod::END_DICT))
        {
            return root_;
        }
        else {
            throw std::logic_error("Error in Build()"s);
        }
    }

    Node Builder::MakeNode(Node::Value&& v) {
        json::Node result;
        std::visit(
            [&result](auto&& v) {
                result = std::move(v);
            },
            std::move(v)
        );

        return result;
    }
    
    DictItemContext DictValueContext::Value(Node::Value value){
        return builder_.Value(value);
    }

    DictValueContext DictItemContext::Key(const std::string& key){
        return builder_.Key(key);
    }
    
    Builder& DictItemContext::EndDict(){
        return builder_.EndDict();
    }
}