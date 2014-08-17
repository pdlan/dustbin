#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <mongo/client/dbclient.h>
#include <jsoncpp/json/json.h>
#include <recycled/jinja2.h>
#include "model.h"
#include "utils.h"

using namespace mongo;

static inline bool check_bsonobj(const BSONObj &obj,
                                 const std::map<std::string, BSONType> &items) {
    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        const std::string &name = it->first;
        BSONType type = it->second;
        if (!obj.hasField(name)) {
            return false;
        }
        if (obj[name].type() != type) {
            return false;
        }
    }
    return true;
}

static inline bool bsonobj_to_article(const BSONObj &obj, Article &article) {
    if (!check_bsonobj(obj, {
        {"id", String},
        {"title", String},
        {"content", String},
        {"tags", Array},
        {"date", Date}
    })) {
        return false;
    }
    article.id = obj.getStringField("id");
    article.title = obj.getStringField("title");
    article.content = obj.getStringField("content");
    const std::vector<BSONElement> &tags = obj.getField("tags").Array();
    for (auto &tag: tags) {
        if (tag.type() != String) {
            return false;
        }
        const std::string &tag_str = tag.String();
        if (article.tags.count(tag_str)) {
            continue;
        }
        article.tags.insert(tag_str);
    }
    const Date_t &date = obj.getField("date").Date();
    article.date = date.toTimeT();
    return true;
}

MongoModel::MongoModel() {}

MongoModel::~MongoModel() {}

bool MongoModel::initialize(const std::string &connstr, const std::string &name,
                            const Json::Value &auth) {
    std::string errmsg;
    ConnectionString cs = ConnectionString::parse(connstr, errmsg);
    if (!cs.isValid()) {
        std::cerr << "Invalid connection string\n"
                  << "----Error message: " << errmsg << std::endl;
        return false;
    }
    this->conn = std::shared_ptr<DBClientBase>(cs.connect(errmsg));
    if (!this->conn) {
        std::cerr << "Cannot connect to database.\n"
                  << "----Error messgage: " << errmsg << std::endl;
        return false;
    }
    if (!auth.empty()) {
        if (!check_members(auth, {
            {"username", Json::stringValue},
            {"password", Json::stringValue}
        })){
            return false;
        }
        const std::string &username = auth["username"].asString();
        const std::string &password = auth["password"].asString();
        if (!this->conn->auth(name, username, password, errmsg)) {
            std::cerr << "Failed to authorize access to database.\n"
                      << "----Error message: " << errmsg << std::endl;
            return false;
        }
    }
    this->name = name;
    return true;
}

bool MongoModel::get_article(const std::string &id, Article &article) {
    if (!this->conn) {
        return false;
    }
    auto cursor = this->conn->query(this->name + ".articles",
                                    QUERY("id" << id));
    if (cursor->more()) {
        const BSONObj &obj = cursor->next();
        Article article_new;
        if (!bsonobj_to_article(obj, article_new)) {
            return false;
        }
        article = article_new;
        return true;
    }
    return false;
}

int MongoModel::get_articles(std::vector<Article> &articles,
                             int articles_per_page, int page,
                             const Json::Value &query) {
    if (!this->conn) {
        return 0;
    }
    Json::FastWriter writer;
    Query query_mongo(writer.write(query));
    query_mongo = query_mongo.sort("date", -1);
    int limit = 0, skip = 0;
    if (articles_per_page > 0) {
        limit = articles_per_page;
        skip = (page - 1) * articles_per_page;
    }
    auto cursor = this->conn->query(this->name + ".articles",
                                    query_mongo, limit, skip);
    int count;
    for (count = 0; cursor->more(); ++count) {
        const BSONObj &obj = cursor->next();
        Article article;
        if (!bsonobj_to_article(obj, article)) {
            continue;
        }
        articles.push_back(article);
    }
    int pages;
    if (articles_per_page > 0) {
        pages = ceil((double)count / (double)articles_per_page);
    } else {
        pages = 1;
    }
    return pages;
}