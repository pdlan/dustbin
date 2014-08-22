#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string>
#include <exception>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <openssl/evp.h>
#include <recycled.h>
#include <recycled/jinja2.h>
#include "model.h"
#include "utils.h"
#include "theme.h"
#include "dustbin.h"

using namespace recycled;
using namespace recycled::jinja2;

static std::string base64decode(const std::string &str) {
    char *out = new char[str.length()];
    int length = EVP_DecodeBlock((unsigned char *)out,
                                 (unsigned char *)str.c_str(), str.length());
    return std::string(out, length);
}

static void article_to_json(const Article *article, Json::Value &json) {
    json["id"] = article->id;
    json["title"] = article->title;
    json["content"] = article->content;
    json["date"] = (Json::LargestUInt)article->date;
    Json::Value tags;
    for (auto &tag: article->tags) {
        tags.append(tag);
    }
    json["tags"] = tags;
}

static void custom_page_to_json(const CustomPage &page, Json::Value &json) {
    json["id"] = page.id;
    json["title"] = page.title;
    json["content"] = page.content;
    json["order"] = page.order;
}

static bool auth(const Connection &conn) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return false;
    }
    const std::string &header = conn.get_header("Authorization");
    if (header.empty()) {
        return false;
    }
    size_t delimiter_pos = header.find(" ");
    if (delimiter_pos == std::string::npos) {
        return false;
    }
    const std::string &type = header.substr(0, delimiter_pos);
    if (type != "Basic") {
        return false;
    }
    size_t length = header.length() - delimiter_pos - 1;
    const std::string &encoded = header.substr(delimiter_pos + 1, length);
    const std::string &decoded = base64decode(encoded);
    delimiter_pos = decoded.find(":");
    if (delimiter_pos == std::string::npos) {
        return false;
    }
    const std::string &username = decoded.substr(0, delimiter_pos);
    length = decoded.length() - delimiter_pos - 1;
    const std::string &password = decoded.substr(delimiter_pos + 1, length);
    return model->auth(username, password);
}

static void require_auth(Connection &conn) {
    conn.add_header("WWW-Authenticate", "Basic realm=\"Admin\"");
    conn.send_error(401);
}

static bool is_number(const std::string &str) {
    for (char c: str) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

static void set_custom_pages_argument(ValueMap &arguments) {
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        return;
    }
    std::vector<CustomPage> pages;
    Json::Value pages_json(Json::arrayValue);
    model->get_pages(pages, false);
    for (auto &page: pages) {
        Json::Value page_json;
        page_json["id"] = page.id;
        page_json["title"] = page.title;
        page_json["order"] = page.order;
        pages_json.append(page_json);
    }
    arguments.insert(std::make_pair("custom_pages", pages_json));
}

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
    const std::shared_ptr<Model> &model = dustbin.get_model();
    const std::shared_ptr<Theme> &theme = dustbin.get_theme();
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
    set_custom_pages_argument(arguments);
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
                Json::Value article_json;
                article_to_json(article, article_json);
                articles_json.append(article_json);
            }
            archive["articles"] = articles_json;
            archive["year"] = year;
            archives.append(archive);
        }
        arguments["archives"] = archives;
    } else {
        for (auto &article: articles) {
            Json::Value article_json;
            article_to_json(&article, article_json);
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
    const std::shared_ptr<Model> &model = dustbin.get_model();
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
    set_custom_pages_argument(arguments);
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void custom_page_handler(Connection &conn) {
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &id = conn.get_path_argument("id");
    if (id.empty()) {
        conn.send_error(404);
        return;
    }
    CustomPage page;
    if (!model->get_page(id, page)) {
        conn.send_error(404);
        return;
    }
    const Template &temp = dustbin.get_template("main/custom_page.html");
    Json::Value page_json;
    custom_page_to_json(page, page_json);
    ValueMap arguments = {
        {"page", page_json}
    };
    set_custom_pages_argument(arguments);
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void admin_index_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    const Template &temp = dustbin.get_template("admin/index.html");
    if (!temp.render({}, conn)) {
        conn.send_error(500);
    }
}

void admin_upload_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    const UploadFile *file_upload = conn.get_file("image");
    if (!file_upload) {
        conn.send_error(400);
        return;
    }
    const Json::Value &config = dustbin.get_config();
    if (!config["enable-upload"].asBool()) {
        conn.send_error(403);
        return;
    }
    const std::string &upload_dir = config["upload-dir"].asString() + "/";
    const std::string &upload_path =
        config["url"]["paths"]["upload"].asString();
    std::string filename = file_upload->filename;
    std::ifstream ifile;
    for (int i = 0; ; ++i) {
        ifile.open(upload_dir + filename);
        if (!ifile.is_open()) {
            break;
        } else {
            ifile.close();
        }
        filename = std::to_string(i) + "-" + filename;
    }
    std::ofstream ofile(upload_dir + filename);
    ofile.write(file_upload->data, file_upload->size);
    ofile.close();
    std::string static_path = replace(upload_path, "<path>", filename);
    Json::Value result, upload, links;
    links["original"] = static_path;
    upload["links"] = links;
    result["upload"] = upload;
    Json::FastWriter writer;
    conn.add_header("Content-Type", "application/json");
    conn.write(writer.write(result));
}

void admin_articles_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
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
    std::vector<Article> articles;
    int pages = model->get_articles(articles, 20, page);
    if (page > pages) {
        conn.send_error(404);
    }
    ValueMap arguments;
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
    Json::Value articles_json;
    for (auto &article: articles) {
        Json::Value article_json;
        article_to_json(&article, article_json);
        articles_json.append(article_json);
    }
    arguments["articles"] = articles_json;
    const Template &temp = dustbin.get_template("admin/articles.html");
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void admin_new_article_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    if (conn.get_method() == HTTPMethod::GET) {
        conn.add_header("Content-Type", "text/html");
        const Template &temp = dustbin.get_template("admin/new_edit_article.html");
        if (!temp.render({{"new", true}}, conn)) {
            conn.send_error(500);
        }
        return;
    }
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    Json::Reader reader;
    Json::Value tags(Json::arrayValue);
    const std::string &title = conn.get_body_argument("title");
    const std::string &content = conn.get_body_argument("content");
    time_t date = time(NULL);
    if (!reader.parse(conn.get_body_argument("tags"), tags) ||
        tags.type() != Json::arrayValue) {
        tags = Json::Value(Json::arrayValue);
    }
    std::string id = title;
    for (int i = 1; ; ++i) {
        if (!model->has_article(id)) {
            break;
        }
        id = title + "-" + std::to_string(i);
    }
    Article article;
    article.id = id;
    article.title = title;
    article.content = content;
    article.date = date;
    for (auto it = tags.begin(); it != tags.end(); ++it) {
        const Json::Value &tag = *it;
        if (!tag.isString()) {
            continue;
        }
        article.tags.insert(tag.asString());
    }
    if (!model->new_article(article)) {
        conn.send_error(500);
        return;
    }
    conn.redirect("/admin/articles/");
}

