#ifndef DUSTBIN_INCLUDE_THEME_H
#define DUSTBIN_INCLUDE_THEME_H
#include <map>
#include <string>

namespace recycled {
namespace jinja2 {
class Environment;
}
}
enum class PageType {Normal, Tags, Archives};
class Theme {
    public:
        Theme() = default;
        ~Theme() = default;
        bool set_theme(const std::string &name);
        std::shared_ptr<recycled::jinja2::Environment> & get_environment();
        int get_articles_per_page(PageType type);
    private:
        std::shared_ptr<recycled::jinja2::Environment> env;
        std::map<PageType, int> articles_per_page;
};
#endif