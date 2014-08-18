#ifndef DUSTBIN_INCLUDE_HANDLERS_H
#define DUSTBIN_INCLUDE_HANDLERS_H
#include <recycled.h>

void page_handler(recycled::Connection &conn);
void archives_handler(recycled::Connection &conn);
void tag_handler(recycled::Connection &conn);
void article_handler(recycled::Connection &conn);
void admin_index_handler(recycled::Connection &conn);
void admin_upload_handler(recycled::Connection &conn);
void admin_articles_handler(recycled::Connection &conn);
void admin_new_article_handler(recycled::Connection &conn);
void admin_edit_article_handler(recycled::Connection &conn);
void admin_delete_article_handler(recycled::Connection &conn);
void admin_config_handler(recycled::Connection &conn);
void admin_restart_handler(recycled::Connection &conn);
#endif