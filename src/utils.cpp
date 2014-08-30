#include <string>
#include <vector>
#include <map>
#include <jsoncpp/json/json.h>
#include "model.h"

bool check_members(const Json::Value &json,
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

std::string replace(const std::string &str,
                    const std::string &from,
                    const std::string &to) {
    std::string buf = str;
    size_t pos;
    while ((pos = buf.find(from)) != std::string::npos) {
        buf.replace(pos, from.length(), to);
    }
    return buf;
};

void article_to_json(const Article *article, Json::Value &json) {
    json["id"] = article->id;
    json["title"] = article->title;
    json["content"] = article->content;
    json["date"] = (Json::LargestUInt)article->date;
    Json::Value tags(Json::arrayValue);
    for (auto &tag: article->tags) {
        tags.append(tag);
    }
    json["tags"] = tags;
}

void custom_page_to_json(const CustomPage &page, Json::Value &json) {
    json["id"] = page.id;
    json["title"] = page.title;
    json["content"] = page.content;
    json["order"] = page.order;
}