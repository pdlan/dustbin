#include <stdint.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
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

using namespace recycled;
using namespace recycled::jinja2;

Dustbin & Dustbin::get_instance() {
    static Dustbin dustbin;
    return dustbin;
}

bool Dustbin::initialize(const Json::Value &config) {
    if (!check_members(config, {
        {"port", Json::intValue},
        {"theme", Json::stringValue},
        {"db", Json::objectValue},
        {"url", Json::objectValue}
    })) {
        return false;
    }
    uint16_t port = (uint16_t)config["port"].asUInt();
    const std::string &theme = config["theme"].asString();
    const Json::Value &db = config["db"];
    const Json::Value &url = config["url"];
    this->config = config;
    if (!this->theme->set_theme(theme)) {
        return false;
    }
    if (!this->set_globals()) {
        return false;
    }
    if (!check_members(url, {
        {"patterns", Json::objectValue},
        {"paths", Json::objectValue}
    })) {
        return false;
    }
    if (!check_members(db, {
        {"type", Json::stringValue}
    })) {
        return false;
    }
    const Json::Value &patterns = url["patterns"];
    const Json::Value &paths = url["paths"];
    if (!check_members(patterns, {
        {"archives", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue}
    }) || ! check_members(paths, {
        {"archives", Json::stringValue},
        {"static", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue}
    })) {
        return false;
    }
    this->paths = {
        {"archives", paths["archives"].asString()},
        {"static", paths["static"].asString()},
        {"page", paths["page"].asString()},
        {"article", paths["article"].asString()}
    };
    std::shared_ptr<Environment> env;
    if (!this->theme || !(env = this->theme->get_environment())) {
        return false;
    }
    if (!env->add_filter("url_for", filter_url_for)) {
        return false;
    }
    const std::string &db_type = db["type"].asString();
    if (db_type == "mongodb") {
        if (!check_members(db, {
            {"connection-string", Json::stringValue},
            {"name", Json::stringValue}
        })) {
            return false;
        }
        const std::string &db_connstr = db["connection-string"].asString();
        const std::string &db_name = db["name"].asString();
        std::shared_ptr<MongoModel> model(new MongoModel);
        if (!model->initialize(db_connstr, db_name)) {
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
                "/static/admin/<.*:path>",
                StaticFileHandler("admin/static/"),
                {HTTPMethod::GET}
            },
            {
                "/static/<.*:path>",
                StaticFileHandler("theme/" + theme + "/static/"),
                {HTTPMethod::GET}
            }
        }));
        this->application->listen(port);
    } catch (const std::exception &e) {
        return false;
    }
    return true;
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

Dustbin::Dustbin(): theme(new Theme) {
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

Json::Value Dustbin::filter_url_for(const Json::Value &arg,
                                    const std::string &type) {
    Dustbin &dustbin = Dustbin::get_instance();
    typedef std::map<std::string, std::tuple<std::string, Json::ValueType>> M;
    static const M arguments = {
        {"static", std::forward_as_tuple("path", Json::stringValue)},
        {"page", std::forward_as_tuple("page", Json::intValue)},
        {"article", std::forward_as_tuple("id", Json::stringValue)}
    };
    if (type == "archives") {
        return dustbin.paths["archives"];
    }
    auto replace = [](const std::string &str,
                      const std::string &from,
                      const std::string &to) {
        size_t pos = str.find(from);
        std::string buf = str;
        if (pos == std::string::npos) {
            return buf;
        }
        buf.replace(pos, from.length(), to);
        return buf;
    };
    auto it = arguments.find(type);
    if (it == arguments.end()) {
        return "";
    }
    const std::tuple<std::string, Json::ValueType> &arg_info = it->second;
    Json::ValueType arg_type = std::get<1>(arg_info);
    if (arg_type != arg.type()) {
        return "";
    }
    const std::string &arg_name = std::get<0>(arg_info);
    std::string arg_value;
    switch (arg_type) {
        case Json::intValue: arg_value = std::to_string(arg.asInt()); break;
        case Json::stringValue: arg_value = arg.asString(); break;
    }
    return replace(dustbin.paths[type], "<"+arg_name+">", arg_value);
}