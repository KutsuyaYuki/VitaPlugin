
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

ParsedJSON parse_json(const char *file_name) {
    json_error_t error;
    json_t *root = json_load_file(file_name, 0, &error);
    ParsedJSON parsed_json = { .count = 0 };

    if (!root) {
        fprintf(stderr, "error: on line %d: %s\\n", error.line, error.text);
        return parsed_json;
    }

    json_t *items = json_object_get(root, "items");
    if (!json_is_array(items)) {
        fprintf(stderr, "error: items is not an array\\n");
        json_decref(root);
        return parsed_json;
    }

    int i;
    for (i = 0; i < json_array_size(items) && i < MAX_ITEMS; i++) {
        json_t *item = json_array_get(items, i);
        json_t *name_json = json_object_get(item, "name"); // Get the "name" field
        const char *item_text = json_string_value(name_json); // Get the string value of "name"

        if (item_text) {
            parsed_json.items[i] = strdup(item_text);
            parsed_json.count++;
        }
    }

    json_decref(root);
    return parsed_json;
}
