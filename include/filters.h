#ifndef DUSTBIN_INCLUDE_FILTERS_H
#define DUSTBIN_INCLUDE_FILTERS_H
#include <time.h>
#include <string>
#include <jsoncpp/json/json.h>

Json::Value filter_url_for_archives(int page);
Json::Value filter_url_for_static(const std::string &path);
Json::Value filter_url_for_page(int page);
Json::Value filter_url_for_article(const std::string &id);
Json::Value filter_url_for_tag(const std::string &tag, int page);
Json::Value filter_url_for_custom_page(const std::string &id);
Json::Value filter_url_for_prev_article(const std::string &id);
Json::Value filter_url_for_next_article(const std::string &id);
#endif