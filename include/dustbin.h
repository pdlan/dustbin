#ifndef DUSTBIN_INCLUDE_DUSTBIN_H
#define DUSTBIN_INCLUDE_DUSTBIN_H
#include <memory>
#include <string>
#include <map>
#include <jsoncpp/json/json.h>
#include <recycled/jinja2.h>
#include <recycled.h>

class Model;
class Theme;
class Dustbin {
    public:
        static Dustbin & get_instance();
        bool initialize(const Json::Value &config);
        bool start();
        recycled::jinja2::Template get_template(const std::string &name);
        std::shared_ptr<Model> & get_model();
        std::shared_ptr<Theme> & get_theme();
    private:
        std::shared_ptr<recycled::Application<recycled::HTTPServer>> application;
        std::shared_ptr<Model> model;
        std::shared_ptr<Theme> theme;
        std::map<std::string, std::string> paths;
        Json::Value config;
        Dustbin();
        Dustbin(const Dustbin &other) = delete;
        ~Dustbin();
        Dustbin & operator=(const Dustbin &other) = delete;
        bool set_globals();
        static Json::Value filter_url_for(const Json::Value &arg,
                                          const std::string &type);
};
#endif