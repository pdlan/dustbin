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

struct CustomPage {
    std::string id;
    std::string title;
    std::string content;
    int order;
};

enum class Direction {Prev, Next};

namespace recycled {
namespace jinja2 {
class Environment;
}
}
static const Json::Value EmptyObject(Json::objectValue);
class Model {
    public:
        virtual bool check_config(const Json::Value &config) = 0;
        virtual bool initialize(const Json::Value &config) = 0;
        virtual bool auth(const std::string &username,
                          const std::string &password) = 0;
        virtual bool has_article(const std::string &id) = 0;
        virtual bool get_article(const std::string &id, Article &article) = 0;
        virtual bool get_prev_next_article_id(const std::string id,
                                              Direction dir,
                                              std::string &dest) = 0;
        virtual int get_articles(std::vector<Article> &articles,
                                 int articles_per_page, int page,
                                 const Json::Value &query=EmptyObject) = 0;
        virtual bool new_article(const Article &article) = 0;
        virtual bool edit_article(const std::string &id,
                                  const std::string &title,
                                  const std::string &content,
                                  const std::set<std::string> &tags) = 0;
        virtual bool delete_article(const std::string &id) = 0;
        virtual bool has_page(const std::string &id) = 0;
        virtual bool get_page(const std::string &id, CustomPage &page) = 0;
        virtual void get_pages(std::vector<CustomPage> &pages,
                               bool has_content=true) = 0;
        // Ignore page.order.
        virtual bool new_page(const CustomPage &page) = 0;
        virtual bool edit_page(const std::string &id, const std::string &title,
                               const std::string &content) = 0;
        virtual bool delete_page(const std::string &id) = 0;
        virtual bool reorder_pages(const std::map<std::string, int> &orders) = 0;
        virtual bool add_user(const std::string &username,
                              const std::string &password) = 0;
};
#endif