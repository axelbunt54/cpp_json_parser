#include "jsonParser.h"

#include <iostream>

namespace json {
    std::string JSONTokenType_to_string(JSONTokenType jsonTokenType) {
        switch (jsonTokenType) {
            case JSONTokenType::String:
                return "String";
            case JSONTokenType::Number:
                return "Number";
            case JSONTokenType::Boolean:
                return "Boolean";
            case JSONTokenType::Syntax:
                return "Syntax";
            case JSONTokenType::Null:
                return "Null";
        }
        return "UnknownType";
    }

    // Error formating
    std::string format_error(std::string base, std::string source, int index) {
        std::ostringstream s;
        int counter = 0;
        int line = 1;
        int column = 0;
        std::string lastline = "";
        std::string indent = "";
        for (auto c : source) {
            if (counter == index) {
                break;
            }

            if (c == '\n') {
                line++;
                column = 0;
                lastline = "";
                indent = "";
            } else if (c == '\t') {
                column++;
                lastline += "  ";
                indent += "  ";
            } else {
                column++;
                lastline += c;
                indent += " ";
            }

            counter++;
        }

        // Continue accumulating the lastline for debugging
        while (counter < source.size()) {
            auto c = source[counter];
            if (c == '\n') {
                break;
            }
            lastline += c;
            counter++;
        }

        s << base << " at line " << line << ", column " << column << std::endl;
        s << lastline << std::endl;
        s << indent << "^";

        return s.str();
    }

    std::string format_parse_error(std::string base, JSONToken token) {
        std::ostringstream s;
        s << "Unexpected token '" << token.value << "', type '"
            << JSONTokenType_to_string(token.type) << "', index ";
        s << std::endl << base;
        return format_error(s.str(), *token.original_json, token.offset);
    }

    // Generic lexers
    int lex_whitespace(std::string raw_json, int index) {
        while (std::isspace(raw_json[index])) {
            if (index == raw_json.length()) {
                break;
            }

            index++;
        }

        return index;
    }

    std::tuple<JSONToken, int, std::string> lex_syntax(std::string raw_json,
                                                       int index) {
        JSONToken token{JSONTokenType::Syntax, "", index};
        std::string value = "";
        auto c = raw_json[index];
        if (c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == ',') {
            token.value += c;
            index++;
        }

        return {token, index, ""};
    }

    std::tuple<JSONToken, int, std::string> lex_string(std::string raw_json,
                                                       int original_index) {
        int index = original_index;
        JSONToken token{JSONTokenType::String, "", index};
        std::string value = "";
        auto c = raw_json[index];
        if (c != '"') {
            return {token, original_index, ""};
        }
        index++;

        // TODO: handle nested quotes
        while (c = raw_json[index], c != '"') {
            if (index == raw_json.length()) {
            return {
                token,
                index,
                format_error("Unexpected EOF while lexing string",
                             raw_json, index)};
            }

            token.value += c;
            index++;
        }
        index++;

        return {token, index, ""};
    }

    std::tuple<JSONToken, int, std::string> lex_number(std::string raw_json,
                                                       int original_index) {
        int index = original_index;
        JSONToken token = {JSONTokenType::Number, "", index};
        std::string value = "";
        while (true) {
            if (index == raw_json.length()) {
                break;
            }

            auto c = raw_json[index];
            if (!(c >= '0' && c <= '9') && !(c == '.') && !(c == '-')) {
                break;
            }

            token.value += c;
            index++;
        }

        return {token, index, ""};
    }

    std::tuple<JSONToken, int, std::string> lex_keyword(std::string raw_json,
                                                        std::string keyword,
                                                        JSONTokenType type,
                                                        int original_index) {
        int index = original_index;
        JSONToken token{type, "", index};
        while (keyword[index - original_index] == raw_json[index]) {
            if (index == raw_json.length()) {
                break;
            }

            index++;
        }

        if (index - original_index == keyword.length()) {
            token.value = keyword;
        }
        return {token, index, ""};
    }

    std::tuple<JSONToken, int, std::string> lex_null(std::string raw_json,
                                                    int index) {
        return lex_keyword(raw_json, "null", JSONTokenType::Null, index);
    }

    std::tuple<JSONToken, int, std::string> lex_true(std::string raw_json,
                                                    int index) {
        return lex_keyword(raw_json, "true", JSONTokenType::Boolean, index);
    }

    std::tuple<JSONToken, int, std::string> lex_false(std::string raw_json,
                                                      int index) {
        return lex_keyword(raw_json, "false", JSONTokenType::Boolean, index);
    }

