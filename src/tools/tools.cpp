#include <string.h>
#include <dlfcn.h>
#include <map>
#include <fstream>
#include <iostream>
#include "model.h"

bool install(const Json::Value &config);

static bool check_members(const Json::Value &json,
                          const std::map<std::string, Json::ValueType> &members);

int main(int argc, char **argv) {
    const char *usage =
        "Usage:\n"
        "Install:\n"
        "    ./tools install [input-configuration=install.conf]\n"
        "Import from wordpress:\n"
        "    ./tools import-wp <file>\n";
    if (argc < 2) {
        std::cerr << usage;
        return 0;
    }
    if (strcmp(argv[1], "install") == 0) {
        std::string filename;
        if (argc == 2) {
            filename = "install.conf";
        } else if (argc == 3) {
            filename = argv[2];
        } else {
            std::cerr << usage;
            return 0;
        }
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "No such file \"" << filename << "\".\n";
            return 0;
        }
        Json::Value config;
        Json::Reader reader;
        if (!reader.parse(file, config)) {
            std::cerr << "File \"" << filename << "\" is not valid JSON.\n";
            return 0;
        }
        file.close();
        if (!install(config)) {
            return 0;
        }
    }
    return 0;
}

bool install(const Json::Value &config) {
    const char *errmsg = "Install configuration is invalid or incomplete.\n";
    if (!check_members(config, {
        {"port", Json::intValue},
        {"ip", Json::stringValue},
        {"static-prefix", Json::stringValue},
        {"db", Json::objectValue},
        {"site", Json::objectValue},
        {"theme", Json::stringValue},
        {"admin", Json::objectValue}
    })) {
        std::cerr << errmsg;
        return false;
    }
    int port = config["port"].asInt();
    const std::string &ip = config["ip"].asString();
    const Json::Value &prefix = config["static-prefix"];
    const Json::Value &db = config["db"];
    const Json::Value &site = config["site"];
    const Json::Value &admin = config["admin"];
    const std::string &theme = config["theme"].asString();
    if (!check_members(db, {{"type", Json::stringValue}}) ||
        !check_members(site, {
            {"name", Json::stringValue},
            {"description", Json::stringValue},
            {"url", Json::stringValue}
        }) ||
        !check_members(admin, {
            {"username", Json::stringValue},
            {"password", Json::stringValue}
        })) {
        std::cerr << errmsg;
        return false;
    }
    std::string model_path = "models/" + db["type"].asString() + ".so";
    void *handle = dlopen(model_path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "No such model.\n";
        return false;
    }
    typedef Model * (*GetModel)();
    GetModel get_model = (GetModel)dlsym(handle, "get_model");
    if (!get_model) {
        std::cerr << "No such symbol \"get_model\".\n";
        return false;
    }
    Model *model = get_model();
    if (!model) {
        std::cerr << "Cannot get model instance.\n";
        return false;
    }
    if (!model->check_config(db)) {
        std::cerr << "Invalid database configuration.\n";
        return false;
    }
    if (!model->initialize(db)) {
        std::cerr << "Cannot initialize model.\n";
        return false;
    }
    if (!model->add_user(admin["username"].asString(),
                         admin["password"].asString())) {
        std::cerr << "Cannot add admin user.\n";
        return false;
    }
    Article article = {
        "hello",
        "hello, world",
        "Welcome to dustbin",
        {"test"},
        time(nullptr)
    };
    if (!model->new_article(article)) {
        std::cerr << "Cannot add article.\n";
        return false;
    }
    Json::Value result, url, patterns, paths;
    result["port"] = port;
    result["ip"] = ip;
    result["site"] = site;
    result["db"] = db;
    result["theme"] = theme;
    result["enable-upload"] = true;
    result["upload-dir"] = "uploads/";
    patterns["archives"] = "/archives/?";
    patterns["archives-page"] = "/archives/page/<page>/?";
    patterns["page"] = "/page/<int:page>/?";
    patterns["article"] = "/article/<id>/?";
    patterns["tag"] = "/tag/<tag>/?";
    patterns["tag-page"] = "/tag/<tag>/page/<page>/?";
    patterns["custom-page"] = "/page/<id>/?";
    patterns["upload"] = "/upload/<.*:path>";
    patterns["feed"] = "/feed";
    paths["prefix"] = prefix,
    paths["archives"] = "/archives/";
    paths["archives-page"] = "/archives/page/<page>/";
    paths["static"] = "/static/<path>";
    paths["page"] = "/page/<page>/";
    paths["article"] = "/article/<id>/";
    paths["tag"] = "/tag/<tag>/";
    paths["tag-page"] = "/tag/<tag>/page/<page>/";
    paths["custom-page"] = "/page/<id>/";
    paths["upload"] = "/upload/<path>";
    paths["feed"] = "/feed";
    url["patterns"] = patterns;
    url["paths"] = paths;
    result["url"] = url;
    std::cout << result.toStyledString();
    dlclose(handle);
    return true;
}

static bool check_members(const Json::Value &json,
                          const std::map<std::string, Json::ValueType> &members) {
    if (!json.isObject()) {
        return false;
    }
    for (auto it = members.cbegin(); it != members.cend(); ++it) {
        const std::string &name = it->first;
        Json::ValueType type = it->second;
        if (!json.isMember(name)) {
            return false;
        }
        if (json[name].type() != type) {
            return false;
        }
    }
    return true;
}