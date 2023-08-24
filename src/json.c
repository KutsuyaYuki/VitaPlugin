
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

ParsedJSON parse_json(const char *file_name) {
    json_error_t error;
    json_t *root = json_load_file(file_name, 0, &error);
    ParsedJSON parsed_json = { .count = 0, .items = NULL, .background_url = NULL }; // Initialize items to NULL

    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return parsed_json;
    }

    // Get the background URL
    json_t *background_json = json_object_get(root, "background");
    if (json_is_string(background_json)) {
        parsed_json.background_url = strdup(json_string_value(background_json));
    }

    json_t *items = json_object_get(root, "items");
    if (!json_is_array(items)) {
        fprintf(stderr, "error: items is not an array\n");
        json_decref(root);
        return parsed_json;
    }

    int item_count = json_array_size(items);
    parsed_json.items = malloc(item_count * sizeof(char *)); // Allocate memory for items
    parsed_json.count = item_count;

    int i;
    for (i = 0; i < item_count; i++) {
        json_t *item = json_array_get(items, i);
        json_t *name_json = json_object_get(item, "name");
        const char *item_text = json_string_value(name_json);
        if (item_text) {
            parsed_json.items[i] = strdup(item_text);
        }
    }

    json_decref(root);
    return parsed_json;
}
