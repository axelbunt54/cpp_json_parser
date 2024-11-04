#include <fstream>
#include <string>


// Open and read the JSON file, get a raw JSON string
std::tuple<std::string, std::string> get_raw_json(std::string file_path) {
    std::string raw_json;
    std::string tmp_line;
    std::ifstream json_file(file_path);

    if (json_file.is_open()) {
        while (std::getline(json_file, tmp_line)) {
            raw_json += tmp_line;
        }
        json_file.close();
    } else {
        return {"", "Unable to open file \"" + file_path + "\""};
    }

    return {raw_json, ""};
}
