#ifndef DUSTBIN_INCLUDE_MODEL_H
#define DUSTBIN_INCLUDE_MODEL_H
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
static Json::Value EmptyObject(Json::objectValue);
class Model {
    public:
        virtual bool get_article(const std::string &id, Article &article) = 0;
        virtual int get_articles(std::vector<Article> &articles,
                                 int articles_per_page, int page,
                                 const Json::Value &query=EmptyObject) = 0;
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
        bool initialize(const std::string &connstr, const std::string &name);
        bool get_article(const std::string &id, Article &article);
        int get_articles(std::vector<Article> &articles,
                         int articles_per_page, int page,
                         const Json::Value &query=EmptyObject);
    private:
        std::shared_ptr<mongo::DBClientBase> conn;
        std::string name;
};
#endif