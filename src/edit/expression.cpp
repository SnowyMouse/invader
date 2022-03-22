#include "expression.hpp"

#undef NDEBUG

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <climits>
#include <cassert>
#include <optional>
#include <cmath>

template <typename Number, Number number_from_value(const std::string &what)> static Number evaluate_expression_of_type(const char *expression, Number input) {
    struct ParsedToken {
        enum Type {
            GROUP,
            NUMBER,
            INPUT,
            POWER,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE
        } type;

        bool is_operator() const noexcept {
            switch(this->type) {
                case Type::ADD:
                case Type::SUBTRACT:
                case Type::MULTIPLY:
                case Type::DIVIDE:
                case Type::POWER:
                    return true;
                default:
                    return false;
            }
        }

        int operator_priority() noexcept {
            assert(this->is_operator());

            switch(this->type) {
                case Type::ADD:
                case Type::SUBTRACT:
                    return 1;
                case Type::MULTIPLY:
                case Type::DIVIDE:
                    return 2;
                case Type::POWER:
                    return 3;
                default:
                    std::terminate();
            }
        }

        std::vector<ParsedToken> group;
        Number number = 1.0; // multiplier if group/input. number if number
    };
    
    // Get tokens
    std::vector<std::string> tokens;
    const char *current_offset = nullptr;
    auto append_token = [&current_offset, &tokens](const char *end) {
        if(current_offset == nullptr) {
            return;
        }
        tokens.emplace_back(current_offset, end);
        current_offset = nullptr;
    };
    const char *cursor;
    for(cursor = expression; *cursor; cursor++) {
        if(*cursor == '(' || *cursor == ')' || *cursor == '+' || *cursor == '-' || *cursor == '/' || *cursor == '*' || *cursor == '^' || *cursor == 'n') {
            append_token(cursor);
            tokens.emplace_back(cursor, cursor + 1);
            continue;
        }
        if(*cursor == ' ' || *cursor == '\n' || *cursor == '\t' || *cursor == '\r') {
            append_token(cursor);
            continue;
        }
        if(current_offset == nullptr) {
            current_offset = cursor;
        }
    }
    append_token(cursor);

    // Do we have tokens?
    if(tokens.empty()) {
        //std::puts("No tokens??");
        throw std::exception();
    }

    // Make sure parenthesis count is correct
    long parenthesis = 0;
    for(auto &t : tokens) {
        if(t == "(") {
            parenthesis++;
        }
        if(t == ")") {
            if(parenthesis <= 0) {
                //std::puts("Bad right parenthesis");
                throw std::exception();
            }
            parenthesis--;
        }
    }
    if(parenthesis != 0) {
        //std::puts("Left parenthesis count != Right parenthesis count");
        throw std::exception();
    }

    std::size_t token_index = 0;

    #define CURRENT_TOKEN (tokens[token_index])

    auto recursively_parse_tokens = [&tokens, &token_index](auto &recursively_parse_tokens) -> ParsedToken {
        ParsedToken new_token;

        auto &main_token = CURRENT_TOKEN;

        if(main_token == "+") {
            new_token.type = ParsedToken::Type::ADD;
        }
        else if(main_token == "-") {
            new_token.type = ParsedToken::Type::SUBTRACT;
        }
        else if(main_token == "*") {
            new_token.type = ParsedToken::Type::MULTIPLY;
        }
        else if(main_token == "/") {
            new_token.type = ParsedToken::Type::DIVIDE;
        }
        else if(main_token == "^") {
            new_token.type = ParsedToken::Type::POWER;
        }
        else if(main_token == "n") {
            new_token.type = ParsedToken::Type::INPUT;
        }
        else if(main_token == "(") {
            new_token.type = ParsedToken::Type::GROUP;
            token_index++;
            while(CURRENT_TOKEN != ")") {
                new_token.group.emplace_back(recursively_parse_tokens(recursively_parse_tokens));
            }
        }
        else if(main_token == ")") {
            throw;
        }
        else {
            new_token.type = ParsedToken::Type::NUMBER;
            new_token.number = number_from_value(main_token);
        }
        token_index++;
        return new_token;
    };
    

    ParsedToken main_group;
    main_group.type = ParsedToken::Type::GROUP;
    while(token_index < tokens.size()) {
        main_group.group.emplace_back(recursively_parse_tokens(recursively_parse_tokens));
    }

    auto recursively_sort_group = [](ParsedToken &group, auto &recursively_sort_group) -> void {
        assert(group.type == ParsedToken::Type::GROUP);

        // Check if empty group!
        if(group.group.empty()) {
            //std::puts("Empty group!");
            throw std::exception();
        }

        // Next, clean up all of its groups
        for(auto &g : group.group) {
            if(g.type == ParsedToken::Type::GROUP) {
                recursively_sort_group(g, recursively_sort_group);
            }
        }

        // Next, go through everything in the group
        std::size_t n = 0;

        // Turn 2n -> 2 * n and -n -> -1 * n
        while(n < group.group.size()) {
            auto &t = group.group[n];
            auto last = n + 1 == group.group.size();
            auto first = n == 0;

            if(!last) {
                auto &next = group.group[n+1];

                if(!next.is_operator()) {
                    // 2n -> 2 * n
                    if(t.type == ParsedToken::Type::NUMBER) {
                        ParsedToken middle;
                        middle.type = ParsedToken::Type::MULTIPLY;
                        group.group.insert(group.group.begin() + n + 1, middle);
                        continue;
                    }

                    // -n -> -1 * n
                    if(t.type == ParsedToken::Type::SUBTRACT) {
                        bool multiply_by_negative_1 = true;
                        if(!first) {
                            // Special handling - exponents get implicit parenthesis
                            if((multiply_by_negative_1 = group.group[n-1].is_operator()) && group.group[n-1].type == ParsedToken::Type::POWER) {
                                next.number *= -1.0;
                                group.group.erase(group.group.begin() + n);
                                continue;
                            }
                        }
                        if(multiply_by_negative_1) {
                            t.type = ParsedToken::Type::MULTIPLY;

                            ParsedToken negative_one;
                            negative_one.number = -1.0;
                            negative_one.type = ParsedToken::Type::NUMBER;
                            group.group.insert(group.group.begin() + n, negative_one);

                            continue;
                        }
                    }
                }
            }

            n++;
        }

        // We should make sure it's a valid expression (operators may not follow operators, non-operators may not follow non-operators, may not end or start with operator)
        if(group.group[0].is_operator() || group.group[group.group.size()-1].is_operator()) {
            //std::puts("May not start or end with operator");
            throw std::exception();
        }
        for(std::size_t i = 0; i < group.group.size() - 1; i++) {
            if(group.group[i].is_operator() == group.group[i + 1].is_operator()) {
                //std::puts("Operators must follow non-operators and vice-versa");
                throw std::exception();
            }
        }

        // Make sure the group size still makes sense
        assert((group.group.size() - 1) % 2 == 0);

        // Sort order-of-operations
        while(group.group.size() > 1) {
            std::size_t group_end = group.group.size() - 1;
            for(std::size_t i = group_end - 1; i > 1; i -= 2) {
                auto this_priority = group.group[i-0].operator_priority();
                auto prev_priority = group.group[i-2].operator_priority();

                if(prev_priority < this_priority) {
                    std::size_t i_start = i - 1;
                    std::size_t i_end = i;

                    for(; i_end < group.group.size(); i_end += 2) {
                        if(group.group[i_end].operator_priority() != this_priority) {
                            break;
                        }
                    }

                    auto iter_start = group.group.begin() + i_start;
                    auto iter_end = group.group.begin() + i_end;

                    ParsedToken new_group;
                    new_group.type = ParsedToken::Type::GROUP;
                    new_group.group = { iter_start, iter_end };
                    group.group.erase(iter_start, iter_end);
                    group.group.insert(group.group.begin() + i_start, new_group);
                    continue;
                }
            }
            break;
        }
        

        // Next, if it's just one thing in a group, we can move the thing up a level
        while(group.group.size() == 1) {
            group = std::move(group.group[0]);

            // If we're no longer a group, we're done
            if(group.type != ParsedToken::Type::GROUP) {
                return;
            }
        }
    };
    
    recursively_sort_group(main_group, recursively_sort_group);

    // Now we can evaluate it!
    auto recursively_evaluate_token = [](const ParsedToken &token, Number input, auto &recursively_evaluate_token) -> Number {
        switch(token.type) {
            case ParsedToken::Type::INPUT:
                return input * token.number;
            case ParsedToken::Type::NUMBER:
                return token.number;
            case ParsedToken::Type::GROUP: {
                Number value = 0.0;
                auto op = ParsedToken::Type::ADD;
                auto length = token.group.size();
                for(std::size_t i = 0; i < length; i+=2) {
                    auto last = i + 1 == length;
                    auto next_value = recursively_evaluate_token(token.group[i], input, recursively_evaluate_token);
                    switch(op) {
                        case ParsedToken::Type::ADD:
                            value += next_value;
                            break;
                        case ParsedToken::Type::SUBTRACT:
                            value -= next_value;
                            break;
                        case ParsedToken::Type::MULTIPLY:
                            value *= next_value;
                            break;
                        case ParsedToken::Type::DIVIDE:
                            if(next_value == 0) {
                                std::fputs("Division by zero!\n", stderr);
                                throw std::exception();
                            }
                            value /= next_value;
                            break;
                        case ParsedToken::Type::POWER:
                            value = std::pow(value, next_value);
                            break;
                        default:
                            assert(false);
                    }
                    if(!last) {
                        op = token.group[i+1].type;
                    }
                }
                return value;
            }
            default:
                assert(false);
                std::terminate();
        }
    };

    return recursively_evaluate_token(main_group, input, recursively_evaluate_token);
}

static double string_to_double(const std::string &what) {
    return std::stod(what);
}

static std::int64_t string_to_int(const std::string &what) {
    return std::stoll(what);
}

namespace Invader::Edit {
    double evaluate_expression(const char *expression, double input) {
        return evaluate_expression_of_type<double, string_to_double>(expression, input);
    }
    std::int64_t evaluate_expression(const char *expression, std::int64_t input) {
        return evaluate_expression_of_type<std::int64_t, string_to_int>(expression, input);
    }
}