void admin_edit_article_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &id = conn.get_path_argument("id");
    if (id.empty()) {
        conn.send_error(404);
        return;
    }
    if (conn.get_method() == HTTPMethod::GET) {
        conn.add_header("Content-Type", "text/html");
        Article article;
        if (!model->get_article(id, article)) {
            conn.send_error(404);
            return;
        }
        Json::Value article_json;
        Json::Value tags(Json::arrayValue);
        article_json["id"] = article.id;
        article_json["title"] = article.title;
        article_json["content"] = article.content;
        article_json["date"] = (Json::LargestUInt)article.date;
        for (auto &tag: article.tags) {
            tags.append(tag);
        }
        Json::FastWriter writer;
        article_json["tags"] = writer.write(tags);
        ValueMap arguments = {
            {"edit", true},
            {"article", article_json}
        };
        const Template &temp = dustbin.get_template("admin/new_edit_article.html");
        if (!temp.render(arguments, conn)) {
            conn.send_error(500);
        }
        return;
    }
    if (!model) {
        conn.send_error(500);
        return;
    }
    Json::Reader reader;
    Json::Value tags_json(Json::arrayValue);
    const std::string &title = conn.get_body_argument("title");
    const std::string &content = conn.get_body_argument("content");
    if (!reader.parse(conn.get_body_argument("tags"), tags_json) ||
        tags_json.type() != Json::arrayValue) {
        tags_json = Json::Value(Json::arrayValue);
    }
    std::set<std::string> tags;
    for (auto it = tags_json.begin(); it != tags_json.end(); ++it) {
        const Json::Value &tag = *it;
        if (!tag.isString()) {
            continue;
        }
        tags.insert(tag.asString());
    }
    if (!model->edit_article(id, title, content, tags)) {
        conn.send_error(404);
        return;
    }
    conn.redirect("/admin/articles/");
}

void admin_delete_article_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &id = conn.get_path_argument("id");
    if (id.empty()) {
        conn.send_error(404);
        return;
    }
    if (!model->delete_article(id)) {
        conn.send_error(404);
        return;
    }
    conn.write("success");
}

