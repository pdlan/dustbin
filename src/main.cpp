#include <iostream>
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
    Dustbin & dustbin = Dustbin::get_instance();
    dustbin.set_cmd_arg(argc, argv);
    if (!dustbin.initialize(config_filename)) {
        return 0;
    }
    if (!dustbin.start()) {
        std::cerr << "Cannot start dustbin.\n";
        return 0;
    }
    return 0;
}