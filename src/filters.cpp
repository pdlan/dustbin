#include <string>
#include <jsoncpp/json/json.h>
#include "dustbin.h"
#include "filters.h"
#include "utils.h"


Json::Value filter_url_for_archives(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &prefix = dustbin.paths["prefix"];
    if (page == 1) {
        return prefix + dustbin.paths["archives"];
    }
    const std::string &path = dustbin.paths["archives-page"];
    return prefix + replace(path, "<page>", std::to_string(page));
}

Json::Value filter_url_for_static(const std::string &path) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &prefix = dustbin.paths["prefix"];
    return prefix + replace(dustbin.paths["static"], "<path>", path);
}

Json::Value filter_url_for_page(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &prefix = dustbin.paths["prefix"];
    if (page == 1) {
        return prefix + "/";
    }
    return prefix + replace(dustbin.paths["page"],
                            "<page>", std::to_string(page));
}

Json::Value filter_url_for_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &prefix = dustbin.paths["prefix"];
    return prefix + replace(dustbin.paths["article"], "<id>", id);
}

Json::Value filter_url_for_tag(const std::string &tag, int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &prefix = dustbin.paths["prefix"];
    if (page == 1) {
        return prefix + replace(dustbin.paths["tag"], "<tag>", tag);
    }
    const std::string &path = replace(dustbin.paths["tag-page"], "<tag>", tag);
    return prefix + replace(path, "<page>", std::to_string(page));
}

Json::Value filter_url_for_custom_page(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &prefix = dustbin.paths["prefix"];
    return prefix + replace(dustbin.paths["custom-page"], "<id>", id);
}