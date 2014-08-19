#ifndef DUSTBIN_INCLUDE_UTILS_H
#define DUSTBIN_INCLUDE_UTILS_H
#include <string>
#include <map>
#include <jsoncpp/json/json.h>
bool check_members(const Json::Value &json,
                   const std::map<std::string, Json::ValueType> &members);
std::string replace(const std::string &str,
                    const std::string &from,
                    const std::string &to);
#endif