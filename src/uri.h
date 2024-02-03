#include "string_utils.h"

#ifndef _URI_H
#define _URI_H

typedef struct {
  StringView scheme;
  StringView host;
  StringView path;
  StringView query;
  StringView fragment;
} UriComponents;

void uri_parse(StringView uri, UriComponents *components);
StringBuffer *uri_components_join(UriComponents *components);

#endif // _URI_H