    std::tuple<std::vector<JSONToken>, std::string> lexify(std::string raw_json) {
        std::vector<JSONToken> tokens;
        auto original_copy = std::make_shared<std::string>(raw_json);
        auto generic_lexers = {lex_syntax, lex_string, lex_number,
                                lex_null,   lex_true,   lex_false};
        for (int i = 0; i < raw_json.length(); i++) {
            if (auto new_index = lex_whitespace(raw_json, i); i != new_index) {
                i = new_index - 1;
                continue;
            }

            auto found = false;
            for (auto lexer : generic_lexers) {
                if (auto [token, new_index, error] = lexer(raw_json, i); i != new_index) {
                    if (error.length()) {
                        return std::make_tuple(std::vector<JSONToken>{}, error);
                    }

                    token.original_json = original_copy;
                    tokens.push_back(token);
                    i = new_index - 1;
                    found = true;
                    break;
                }
            }

            if (found) {
                continue;
            }

            return {
                std::vector<JSONToken>{},
                format_error("Unable to lex", raw_json, i)
            };
        }

        return {tokens, ""};
    }

    std::tuple<std::vector<JSONValue>, int, std::string>
    parse_array(std::vector<JSONToken> tokens, int index) {
        std::vector<JSONValue> children = {};
        while (index < tokens.size()) {
            auto t = tokens[index];
            if (t.type == JSONTokenType::Syntax) {
                if (t.value == "]") {
                    return {children, index + 1, ""};
                }

                if (t.value == ",") {
                    index++;
                    t = tokens[index];
                } else if (children.size() > 0) {
                    return {{},
                            index,
                            format_parse_error("Expected comma after element in array", t)};
                }
            }

            auto [child, new_index, error] = parse(tokens, index);
            if (error.size()) {
                return {{}, index, error};
            }

            children.push_back(child);
            index = new_index;
        }

        return {
            {},
            index,
            format_parse_error("Unexpected EOF while parsing array", tokens[index])
        };
    }

    std::tuple<std::unordered_map<std::string, JSONValue>, int, std::string>
    parse_object(std::vector<JSONToken> tokens, int index) {
        std::unordered_map<std::string, JSONValue> values = {};
        while (index < tokens.size()) {
            auto t = tokens[index];
            if (t.type == JSONTokenType::Syntax) {
                if (t.value == "}") {
                    return {values, index + 1, ""};
                }

                if (t.value == ",") {
                    index++;
                    t = tokens[index];
                } else if (values.size() > 0) {
                    return {
                        {},
                        index,
                        format_parse_error("Expected comma after element "
                                           "in object", t)};
                } else {
                    return {{},
                            index,
                            format_parse_error(
                                "Expected key-value pair or closing "
                                "brace in object", t)};
                }
            }

            auto [key, new_index, error] = parse(tokens, index);
            if (error.size()) {
                return {{}, index, error};
            }

            if (key.type != JSONValueType::String) {
                return {
                    {}, index, format_parse_error("Expected string key in object", t)
                };
            }
            index = new_index;
            t = tokens[index];

            if (!(t.type == JSONTokenType::Syntax && t.value == ":")) {
                return {{},
                    index,
                    format_parse_error("Expected colon after key in object", t)
                };
            }
            index++;
            t = tokens[index];

            auto [value, new_index1, error1] = parse(tokens, index);
            if (error1.size()) {
                return {{}, index, error1};
            }

            values[key.string.value()] = value;
            index = new_index1;
        }

        return {values, index + 1, ""};
    }

    std::tuple<JSONValue, int, std::string>
    parse(std::vector<JSONToken> tokens, int index) {
        json::JSONToken token = tokens[index];
        switch (token.type) {
            case JSONTokenType::Number: {
                JSONValue v_tmp;
                v_tmp.type = JSONValueType::Number;

                // Test optimization (we could parse always to double, but let's try to use the smallest type)
                // TODO: use long, float, short, byte, ...
                if (token.value.find('.') != std::string::npos) {
                    double n = std::stod(token.value);
                    v_tmp.float_num = n;
                } else {
                    int n = std::stoi(token.value);
                    v_tmp.integer_num = n;
                }

                return {
                    v_tmp,
                    index + 1,
                    ""
                };
            }
            case JSONTokenType::Boolean: {
                return {
                    JSONValue {
                        .type = JSONValueType::Boolean,
                        .boolean = token.value == "true"
                    },
                    index + 1,
                    ""
                };
            }
            case JSONTokenType::Null: {
                return {
                    JSONValue {
                        .type = JSONValueType::Null
                    },
                    index + 1,
                    ""
                };
            }
            case JSONTokenType::String: {
                return {
                    JSONValue {
                        .type = JSONValueType::String,
                        .string = token.value
                    },
                    index + 1,
                    ""
                };
            }
            case JSONTokenType::Syntax: {
                if (token.value == "[") {
                    auto [array, new_index, error] = parse_array(tokens,
                                                                 index + 1);
                    return {
                        JSONValue {
                            .type = JSONValueType::Array,
                            .array = array
                        },
                        new_index,
                        error
                    };
                }

                if (token.value == "{") {
                    auto [object, new_index, error] = parse_object(tokens,
                                                                   index + 1);
                    return {
                        JSONValue {
                            .type = JSONValueType::Object,
                            .object = std::optional(object)
                        },
                        new_index,
                        error
                    };
                }
            }
        }

        return std::make_tuple(
            json::JSONValue{},
            index,
            format_parse_error("Failed to parse", token)
        );
    }

