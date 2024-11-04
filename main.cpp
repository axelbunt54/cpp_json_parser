#include "jsonParser.h"
#include "utils.h"

#include <iostream>


bool check_number_of_arguments(int argc) {
    if (argc != 3) {
        std::cerr << "Usage: {json_file_path} {json_expression}";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (!check_number_of_arguments(argc))
        return 1;

    std::string file_path = argv[1];
    std::string json_expression = argv[2];

    auto [raw_json, error] = get_raw_json(file_path);

    if (!error.empty()) {
        std::cerr << error << std::endl;
        return 1;
    }

    auto [json_ast, error1] = json::lexify_and_parse(raw_json);

    if (!error1.empty()) {
        std::cerr << error1 << std::endl;
        return 1;
    }

    std::vector<std::string> path_tokens = json::lex_expression(json_expression);
    auto [result_value, error2] = json::get_json_value_by_path(json_ast, path_tokens);

    if (!error2.empty()) {
        std::cerr << error2 << std::endl;
        return 1;
    }

    std::cout << json::stringify(result_value);

    return 0;
}
