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