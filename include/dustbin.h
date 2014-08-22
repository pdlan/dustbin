#ifndef DUSTBIN_INCLUDE_DUSTBIN_H
#define DUSTBIN_INCLUDE_DUSTBIN_H
#include <memory>
#include <set>
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
        bool initialize(const std::string &config_filename);
        void set_cmd_arg(int argc, char **argv);
        bool start();
        recycled::jinja2::Template get_template(const std::string &name);
        const std::shared_ptr<Model> & get_model();
        const std::shared_ptr<Theme> & get_theme();
        const std::string & get_config_filename();
        void restart();
        bool check_config(const Json::Value &config);
        const Json::Value & get_config();
    private:
        std::shared_ptr<recycled::Application<recycled::HTTPServer>> application;
        std::shared_ptr<Model> model;
        std::shared_ptr<Theme> theme;
        std::map<std::string, std::string> paths;
        std::set<std::string> models;
        Json::Value config;
        std::string config_filename;
        char **argv;
        void *model_handle;
        Dustbin();
        Dustbin(const Dustbin &other) = delete;
        ~Dustbin();
        Dustbin & operator=(const Dustbin &other) = delete;
        bool init_model();
        bool init_theme();
        bool init_server();
        bool load_models();
        bool set_globals();
        void error_handler(int code, recycled::Connection &conn);
};
#endif