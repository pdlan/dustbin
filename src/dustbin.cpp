#include <stdint.h>
#include <iostream>
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
/*
struct UrlForPage {
    UrlForPage() {
        Dustbin &dustbin = Dustbin::get_instance();
        this->path = dustbin.paths["page"];
    }
    std::string path;
    const std::vector<Json::ValueType> arguments = {Json::intValue};
};
*/

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
        {"archives-page", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue},
        {"tag", Json::stringValue},
        {"tag-page", Json::stringValue}
    }) || ! check_members(paths, {
        {"archives", Json::stringValue},
        {"archives-page", Json::stringValue},
        {"static", Json::stringValue},
        {"page", Json::stringValue},
        {"article", Json::stringValue},
        {"tag", Json::stringValue},
        {"tag-page", Json::stringValue}
    })) {
        return false;
    }
    this->paths = {
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
        return false;
    }
    if (!env->add_filter("url_for_archives", filter_url_for_archives) ||
        !env->add_filter("url_for_static", filter_url_for_static) ||
        !env->add_filter("url_for_page", filter_url_for_page) ||
        !env->add_filter("url_for_article", filter_url_for_article) ||
        !env->add_filter("url_for_tag", filter_url_for_tag)) {
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