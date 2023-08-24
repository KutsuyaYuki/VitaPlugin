
#ifndef JSON_H
#define JSON_H

#include <jansson.h>

typedef struct {
    char **items; // Changed to a pointer
    int count;
    char *background_url;
} ParsedJSON;

ParsedJSON parse_json(const char *file_name);

#endif // JSON_H
