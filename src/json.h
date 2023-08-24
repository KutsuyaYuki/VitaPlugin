
#ifndef JSON_H
#define JSON_H

#include <jansson.h>

#define MAX_ITEMS 100

typedef struct {
    char *items[MAX_ITEMS];
    int count;
} ParsedJSON;

ParsedJSON parse_json(const char *file_name);

#endif // JSON_H
