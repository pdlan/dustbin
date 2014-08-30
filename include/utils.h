#ifndef DUSTBIN_INCLUDE_UTILS_H
#define DUSTBIN_INCLUDE_UTILS_H
#include <string>
#include <map>
#include <jsoncpp/json/json.h>
struct Article;
struct CustomPage;
bool check_members(const Json::Value &json,
                   const std::map<std::string, Json::ValueType> &members);
std::string replace(const std::string &str,
                    const std::string &from,
                    const std::string &to);
void article_to_json(const Article *article, Json::Value &json);
void custom_page_to_json(const CustomPage &page, Json::Value &json);
#endif