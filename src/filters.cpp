#include <time.h>
#include <string>
#include <memory>
#include <jsoncpp/json/json.h>
#include "dustbin.h"
#include "model.h"
#include "filters.h"
#include "utils.h"

Json::Value filter_url_for_archives(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    if (page == 1) {
        return prefix + paths["archives"].asString();
    }
    const std::string &path = paths["archives-page"].asString();
    return prefix + replace(path, "<page>", std::to_string(page));
}

Json::Value filter_url_for_static(const std::string &path) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + replace(paths["static"].asString(), "<path>", path);
}

Json::Value filter_url_for_page(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    if (page == 1) {
        return prefix + "/";
    }
    return prefix + replace(paths["page"].asString(),
                            "<page>", std::to_string(page));
}

Json::Value filter_url_for_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + replace(paths["article"].asString(), "<id>", id);
}

Json::Value filter_url_for_tag(const std::string &tag, int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    if (page == 1) {
        return prefix + replace(paths["tag"].asString(), "<tag>", tag);
    }
    const std::string &path =
        replace(paths["tag-page"].asString(), "<tag>", tag);
    return prefix + replace(path, "<page>", std::to_string(page));
}

Json::Value filter_url_for_custom_page(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + replace(paths["custom-page"].asString(), "<id>", id);
}

Json::Value filter_url_for_prev_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    std::string dest;
    if (!model->get_prev_next_article_id(id, Direction::Prev, dest)) {
        return Json::Value();
    }
    return filter_url_for_article(dest);
}

Json::Value filter_url_for_next_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    std::string dest;
    if (!model->get_prev_next_article_id(id, Direction::Next, dest)) {
        return Json::Value();
    }
    return filter_url_for_article(dest);
}