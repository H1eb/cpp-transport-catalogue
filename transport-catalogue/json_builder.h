#pragma once

#include "json.h"

#include <vector>

namespace json {

    enum class LastMethod {
        NONE,
        KEY,
        VALUE,
        START_DICT,
        END_DICT,
        START_ARRAY,
        END_ARRAY
    };

    class DictValueContext;
    class DictItemContext;
    class ArrayItemContext;
    
    class Builder {
    public:
        Builder() = default;
        
        DictValueContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();

    private:
        Node root_;
        std::vector<Node> nodes_stack_;
        std::vector<std::string> key_stack_;
        LastMethod last_method_ = LastMethod::NONE;

        Node MakeNode(Node::Value&& v);
    };

    class DictValueContext: public Builder{
    public:
        DictValueContext(Builder& builder)
            :builder_(builder){}
        DictValueContext Key(const std::string& key)=delete;
        DictItemContext Value(Node::Value value);
        Builder& EndDict()=delete;
        Builder& EndArray()=delete;
        Node Build()=delete;

    private:
        Builder& builder_;
    };

    class DictItemContext: public Builder{
    public:
        DictItemContext(Builder& builder)
            :builder_(builder){}
        DictItemContext Value(Node::Value value)=delete;
        DictValueContext Key(const std::string& key);
        DictItemContext StartDict()=delete;
        ArrayItemContext StartArray()=delete;
        Builder& EndDict();
        Builder& EndArray()=delete;
        Node Build()=delete;

    private:
        Builder& builder_;
    };

    class ArrayItemContext: public Builder{
    public:
        ArrayItemContext(Builder& builder)
            :builder_(builder){}
        Builder& EndDict()=delete;
        Node Build()=delete;

    private:
        Builder& builder_;
    };
}