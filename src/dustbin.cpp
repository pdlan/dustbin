#include <stdint.h>
#include <dlfcn.h>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <jsoncpp/json/json.h>
#include <recycled.h>
#include <recycled/jinja2.h>
#include "dustbin.h"
#include "handlers.h"
#include "model.h"
#include "theme.h"
#include "utils.h"
#include "global_functions.h"

using namespace recycled;
using namespace recycled::jinja2;

Dustbin & Dustbin::get_instance() {
    static Dustbin dustbin;
    return dustbin;
}

bool Dustbin::initialize(const std::string &config_filename) {
    this->config_filename = config_filename;
    std::ifstream config_file(config_filename);
    if (!config_file.is_open()) {
        std::cerr << "No such configuration file.\n";
        return false;
    }
    Json::Reader reader;
    Json::Value config;
    if (!reader.parse(config_file, config)) {
        std::cerr << "Cannot parse the configuration file.\n";
        return false;
    }
    config_file.close();
    this->config = config;
    if (!this->check_config(this->config)) {
        std::cerr << "Invalid configuration file.\n";
        return false;
    }
    if (!this->init_theme()) {
        return false;
    }
    if (!this->init_model()) {
        return false;
    }
    if (!this->set_globals()) {
        return false;
    }
    if (!this->init_server()) {
       
       return false;
    }
    return true;
}

void Dustbin::set_cmd_arg(int argc, char **argv) {
    this->argv = new char *[argc + 1];
    for (int i = 0; i < argc; ++i) {
        this->argv[i] = argv[i];
    }
    this->argv[argc] = 0;
}

bool Dustbin::start() {
    IOLoop &ioloop = IOLoop::get_instance();
    return ioloop.start();
}

recycled::jinja2::Template Dustbin::get_template(const std::string &name) {
    return this->theme->get_environment()->get_template(name);
}

const std::shared_ptr<Model> & Dustbin::get_model() {
    return this->model;
}

const std::shared_ptr<Theme> & Dustbin::get_theme() {
    return this->theme;
}

const std::string & Dustbin::get_config_filename() {
    return this->config_filename;
}

void Dustbin::restart() {
    execv(this->argv[0], this->argv);
}

const Json::Value & Dustbin::get_config() {
    return this->config;
}

Dustbin::Dustbin(): theme(new Theme), argv(nullptr), model_handle(nullptr) {
}

Dustbin::~Dustbin() {
    if (this->model_handle) {
        dlclose(this->model_handle);
    }
}

bool Dustbin::check_config(const Json::Value &config) {
    if (!check_members(config, {
        {"ip", Json::stringValue},
        {"port", Json::intValue},
        {"theme", Json::stringValue},
        {"db", Json::objectValue},
        {"url", Json::objectValue},
        {"enable-upload", Json::booleanValue}
    })) {
        return false;
    }
    bool enable_upload = config["enable-upload"].asBool();
    if (enable_upload) {
        if (!config.isMember("upload-dir") ||
            config["upload-dir"].type() != Json::stringValue) {
            return false;
        }
    }
    const Json::Value &url = config["url"];
    if (!check_members(url, {
        {"patterns", Json::objectValue},
        {"paths", Json::objectValue}
    })) {
        return false;
    }
    const Json::Value &patterns = url["patterns"];
    const Json::Value &paths = url["paths"];
    if (!check_members(patterns, {
        {"archives", Json::stringValue},
        {"archives-page", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue},
        {"tag", Json::stringValue},
        {"tag-page", Json::stringValue},
        {"custom-page", Json::stringValue},
        {"feed", Json::stringValue}
    }) || ! check_members(paths, {
        {"prefix", Json::stringValue},
        {"archives", Json::stringValue},
        {"archives-page", Json::stringValue},
        {"static", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue},
        {"tag", Json::stringValue},
        {"tag-page", Json::stringValue},
        {"custom-page", Json::stringValue},
        {"feed", Json::stringValue}
    })) {
        return false;
    }
    if (enable_upload) {
        if (!patterns.isMember("upload") ||
            patterns["upload"].type() != Json::stringValue ||
            !paths.isMember("upload") ||
            paths["upload"].type() != Json::stringValue) {
            return false;
        }
    }
    const Json::Value &db = this->config["db"];
    if (!check_members(db, {
        {"type", Json::stringValue}
    })) {
        return false;
    }
    return true;
}

