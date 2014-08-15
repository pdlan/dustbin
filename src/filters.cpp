#include <string>
#include <jsoncpp/json/json.h>
#include "dustbin.h"
#include "filters.h"

static std::string replace(const std::string &str,
                           const std::string &from,
                           const std::string &to) {
    std::string buf = str;
    size_t pos;
    while ((pos = buf.find(from)) != std::string::npos) {
        buf.replace(pos, from.length(), to);
    }
    return buf;
};

Json::Value filter_url_for_archives(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    if (page == 1) {
        return dustbin.paths["archives"];
    }
    const std::string &path = dustbin.paths["archives-page"];
    return replace(path, "<page>", std::to_string(page));
}

Json::Value filter_url_for_static(const std::string &path) {
    Dustbin &dustbin = Dustbin::get_instance();
    return replace(dustbin.paths["static"], "<path>", path);
}

Json::Value filter_url_for_page(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    if (page == 1) {
        return "/";
    }
    return replace(dustbin.paths["page"], "<page>", std::to_string(page));
}

Json::Value filter_url_for_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    return replace(dustbin.paths["article"], "<id>", id);
}

Json::Value filter_url_for_tag(const std::string &tag, int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    if (page == 1) {
        return replace(dustbin.paths["tag"], "<tag>", tag);
    }
    const std::string &path = replace(dustbin.paths["tag-page"], "<tag>", tag);
    return replace(path, "<page>", std::to_string(page));
}