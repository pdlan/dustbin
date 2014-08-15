#include <stdio.h>
#include <string>
#include <exception>
#include <memory>
#include <vector>
#include <recycled.h>
#include <recycled/jinja2.h>
#include "model.h"
#include "theme.h"
#include "dustbin.h"

using namespace recycled;
using namespace recycled::jinja2;

void page_handler(Connection &conn) {
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    int page;
    try {
        page = std::stoi(conn.get_path_argument("page"));
    } catch (const std::exception &e) {
        page = 1;
    }
    if (page < 1) {
        conn.send_error(404);
        return;
    }
    std::shared_ptr<Model> &model = dustbin.get_model();
    std::shared_ptr<Theme> &theme = dustbin.get_theme();
    if (!model || !theme) {
        conn.send_error(500);
        return;
    }
    int articles_per_page = theme->get_articles_per_page(PageType::Normal);
    ValueMap arguments;
    Json::Value articles_json;
    std::vector<Article> articles;
    int pages = model->get_articles(articles, articles_per_page, page);
    if (page > pages) {
        conn.send_error(404);
    }
    if (page > 1 && page <= pages) {
        arguments["has_prev"] = true;
        arguments["prev"] = page - 1;
    }
    if (page < pages && page > 0) {
        arguments["has_next"] = true;
        arguments["next"] = page + 1;
    }
    arguments["pages"] = pages;
    arguments["page"] = page;
    for (auto &article: articles) {
        Json::Value article_json(Json::objectValue), tags(Json::arrayValue);
        article_json["id"] = article.id;
        article_json["title"] = article.title;
        article_json["content"] = article.content;
        article_json["date"] = (Json::LargestUInt)article.date;
        for (auto &tag: article.tags) {
            tags.append(tag);
        }
        article_json["tags"] = tags;
        articles_json.append(article_json);
    }
    arguments["articles"] = articles_json;
    const Template &temp = dustbin.get_template("main/page.html");
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void article_handler(Connection &conn) {
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &id = conn.get_path_argument("id");
    if (id.empty()) {
        conn.send_error(404);
        return;
    }
    Article article;
    model->get_article(id, article);
    const Template &temp = dustbin.get_template("main/test.html");
    if (!temp.render({{"id", id}}, conn)) {
        conn.send_error(500);
    }
}