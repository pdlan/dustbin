#ifndef DUSTBIN_INCLUDE_MODELS_MONGO_H
#define DUSTBIN_INCLUDE_MODELS_MONGO_H
#include <memory>
#include <string>
#include <set>
#include <vector>
#include <jsoncpp/json/json.h>
#include "model.h"

namespace mongo {
class DBClientBase;
}
class MongoModel: public Model {
    public:
        MongoModel();
        MongoModel(const MongoModel &other) = delete;
        ~MongoModel();
        MongoModel & operator=(const MongoModel &other) = delete;
        bool check_config(const Json::Value &config);
        bool initialize(const Json::Value &config);
        bool auth(const std::string &username, const std::string &password);
        bool has_article(const std::string &id);
        bool get_article(const std::string &id, Article &article);
        int get_articles(std::vector<Article> &articles,
                         int articles_per_page, int page,
                         const Json::Value &query=EmptyObject);
        bool new_article(const Article &article);
        bool edit_article(const std::string &id,
                          const std::string &title,
                          const std::string &content,
                          const std::set<std::string> &tags);
        bool delete_article(const std::string &id);
        bool has_page(const std::string &id);
        bool get_page(const std::string &id, CustomPage &page);
        void get_pages(std::vector<CustomPage> &pages,
                       bool has_content=true);
    private:
        std::shared_ptr<mongo::DBClientBase> conn;
        std::string name;
};
#endif