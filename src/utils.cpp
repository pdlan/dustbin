#include <string>
#include <vector>
#include <map>
#include <jsoncpp/json/json.h>

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