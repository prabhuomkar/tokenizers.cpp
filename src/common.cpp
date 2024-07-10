// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/common.h"

#include <string>
#include <utility>
#include <vector>

#include "simdjson.h"

Token::Token() {}

Token::Token(int id, std::string value, std::pair<int, int> offsets)
    : id(id), value(value), offsets(offsets) {}