bool Dustbin::init_model() {
    const Json::Value &db = this->config["db"];
    const std::string &db_type = db["type"].asString();
    std::string model_path = "models/" + db_type + ".so";
    void *handle = dlopen(model_path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot load model.\n"
                  << "----Detail: " << dlerror() << std::endl;
        return false;
    }
    this->model_handle = handle;
    typedef Model * (*GetModel)();
    GetModel get_model = (GetModel)dlsym(handle, "get_model");
    if (!get_model) {
        std::cerr << "No get_model function in this model.\n";
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
    this->model.reset(model);
    return true;
}

bool Dustbin::init_theme() {
    const std::string &theme = this->config["theme"].asString();
    if (!this->theme->set_theme(theme)) {
        std::cerr << "Cannot set theme \"" << theme << "\".\n";
        return false;
    }
    std::shared_ptr<Environment> env;
    if (!this->theme || !(env = this->theme->get_environment())) {
        std::cerr << "Bad template environment.\n";
        return false;
    }
    if (!env->add_global_func("url_for_archives", global_url_for_archives) ||
        !env->add_global_func("url_for_static", global_url_for_static) ||
        !env->add_global_func("url_for_page", global_url_for_page) ||
        !env->add_global_func("url_for_article", global_url_for_article) ||
        !env->add_global_func("url_for_tag", global_url_for_tag) ||
        !env->add_global_func("url_for_custom_page",
                              global_url_for_custom_page) ||
        !env->add_global_func("url_for_prev_article",
                              global_url_for_prev_article) ||
        !env->add_global_func("url_for_next_article",
                              global_url_for_next_article) ||
        !env->add_global_func("url_for_feed", global_url_for_feed) ||
        !env->add_global_func("get_articles", global_get_articles) ||
        !env->add_global_func("get_article", global_get_article) ||
        !env->add_global_func("get_pages", global_get_pages) ||
        !env->add_global_func("get_page", global_get_page) ||
        !env->add_global_func("py_import", global_py_import)) {
        std::cerr << "Cannot add global functions\n";
        return false;
    }
    return true;
}

bool Dustbin::init_server() {
    uint16_t port = (uint16_t)this->config["port"].asUInt();
    const std::string &ip = this->config["ip"].asString();
    const std::string &theme = this->config["theme"].asString();
    const Json::Value &url = this->config["url"];
    const Json::Value &patterns = url["patterns"];
    try {
        std::vector<HandlerStruct> handlers = {
            {
                "/static/<.*:path>",
                StaticFileHandler("theme/" + theme + "/static/"),
                {HTTPMethod::GET}
            },
            {
                "/",
                page_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["page"].asString(),
                page_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["custom-page"].asString(),
                custom_page_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["article"].asString(),
                article_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["archives-page"].asString(),
                archives_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["archives"].asString(),
                archives_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["tag-page"].asString(),
                tag_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["tag"].asString(),
                tag_handler,
                {HTTPMethod::GET}
            },
            {
                patterns["feed"].asString(),
                feed_handler,
                {HTTPMethod::GET}
            },
            {
                "/admin/static/<.*:path>",
                StaticFileHandler("admin/static/"),
                {HTTPMethod::GET}
            },
            {
                "/admin/?",
                admin_index_handler,
                {HTTPMethod::GET}
            },
            {
                "/admin/upload/?",
                admin_upload_handler,
                {HTTPMethod::POST}
            },
            {
                "/admin/articles/?",
                admin_articles_handler,
                {HTTPMethod::GET}
            },
            {
                "/admin/articles/page/<page>/?",
                admin_articles_handler,
                {HTTPMethod::GET}
            },
            {
                "/admin/articles/new/?",
                admin_new_article_handler,
                {HTTPMethod::GET, HTTPMethod::POST}
            },
            {
                "/admin/articles/edit/<id>/?",
                admin_edit_article_handler,
                {HTTPMethod::GET, HTTPMethod::POST}
            },
            {
                "/admin/articles/delete/<id>/?",
                admin_delete_article_handler,
                {HTTPMethod::POST}
            },
            {
                "/admin/config/?",
                admin_config_handler,
                {HTTPMethod::GET, HTTPMethod::POST}
            },
            {
                "/admin/restart/?",
                admin_restart_handler,
                {HTTPMethod::GET}
            },
            {
                "/admin/pages/?",
                admin_pages_handler,
                {HTTPMethod::GET}
            },
            {
                "/admin/pages/new/?",
                admin_new_page_handler,
                {HTTPMethod::GET, HTTPMethod::POST}
            },
            {
                "/admin/pages/edit/<id>/?",
                admin_edit_page_handler,
                {HTTPMethod::GET, HTTPMethod::POST}
            },
            {
                "/admin/pages/delete/<id>/?",
                admin_delete_page_handler,
                {HTTPMethod::POST}
            },
            {
                "/admin/pages/reorder/?",
                admin_reorder_pages_handler,
                {HTTPMethod::POST}
            }
        };
        if (this->config["enable-upload"].asBool()) {
            handlers.push_back({
                patterns["upload"].asString(),
                StaticFileHandler(this->config["upload-dir"].asString()),
                {HTTPMethod::GET}
            });
        }
        this->application.reset(new Application<HTTPServer>(handlers));
        auto error_handler =
            std::bind(&Dustbin::error_handler, this,
                      std::placeholders::_1, std::placeholders::_2);
        this->application->set_error_handler(error_handler);
        this->application->listen(port, ip);
    } catch (const std::exception &e) {
        std::cerr << "Cannot listen.\n";
        return false;
    }
    return true;
}

bool Dustbin::set_globals() {
    if (!this->theme) {
        return false;
    }
    std::shared_ptr<Environment> &env = this->theme->get_environment();
    if (!env) {
        return false;
    }
    if (!this->config.isObject() || !this->config.isMember("site")) {
        return false;
    }
    const Json::Value &site = this->config["site"];
    if (!check_members(site, {
        {"name", Json::stringValue},
        {"description", Json::stringValue},
        {"url", Json::stringValue}
    })) {
        return false;
    }
    env->add_global("site", site);
    return true;
}

void Dustbin::error_handler(int code, recycled::Connection &conn) {
    if (code != 404) {
        default_error_handler(code, conn);
        return;
    }
    const Template &temp = this->get_template("main/404.html");
    if (!temp.render({}, conn)) {
        default_error_handler(code, conn);
        return;
    }
    conn.add_header("Content-Type", "text/html");
    conn.set_status(code);
}