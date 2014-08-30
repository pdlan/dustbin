#ifndef DUSTBIN_INCLUDE_GLOBAL_FUNCTIONS_H
#define DUSTBIN_INCLUDE_GLOBAL_FUNCTIONS_H
#include <time.h>
#include <string>
#include <boost/python.hpp>
#include <jsoncpp/json/json.h>

Json::Value global_url_for_archives(int page);
Json::Value global_url_for_static(const std::string &path);
Json::Value global_url_for_page(int page);
Json::Value global_url_for_article(const std::string &id);
Json::Value global_url_for_feed();
Json::Value global_url_for_tag(const std::string &tag, int page);
Json::Value global_url_for_custom_page(const std::string &id);
Json::Value global_url_for_prev_article(const std::string &id);
Json::Value global_url_for_next_article(const std::string &id);
Json::Value global_get_articles(int articles_per_page, int page,
                                const Json::Value &query);
Json::Value global_get_article(const std::string &id);
Json::Value global_get_pages();
Json::Value global_get_page(const std::string &id);
boost::python::object global_py_import(const std::string &name);
#endif