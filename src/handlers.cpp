#include <stdio.h>
#include <time.h>
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

static void page_tag_archives_handler(Connection &conn, PageType type) {
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    int page;
    Json::Value query(Json::objectValue);
    try {
        page = std::stoi(conn.get_path_argument("page"));
    } catch (const std::exception &e) {
        page = 1;
    }
    if (page < 1) {
        conn.send_error(404);
        return;
    }
    ValueMap arguments;
    if (type == PageType::Tag) {
        std::string tag = conn.get_path_argument("tag");
        if (tag.empty()) {
            conn.send_error(404);
            return;
        }
        query["tags"] = tag;
        arguments["name"] = tag;
    }
    std::shared_ptr<Model> &model = dustbin.get_model();
    std::shared_ptr<Theme> &theme = dustbin.get_theme();
    if (!model || !theme) {
        conn.send_error(500);
        return;
    }
    int articles_per_page = theme->get_articles_per_page(type);
    Json::Value articles_json;
    std::vector<Article> articles;
    int pages = model->get_articles(articles, articles_per_page, page, query);
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
    if (type == PageType::Archives) {
        Json::Value archives;
        std::vector<const Article *> *year_articles = nullptr;
        std::map<int, std::vector<const Article *> *> year_articles_map;
        for (auto &article: articles) {
            tm *info = localtime(&article.date);
            int year = 1900 + info->tm_year;
            auto it = year_articles_map.find(year);
            if (it == year_articles_map.end()) {
                year_articles = new std::vector<const Article *>;
                year_articles_map[year] = year_articles;
            } else {
                year_articles = it->second;
            }
            year_articles->push_back(&article);
        }
        for (auto it = year_articles_map.begin();
             it != year_articles_map.end(); ++it) {
            int year = it->first;
            std::vector<const Article *> *year_articles = it->second;
            Json::Value archive, articles_json;
            for (auto &article: *year_articles) {
                Json::Value article_json, tags;
                article_json["id"] = article->id;
                article_json["title"] = article->title;
                article_json["content"] = article->content;
                article_json["date"] = (Json::LargestUInt)article->date;
                for (auto &tag: article->tags) {
                    tags.append(tag);
                }
                article_json["tags"] = tags;
                articles_json.append(article_json);
            }
            archive["articles"] = articles_json;
            archive["year"] = year;
            archives.append(archive);
        }
        arguments["archives"] = archives;
    } else {
        for (auto &article: articles) {
            Json::Value article_json, tags;
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
    }
    Template temp;
    switch (type) {
        case PageType::Normal:
            temp = dustbin.get_template("main/page.html");
            break;
        case PageType::Tag:
            temp = dustbin.get_template("main/tag.html");
            break;
        case PageType::Archives:
            temp = dustbin.get_template("main/archives.html");
            break;
    }
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void page_handler(Connection &conn) {
    page_tag_archives_handler(conn, PageType::Normal);
}

void archives_handler(Connection &conn) {
    page_tag_archives_handler(conn, PageType::Archives);
}

void tag_handler(Connection &conn) {
    page_tag_archives_handler(conn, PageType::Tag);
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
    if (!model->get_article(id, article)) {
        conn.send_error(404);
        return;
    }
    const Template &temp = dustbin.get_template("main/article.html");
    Json::Value article_json, tags;
    article_json["id"] = article.id;
    article_json["title"] = article.title;
    article_json["content"] = article.content;
    article_json["date"] = (Json::LargestUInt)article.date;
    for (auto &tag: article.tags) {
        tags.append(tag);
    }
    article_json["tags"] = tags;
    ValueMap arguments = {
        {"article", article_json}
    };
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}