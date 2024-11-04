#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <sstream>
#include <regex>


namespace json {
    // Types and struct for lexing (tokenization)
    enum class JSONTokenType { String, Number, Boolean, Syntax, Null };

    struct JSONToken {
        JSONTokenType type;
        std::string value;
        int offset;
        std::shared_ptr<std::string> original_json;
    };

    // Types and struct for parsing
    enum class JSONValueType { String, Number, Boolean, Object, Array, Null };

    struct JSONValue {
        JSONValueType type;
        // TODO: rewrite to union
        std::optional<std::string> string;
        std::optional<int> integer_num;
        std::optional<double> float_num;
        std::optional<bool> boolean;
        std::optional<std::unordered_map<std::string, JSONValue>> object;
        std::optional<std::vector<JSONValue>> array;
    };

    // JSON
    std::tuple<std::vector<JSONToken>, std::string>
    lexify(std::string raw_json);  // TODO: add link

    std::tuple<JSONValue, int, std::string>
    parse(std::vector<JSONToken> tokens, int index = 0);

    std::tuple<JSONValue, std::string> lexify_and_parse(std::string raw_json);
    std::string stringify(JSONValue value, std::string indent = "");

    // Expression
    std::vector<std::string> lex_expression(const std::string& expression);

    std::tuple<json::JSONValue, std::string>
    get_json_value_by_path(const json::JSONValue v,  // TODO: add link
                           const std::vector<std::string>& path_tokens,
                           int index = 0);
}  // Namespace json