    std::tuple<JSONValue, std::string> lexify_and_parse(std::string raw_json) {
        auto [tokens, error] = json::lexify(raw_json);
        if (error.size()) {
            return std::make_tuple(json::JSONValue{}, error);
        }

        auto [ast, _, error1] = json::parse(tokens);
        return {ast, error1};
    }

    std::string stringify(JSONValue v, std::string indent) {
        switch (v.type) {
            case JSONValueType::String:
                return "\"" + v.string.value() + "\"";
            case JSONValueType::Boolean:
                return (v.boolean.value() ? "true" : "false");
            case JSONValueType::Number:
                if (v.integer_num.has_value()) {
                    return std::to_string(v.integer_num.value());
                } else {
                    return std::to_string(v.float_num.value());
                }
            case JSONValueType::Null:
                return "null";
            case JSONValueType::Array: {
                std::string s = "[\n";
                auto a = v.array.value();
                for (int i = 0; i < a.size(); i++) {
                    auto value = a[i];
                    s += indent + "  " + stringify(value, indent + "  ");
                    if (i < a.size() - 1) {
                        s += ",";
                    }

                    s += "\n";
                }

                return s + indent + "]";
            }
            case JSONValueType::Object: {
                std::string s = "{\n";
                auto values = v.object.value();
                auto i = 0;
                for (auto const &[key, value] : values) {
                    s += indent + "  " + "\"" + key +
                    "\": " + stringify(value, indent + "  ");

                    if (i < values.size() - 1) {
                        s += ",";
                    }

                    s += "\n";
                    i++;
                }

                return s + indent + "}";
            }
        }
        return "Unknown Value Type";
    }

    // Expression parsing
    std::vector<std::string> lex_expression(const std::string& json_expression) {
        // TODO: add expression check
        std::vector<std::string> tokens;
        std::regex re(R"((\w+)|\[(\d+|\w+)\])");  // Regex for keys and values
        std::smatch match;

        std::string::const_iterator searchStart(json_expression.cbegin());
        while (std::regex_search(searchStart, json_expression.cend(), match, re)) {
            tokens.push_back(match[0]);
            searchStart = match.suffix().first;
        }
        return tokens;
    }

    std::tuple<json::JSONValue, std::string> get_json_value_by_path(const json::JSONValue v, const std::vector<std::string>& path_tokens, int index) {
        if (index >= path_tokens.size()) {
            return {v, ""};
        }

        std::string token = path_tokens[index];
        index++;

        switch (v.type) {
            case json::JSONValueType::Array: {
                if (token.find('[') != std::string::npos) {
                    std::string tmp_token;
                    for (char c : token) {
                        if (c == '[' || c == ']' || c == ' ') {
                            continue;
                        }
                        tmp_token += c;
                    }
                    token = tmp_token;
                }

                try {
                    int array_index = std::stoi(token);
                    const auto& arr = v.array.value();

                    if (array_index < 0 || array_index >= static_cast<int>(arr.size())) {
                        throw std::out_of_range("Array index out of bounds");
                    }

                    return get_json_value_by_path(arr[array_index], path_tokens, index);
                } catch (const std::invalid_argument&) {
                    return std::make_tuple(
                        JSONValue{},
                        "Invalid index: '" + token + "' is not a number"
                    );
                } catch (const std::out_of_range&) {
                    return std::make_tuple(
                        JSONValue{},
                        "Array index out of bounds: " + token
                    );
                }
            }
            case json::JSONValueType::Object: {
                auto pos = v.object.value().find(token);
                if (pos == v.object.value().end()) {
                    return std::make_tuple(json::JSONValue{}, "Key error: " + token);
                } else {
                    json::JSONValue new_value = pos->second;
                    return get_json_value_by_path(new_value, path_tokens, index);
                }
            }
            default:
                return get_json_value_by_path(v, path_tokens, index);
        }
    }
} // namespace json
