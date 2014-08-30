#include <time.h>
#include <string>
#include <memory>
#include <vector>
#include <jsoncpp/json/json.h>
#include <boost/python.hpp>
#include "dustbin.h"
#include "model.h"
#include "global_functions.h"
#include "utils.h"

Json::Value global_url_for_archives(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    if (page == 1) {
        return prefix + paths["archives"].asString();
    }
    const std::string &path = paths["archives-page"].asString();
    return prefix + replace(path, "<page>", std::to_string(page));
}

Json::Value global_url_for_static(const std::string &path) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + replace(paths["static"].asString(), "<path>", path);
}

Json::Value global_url_for_page(int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    if (page == 1) {
        return prefix + "/";
    }
    return prefix + replace(paths["page"].asString(),
                            "<page>", std::to_string(page));
}

Json::Value global_url_for_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + replace(paths["article"].asString(), "<id>", id);
}

Json::Value global_url_for_tag(const std::string &tag, int page) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    if (page == 1) {
        return prefix + replace(paths["tag"].asString(), "<tag>", tag);
    }
    const std::string &path =
        replace(paths["tag-page"].asString(), "<tag>", tag);
    return prefix + replace(path, "<page>", std::to_string(page));
}

Json::Value global_url_for_custom_page(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + replace(paths["custom-page"].asString(), "<id>", id);
}

Json::Value global_url_for_prev_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    std::string dest;
    if (!model->get_prev_next_article_id(id, Direction::Prev, dest)) {
        return Json::Value();
    }
    return global_url_for_article(dest);
}

Json::Value global_url_for_next_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    std::string dest;
    if (!model->get_prev_next_article_id(id, Direction::Next, dest)) {
        return Json::Value();
    }
    return global_url_for_article(dest);
}

Json::Value global_url_for_feed() {
    Dustbin &dustbin = Dustbin::get_instance();
    const Json::Value &paths = dustbin.get_config()["url"]["paths"];
    const std::string &prefix = paths["prefix"].asString();
    return prefix + paths["feed"].asString();
}

Json::Value global_get_articles(int articles_per_page, int page,
                                const Json::Value &query) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    std::vector<Article> articles;
    model->get_articles(articles, articles_per_page, page, query);
    Json::Value articles_json(Json::arrayValue);
    for (auto &article: articles) {
        Json::Value article_json;
        article_to_json(&article, article_json);
        articles_json.append(article_json);
    }
    return articles_json;
}

Json::Value global_get_article(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    Article article;
    if (!model->get_article(id, article)) {
        return Json::Value();
    }
    Json::Value article_json;
    article_to_json(&article, article_json);
    return article_json;
}

Json::Value global_get_pages() {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    std::vector<CustomPage> pages;
    model->get_pages(pages);
    Json::Value pages_json(Json::arrayValue);
    for (auto &page: pages) {
        Json::Value page_json;
        custom_page_to_json(page, page_json);
        pages_json.append(page_json);
    }
    return pages_json;
}

Json::Value global_get_page(const std::string &id) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return Json::Value();
    }
    CustomPage page;
    if (!model->get_page(id, page)) {
        return Json::Value();
    }
    Json::Value page_json;
    custom_page_to_json(page, page_json);
    return page_json;
}

boost::python::object global_py_import(const std::string &name) {
    using namespace boost;
    const python::object &obj = python::import(name.c_str());
    return obj;
}