void admin_config_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    const std::string &config_filename = dustbin.get_config_filename();
    const Template &temp = dustbin.get_template("admin/config.html");
    std::ostringstream config_ss;
    if (conn.get_method() == HTTPMethod::GET) {
        std::ifstream fs(config_filename);
        if (!fs.is_open()) {
            conn.send_error(500);
            return;
        }
        config_ss << fs.rdbuf();
        fs.close();
    } else {
        const std::string &config = conn.get_body_argument("config");
        if (config.empty()) {
            conn.send_error(400);
            return;
        }
        config_ss.str(config);
        std::ofstream fs(config_filename);
        if (!fs.is_open()) {
            conn.send_error(500);
            return;
        }
        fs << config;
        fs.close();
    }
    ValueMap arguments = {
        {"config", config_ss.str()}
    };
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void admin_restart_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    dustbin.restart();
}

void admin_pages_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    conn.add_header("Content-Type", "text/html");
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    std::vector<CustomPage> pages;
    model->get_pages(pages, true);
    Json::Value pages_json(Json::arrayValue);
    for (auto &page: pages) {
        Json::Value page_json;
        custom_page_to_json(page, page_json);
        pages_json.append(page_json);
    }
    ValueMap arguments = {
        {"pages", pages_json}
    };
    const Template &temp = dustbin.get_template("admin/pages.html");
    if (!temp.render(arguments, conn)) {
        conn.send_error(500);
    }
}

void admin_new_page_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    if (conn.get_method() == HTTPMethod::GET) {
        conn.add_header("Content-Type", "text/html");
        const Template &temp = dustbin.get_template("admin/new_edit_page.html");
        if (!temp.render({{"new", true}}, conn)) {
            conn.send_error(500);
        }
        return;
    }
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &title = conn.get_body_argument("title");
    std::string id;
    if (is_number(id)) {
        id = "p" + title;
    } else {
        id = title;
    }
    for (int i = 1; ; ++i) {
        if (!model->has_page(id)) {
            break;
        }
        id = title + "-" + std::to_string(i);
    }
    const std::string &content = conn.get_body_argument("content");
    CustomPage page;
    page.id = id;
    page.title = title;
    page.content = content;
    if (!model->new_page(page)) {
        conn.send_error(500);
        return;
    }
    conn.redirect("/admin/pages/");
}

void admin_edit_page_handler(recycled::Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &id = conn.get_path_argument("id");
    if (id.empty()) {
        conn.send_error(404);
        return;
    }
    if (conn.get_method() == HTTPMethod::GET) {
        conn.add_header("Content-Type", "text/html");
        CustomPage page;
        if (!model->get_page(id, page)) {
            conn.send_error(404);
            return;
        }
        Json::Value page_json;
        custom_page_to_json(page, page_json);
        ValueMap arguments = {
            {"edit", true},
            {"page", page_json}
        };
        const Template &temp = dustbin.get_template("admin/new_edit_page.html");
        if (!temp.render(arguments, conn)) {
            conn.send_error(500);
        }
        return;
    }
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &title = conn.get_body_argument("title");
    const std::string &content = conn.get_body_argument("content");
    if (!model->edit_page(id, title, content)) {
        conn.send_error(404);
        return;
    }
    conn.redirect("/admin/pages/");
}

void admin_delete_page_handler(recycled::Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    const std::string &id = conn.get_path_argument("id");
    if (id.empty()) {
        conn.send_error(404);
        return;
    }
    if (!model->delete_page(id)) {
        conn.send_error(404);
        return;
    }
    conn.write("success");
}

void admin_reorder_pages_handler(Connection &conn) {
    if (!auth(conn)) {
        require_auth(conn);
        return;
    }
    Dustbin &dustbin = Dustbin::get_instance();
    const std::shared_ptr<Model> &model = dustbin.get_model();
    if (!model) {
        conn.send_error(500);
        return;
    }
    std::string orders(conn.get_body(), conn.get_body_size());
    if (orders.empty()) {
        conn.send_error(400);
        return;
    }
    Json::Value orders_json;
    Json::Reader reader;
    if (!reader.parse(orders, orders_json)) {
        conn.send_error(400);
        return;
    }
    if (!orders_json.isObject()) {
        conn.send_error(400);
        return;
    }
    std::map<std::string, int> orders_map;
    const Json::Value::Members &members = orders_json.getMemberNames();
    for (auto &id: members) {
        const Json::Value &value = orders_json[id];
        if (!value.isInt()) {
            conn.send_error(400);
            return;
        }
        int order = value.asInt();
        orders_map.insert(std::make_pair(id, order));
    }
    if (!model->reorder_pages(orders_map)) {
        conn.send_error(500);
        return;
    }
    conn.write("success");
}