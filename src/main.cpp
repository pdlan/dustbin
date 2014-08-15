#include <iostream>
#include <fstream>
#include <string>
#include <jsoncpp/json/json.h>
#include "dustbin.h"

int main(int argc, char **argv) {
    std::string config_filename;
    if (argc == 1) {
        config_filename = "dustbin.conf";
    } else if (argc == 2) {
        config_filename = argv[1];
    } else {
        std::cerr << "Usage: ./dustbin [config]\n";
        return 0;
    }
    std::ifstream config_file(config_filename);
    if (!config_file) {
        std::cerr << "No such configuration file.\n";
        return 0;
    }
    Json::Reader reader;
    Json::Value config;
    if (!reader.parse(config_file, config)) {
        std::cerr << "Cannot parse the configuration file.\n";
        return 0;
    }
    config_file.close();
    Dustbin & dustbin = Dustbin::get_instance();
    if (!dustbin.initialize(config)) {
        std::cerr << "Cannot initialize dustbin.\n";
        return 0;
    }
    if (!dustbin.start()) {
        std::cerr << "Cannot start dustbin.\n";
        return 0;
    }
    return 0;
}