#ifndef DUSTBIN_INCLUDE_MODEL_H
#define DUSTBIN_INCLUDE_MODEL_H
#include <memory>
#include <string>
#include <set>
#include <vector>
#include <jsoncpp/json/json.h>

struct Article {
    std::string id;
    std::string title;
    std::string content;
    std::set<std::string> tags;
    time_t date;
};

namespace recycled {
namespace jinja2 {
class Environment;
}
}
static const Json::Value EmptyObject(Json::objectValue);
class Model {
    public:
        virtual bool auth(const std::string &username,
                          const std::string &password) = 0;
        virtual bool has_article(const std::string &id) = 0;
        virtual bool get_article(const std::string &id, Article &article) = 0;
        virtual int get_articles(std::vector<Article> &articles,
                                 int articles_per_page, int page,
                                 const Json::Value &query=EmptyObject) = 0;
        virtual bool new_article(const Article &article) = 0;
        virtual bool edit_article(const std::string &id,
                                  const std::string &title,
                                  const std::string &content,
                                  const std::set<std::string> &tags) = 0;
        virtual bool delete_article(const std::string &id) = 0;
};

namespace mongo {
class DBClientBase;
}
class MongoModel: public Model {
    public:
        MongoModel();
        MongoModel(const MongoModel &other) = delete;
        ~MongoModel();
        MongoModel & operator=(const MongoModel &other) = delete;
        bool initialize(const std::string &connstr, const std::string &name,
                        const Json::Value &auth = EmptyObject);
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
    private:
        std::shared_ptr<mongo::DBClientBase> conn;
        std::string name;
};
#endif