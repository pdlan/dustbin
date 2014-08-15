#include <time.h>
#include <fstream>
#include <string>
#include <memory>
#include <recycled.h>
#include <recycled/jinja2.h>
#include <jsoncpp/json/json.h>
#include "theme.h"
#include "utils.h"

using namespace recycled::jinja2;

static Json::Value filter_format_time(time_t t, const std::string &format) {
    const int BufferLength = 256;
    char buffer[BufferLength];
    tm *timeinfo = localtime(&t);
    strftime(buffer, BufferLength, format.c_str(), timeinfo);
    return buffer;
}

bool Theme::set_theme(const std::string &theme) {
    std::ifstream conf_file("theme/" + theme + "/theme.conf");
    if (!conf_file) {
        return false;
    }
    Json::Reader reader;
    Json::Value config;
    if (!reader.parse(conf_file, config)) {
        conf_file.close();
        return false;
    }
    conf_file.close();
    if (!check_members(config, {
        {"name", Json::stringValue},
        {"author", Json::stringValue},
        {"articles-per-page", Json::objectValue}
    })) {
        return false;
    }
    const std::string &main_templates_path = "theme/" + theme + "/templates/";
    try {
        PrefixLoader loader({
            {"main",
             std::make_shared<FileSystemLoader>(main_templates_path)
            },
            {"admin",
             std::make_shared<FileSystemLoader>("admin/templates/")
            }
        });
        this->env.reset(new Environment(loader));
    } catch (const std::exception &e) {
        return false;
    }
    this->env->add_filter("format_time", filter_format_time);
    Json::Value articles_per_page = config["articles-per-page"];
    if (!check_members(articles_per_page, {
        {"normal", Json::intValue},
        {"tags", Json::intValue},
        {"archives", Json::intValue}
    })) {
        return false;
    }
    this->articles_per_page = {
        {PageType::Normal, articles_per_page["normal"].asInt()},
        {PageType::Tags, articles_per_page["tags"].asInt()},
        {PageType::Archives, articles_per_page["archives"].asInt()}
    };
    return true;
}

std::shared_ptr<recycled::jinja2::Environment> & Theme::get_environment() {
    return this->env;
}

int Theme::get_articles_per_page(PageType type) {
    auto it = this->articles_per_page.find(type);
    if (it == this->articles_per_page.end()) {
        return -1;
    } else {
        return it->second;
    }
}