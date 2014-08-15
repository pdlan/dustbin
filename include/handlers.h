#ifndef DUSTBIN_INCLUDE_HANDLERS_H
#define DUSTBIN_INCLUDE_HANDLERS_H
#include <recycled.h>

void page_handler(recycled::Connection &conn);
void article_handler(recycled::Connection &conn);
#endif