#include <stdint.h>
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
#include "filters.h"

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
        return 0;
    }
    Json::Reader reader;
    Json::Value config;
    if (!reader.parse(config_file, config)) {
        std::cerr << "Cannot parse the configuration file.\n";
        return 0;
    }
    config_file.close();
    if (!check_members(config, {
        {"ip", Json::stringValue},
        {"port", Json::intValue},
        {"theme", Json::stringValue},
        {"db", Json::objectValue},
        {"url", Json::objectValue}
    })) {
        std::cerr << "Invalid configuration file.\n";
        return false;
    }
    uint16_t port = (uint16_t)config["port"].asUInt();
    const std::string &ip = config["ip"].asString();
    const std::string &theme = config["theme"].asString();
    const Json::Value &db = config["db"];
    const Json::Value &url = config["url"];
    this->config = config;
    if (!this->theme->set_theme(theme)) {
        std::cerr << "Cannot set theme \"" << theme << "\".\n";
        return false;
    }
    if (!this->set_globals()) {
        return false;
    }
    if (!check_members(url, {
        {"patterns", Json::objectValue},
        {"paths", Json::objectValue}
    })) {
        std::cerr << "Invalid configuration file.\n";
        return false;
    }
    if (!check_members(db, {
        {"type", Json::stringValue}
    })) {
        std::cerr << "Invalid configuration file.\n";
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
        {"tag-page", Json::stringValue}
    }) || ! check_members(paths, {
        {"prefix", Json::stringValue},
        {"archives", Json::stringValue},
        {"archives-page", Json::stringValue},
        {"static", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue},
        {"tag", Json::stringValue},
        {"tag-page", Json::stringValue}
    })) {
        std::cerr << "Invalid configuration file.\n";
        return false;
    }
    this->paths = {
        {"prefix", paths["prefix"].asString()},
        {"archives", paths["archives"].asString()},
        {"archives-page", paths["archives-page"].asString()},
        {"static", paths["static"].asString()},
        {"page", paths["page"].asString()},
        {"article", paths["article"].asString()},
        {"tag", paths["tag"].asString()},
        {"tag-page", paths["tag-page"].asString()}
    };
    std::shared_ptr<Environment> env;
    if (!this->theme || !(env = this->theme->get_environment())) {
        std::cerr << "Bad template environment.\n";
        return false;
    }
    if (!env->add_filter("url_for_archives", filter_url_for_archives) ||
        !env->add_filter("url_for_static", filter_url_for_static) ||
        !env->add_filter("url_for_page", filter_url_for_page) ||
        !env->add_filter("url_for_article", filter_url_for_article) ||
        !env->add_filter("url_for_tag", filter_url_for_tag)) {
        std::cerr << "Cannot add filters\n";
        return false;
    }
    const std::string &db_type = db["type"].asString();
    if (db_type == "mongodb") {
        if (!check_members(db, {
            {"host", Json::stringValue},
            {"name", Json::stringValue}
        })) {
            return false;
        }
        const std::string &db_connstr = db["host"].asString();
        const std::string &db_name = db["name"].asString();
        Json::Value db_auth(Json::objectValue);
        if (db.isMember("auth")) {
            if (db["auth"].type() == Json::objectValue) {
                db_auth = db["auth"];
            } else if (db["auth"].type() == Json::nullValue) {
            } else {
                std::cerr << "Invalid configuration file.\n";
                return false;
            }
        }
        std::shared_ptr<MongoModel> model(new MongoModel);
        if (!model->initialize(db_connstr, db_name, db_auth)) {
            std::cerr << "Cannnot connect to database.\n";
            return false;
        }
        this->model = model;
    } else {
        std::cerr << "Unsupported database.\n";
        return false;
    }
    try {
        this->application.reset(new Application<HTTPServer>({
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
            }
        }));
        this->application->listen(port, ip);
    } catch (const std::exception &e) {
        std::cerr << "Cannot listen.\n";
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

std::shared_ptr<Model> & Dustbin::get_model() {
    return this->model;
}

std::shared_ptr<Theme> & Dustbin::get_theme() {
    return this->theme;
}

const std::string & Dustbin::get_config_filename() {
    return this->config_filename;
}

void Dustbin::restart() {
    execv(this->argv[0], this->argv);
}

Dustbin::Dustbin(): theme(new Theme), argv(nullptr) {
}

Dustbin::~Dustbin() {
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