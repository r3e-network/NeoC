/*
  cJSON - JSON parser in C
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "cJSON.h"

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1024
#endif

typedef struct {
    void *(*allocate)(size_t size);
    void (*deallocate)(void *ptr);
} cjson_hooks_internal;

static cjson_hooks_internal global_hooks = { malloc, free };
static const char *global_error_pointer = NULL;

static void *cjson_malloc(size_t size) {
    if (!global_hooks.allocate) {
        return NULL;
    }
    return global_hooks.allocate(size);
}

static void cjson_free(void *ptr) {
    if (ptr && global_hooks.deallocate) {
        global_hooks.deallocate(ptr);
    }
}

void *cJSON_malloc(size_t size) {
    return cjson_malloc(size);
}

void cJSON_free(void *ptr) {
    cjson_free(ptr);
}

void cJSON_InitHooks(cJSON_Hooks *hooks) {
    if (!hooks) {
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        return;
    }
    global_hooks.allocate = hooks->malloc_fn ? hooks->malloc_fn : malloc;
    global_hooks.deallocate = hooks->free_fn ? hooks->free_fn : free;
}

static cJSON *cjson_new_item(void) {
    cJSON *node = (cJSON *)cjson_malloc(sizeof(cJSON));
    if (node) {
        memset(node, 0, sizeof(cJSON));
    }
    return node;
}

static void cjson_delete_item(cJSON *item) {
    if (!item) {
        return;
    }

    if (!(item->type & cJSON_IsReference) && item->child) {
        cJSON *child = item->child;
        while (child) {
            cJSON *next = child->next;
            cjson_delete_item(child);
            child = next;
        }
    }

    if (!(item->type & cJSON_IsReference) && item->valuestring) {
        cjson_free(item->valuestring);
    }

    if (!(item->type & cJSON_StringIsConst) && item->string) {
        cjson_free(item->string);
    }

    cjson_free(item);
}

void cJSON_Delete(cJSON *item) {
    cjson_delete_item(item);
}

static void skip_whitespace(const char **input) {
    while (**input && isspace((unsigned char)**input)) {
        (*input)++;
    }
}

static char *cjson_strdup(const char *string) {
    if (!string) {
        return NULL;
    }
    size_t len = strlen(string) + 1;
    char *copy = (char *)cjson_malloc(len);
    if (copy) {
        memcpy(copy, string, len);
    }
    return copy;
}

static const char *parse_value(cJSON *item, const char *input, size_t depth);
static __attribute__((unused)) char *print_value_buffered(const cJSON *item, int depth, int fmt, int *out_len);
static __attribute__((unused)) char *print_value_buffered(const cJSON *item, int depth, int fmt, int *out_len) {
    (void)item;
    (void)depth;
    (void)fmt;
    (void)out_len;
    return NULL;
}

static const char *parse_number(cJSON *item, const char *input) {
    char *endptr = NULL;
    double number = strtod(input, &endptr);
    if (endptr == input) {
        global_error_pointer = input;
        return NULL;
    }
    item->type = cJSON_Number;
    item->valuedouble = number;
    item->valueint = (int)number;
    return endptr;
}

static unsigned parse_hex4(const char *input) {
    unsigned value = 0;
    for (int i = 0; i < 4 && input[i]; ++i) {
        unsigned char c = (unsigned char)input[i];
        value <<= 4;
        if (c >= '0' && c <= '9') {
            value |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            value |= (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            value |= (c - 'A' + 10);
        } else {
            return 0;
        }
    }
    return value;
}

static const char *parse_string(cJSON *item, const char *input) {
    if (*input != '"') {
        global_error_pointer = input;
        return NULL;
    }
    input++;

    const char *ptr = input;
    size_t length = 0;
    while (*ptr && *ptr != '"') {
        if (*ptr == '\\' && ptr[1]) {
            ptr++;
        }
        ptr++;
        length++;
    }

    if (*ptr != '"') {
        global_error_pointer = input;
        return NULL;
    }

    char *string = (char *)cjson_malloc(length + 1);
    if (!string) {
        return NULL;
    }

    size_t index = 0;
    ptr = input;
    while (*ptr && *ptr != '"') {
        if (*ptr != '\\') {
            string[index++] = *ptr++;
        } else {
            ptr++;
            switch (*ptr) {
                case '"': string[index++] = '"'; break;
                case '\\': string[index++] = '\\'; break;
                case '/': string[index++] = '/'; break;
                case 'b': string[index++] = '\b'; break;
                case 'f': string[index++] = '\f'; break;
                case 'n': string[index++] = '\n'; break;
                case 'r': string[index++] = '\r'; break;
                case 't': string[index++] = '\t'; break;
                case 'u': {
                    unsigned uc = parse_hex4(ptr + 1);
                    ptr += 4;
                    if (uc < 0x80) {
                        string[index++] = (char)uc;
                    } else if (uc < 0x800) {
                        string[index++] = (char)(0xC0 | (uc >> 6));
                        string[index++] = (char)(0x80 | (uc & 0x3F));
                    } else {
                        string[index++] = (char)(0xE0 | (uc >> 12));
                        string[index++] = (char)(0x80 | ((uc >> 6) & 0x3F));
                        string[index++] = (char)(0x80 | (uc & 0x3F));
                    }
                    break;
                }
                default:
                    string[index++] = *ptr;
                    break;
            }
            ptr++;
        }
    }
    string[index] = '\0';

    item->type = cJSON_String;
    item->valuestring = string;
    return ptr + 1;
}

static const char *parse_array(cJSON *item, const char *input, size_t depth) {
    if (*input != '[') {
        global_error_pointer = input;
        return NULL;
    }
    input++;
    skip_whitespace(&input);

    item->type = cJSON_Array;
    item->child = NULL;

    if (*input == ']') {
        return input + 1;
    }

    cJSON *child = cjson_new_item();
    if (!child) {
        return NULL;
    }
    item->child = child;

    input = parse_value(child, input, depth + 1);
    if (!input) {
        return NULL;
    }
    skip_whitespace(&input);

    while (*input == ',') {
        input++;
        skip_whitespace(&input);

        cJSON *new_item = cjson_new_item();
        if (!new_item) {
            return NULL;
        }
        child->next = new_item;
        new_item->prev = child;
        child = new_item;

        input = parse_value(child, input, depth + 1);
        if (!input) {
            return NULL;
        }
        skip_whitespace(&input);
    }

    if (*input != ']') {
        global_error_pointer = input;
        return NULL;
    }
    return input + 1;
}

static const char *parse_object(cJSON *item, const char *input, size_t depth) {
    if (*input != '{') {
        global_error_pointer = input;
        return NULL;
    }
    input++;
    skip_whitespace(&input);

    item->type = cJSON_Object;
    item->child = NULL;

    if (*input == '}') {
        return input + 1;
    }

    cJSON *child = cjson_new_item();
    if (!child) {
        return NULL;
    }
    item->child = child;

    input = parse_string(child, input);
    if (!input || !child->valuestring) {
        return NULL;
    }
    child->string = child->valuestring;
    child->valuestring = NULL;

    skip_whitespace(&input);
    if (*input != ':') {
        global_error_pointer = input;
        return NULL;
    }
    input++;
    skip_whitespace(&input);

    input = parse_value(child, input, depth + 1);
    if (!input) {
        return NULL;
    }
    skip_whitespace(&input);

    while (*input == ',') {
        input++;
        skip_whitespace(&input);

        cJSON *new_item = cjson_new_item();
        if (!new_item) {
            return NULL;
        }
        child->next = new_item;
        new_item->prev = child;
        child = new_item;

        input = parse_string(child, input);
        if (!input || !child->valuestring) {
            return NULL;
        }
        child->string = child->valuestring;
        child->valuestring = NULL;

        skip_whitespace(&input);
        if (*input != ':') {
            global_error_pointer = input;
            return NULL;
        }
        input++;
        skip_whitespace(&input);

        input = parse_value(child, input, depth + 1);
        if (!input) {
            return NULL;
        }
        skip_whitespace(&input);
    }

    if (*input != '}') {
        global_error_pointer = input;
        return NULL;
    }
    return input + 1;
}

static const char *parse_value(cJSON *item, const char *input, size_t depth) {
    if (depth > CJSON_NESTING_LIMIT) {
        global_error_pointer = input;
        return NULL;
    }

    skip_whitespace(&input);
    if (!input || !*input) {
        global_error_pointer = input;
        return NULL;
    }

    switch (*input) {
        case '"':
            return parse_string(item, input);
        case '{':
            return parse_object(item, input, depth);
        case '[':
            return parse_array(item, input, depth);
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parse_number(item, input);
        case 't':
            if (!strncmp(input, "true", 4)) {
                item->type = cJSON_True;
                item->valueint = 1;
                return input + 4;
            }
            break;
        case 'f':
            if (!strncmp(input, "false", 5)) {
                item->type = cJSON_False;
                item->valueint = 0;
                return input + 5;
            }
            break;
        case 'n':
            if (!strncmp(input, "null", 4)) {
                item->type = cJSON_NULL;
                return input + 4;
            }
            break;
        default:
            break;
    }

    global_error_pointer = input;
    return NULL;
}

cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated) {
    if (!value) {
        return NULL;
    }

    cJSON *item = cjson_new_item();
    if (!item) {
        return NULL;
    }

    const char *end = parse_value(item, value, 0);
    if (!end) {
        cjson_delete_item(item);
        return NULL;
    }

    skip_whitespace(&end);
    if (require_null_terminated && *end) {
        cjson_delete_item(item);
        global_error_pointer = end;
        return NULL;
    }

    if (return_parse_end) {
        *return_parse_end = end;
    }

    return item;
}

cJSON *cJSON_Parse(const char *value) {
    return cJSON_ParseWithOpts(value, NULL, 0);
}

cJSON *cJSON_ParseWithLength(const char *value, size_t length) {
    const char *end = NULL;
    cJSON *item = cJSON_ParseWithOpts(value, &end, 0);
    if (!item) {
        return NULL;
    }
    if ((size_t)(end - value) != length) {
        cjson_delete_item(item);
        global_error_pointer = value + length;
        return NULL;
    }
    return item;
}

/* Printing */
static int ensure_capacity(char **buffer, int *length, int required) {
    if (*length >= required) {
        return 1;
    }
    int new_length = (*length == 0) ? 256 : *length;
    while (new_length < required) {
        new_length *= 2;
    }
    char *new_buffer = (char *)cjson_malloc((size_t)new_length);
    if (!new_buffer) {
        return 0;
    }
    if (*buffer) {
        memcpy(new_buffer, *buffer, (size_t)*length);
        cjson_free(*buffer);
    }
    *buffer = new_buffer;
    *length = new_length;
    return 1;
}

static int append_char(char **buffer, int *length, int *offset, char c) {
    if (!ensure_capacity(buffer, length, *offset + 1)) {
        return 0;
    }
    (*buffer)[(*offset)++] = c;
    return 1;
}

static int append_string(char **buffer, int *length, int *offset, const char *str) {
    size_t len = strlen(str);
    if (!ensure_capacity(buffer, length, *offset + (int)len)) {
        return 0;
    }
    memcpy(*buffer + *offset, str, len);
    *offset += (int)len;
    return 1;
}

static int print_quoted_string(const char *input, char **buffer, int *length, int *offset) {
    if (!append_char(buffer, length, offset, '"')) {
        return 0;
    }
    while (*input) {
        unsigned char c = (unsigned char)*input++;
        switch (c) {
            case '"':
                if (!append_string(buffer, length, offset, "\\""")) return 0;
                break;
            case '\\':
                if (!append_string(buffer, length, offset, "\\\\")) return 0;
                break;
            case '\b':
                if (!append_string(buffer, length, offset, "\\b")) return 0;
                break;
            case '\f':
                if (!append_string(buffer, length, offset, "\\f")) return 0;
                break;
            case '\n':
                if (!append_string(buffer, length, offset, "\\n")) return 0;
                break;
            case '\r':
                if (!append_string(buffer, length, offset, "\\r")) return 0;
                break;
            case '\t':
                if (!append_string(buffer, length, offset, "\\t")) return 0;
                break;
            default:
                if (c < 0x20) {
                    char escaped[7];
                    snprintf(escaped, sizeof(escaped), "\\u%04x", c);
                    if (!append_string(buffer, length, offset, escaped)) {
                        return 0;
                    }
                } else {
                    if (!append_char(buffer, length, offset, (char)c)) return 0;
                }
                break;
        }
    }
    return append_char(buffer, length, offset, '"');
}

static int print_value_internal(const cJSON *item, int depth, int fmt, char **buffer, int *length, int *offset);

static int print_array_internal(const cJSON *item, int depth, int fmt, char **buffer, int *length, int *offset) {
    if (!append_char(buffer, length, offset, '[')) return 0;
    if (item->child) {
        if (fmt && !append_char(buffer, length, offset, '\n')) return 0;
        cJSON *child = item->child;
        while (child) {
            if (fmt) {
                for (int i = 0; i < depth + 1; ++i) {
                    if (!append_char(buffer, length, offset, '\t')) return 0;
                }
            }
            if (!print_value_internal(child, depth + 1, fmt, buffer, length, offset)) return 0;
            if (child->next) {
                if (!append_char(buffer, length, offset, ',')) return 0;
                if (fmt && !append_char(buffer, length, offset, '\n')) return 0;
            }
            child = child->next;
        }
        if (fmt) {
            if (!append_char(buffer, length, offset, '\n')) return 0;
            for (int i = 0; i < depth; ++i) {
                if (!append_char(buffer, length, offset, '\t')) return 0;
            }
        }
    }
    return append_char(buffer, length, offset, ']');
}

static int print_object_internal(const cJSON *item, int depth, int fmt, char **buffer, int *length, int *offset) {
    if (!append_char(buffer, length, offset, '{')) return 0;
    if (item->child) {
        if (fmt && !append_char(buffer, length, offset, '\n')) return 0;
        cJSON *child = item->child;
        while (child) {
            if (fmt) {
                for (int i = 0; i < depth + 1; ++i) {
                    if (!append_char(buffer, length, offset, '\t')) return 0;
                }
            }
            if (!print_quoted_string(child->string ? child->string : "", buffer, length, offset)) return 0;
            if (!append_char(buffer, length, offset, ':')) return 0;
            if (fmt && !append_char(buffer, length, offset, ' ')) return 0;
            if (!print_value_internal(child, depth + 1, fmt, buffer, length, offset)) return 0;
            if (child->next) {
                if (!append_char(buffer, length, offset, ',')) return 0;
                if (fmt && !append_char(buffer, length, offset, '\n')) return 0;
            }
            child = child->next;
        }
        if (fmt) {
            if (!append_char(buffer, length, offset, '\n')) return 0;
            for (int i = 0; i < depth; ++i) {
                if (!append_char(buffer, length, offset, '\t')) return 0;
            }
        }
    }
    return append_char(buffer, length, offset, '}');
}

static int print_value_internal(const cJSON *item, int depth, int fmt, char **buffer, int *length, int *offset) {
    switch (item->type & 0xFF) {
        case cJSON_NULL:
            return append_string(buffer, length, offset, "null");
        case cJSON_False:
            return append_string(buffer, length, offset, "false");
        case cJSON_True:
            return append_string(buffer, length, offset, "true");
        case cJSON_Number: {
            char num[64];
            if (fabs((double)item->valueint - item->valuedouble) <= DBL_EPSILON) {
                snprintf(num, sizeof(num), "%d", item->valueint);
            } else {
                snprintf(num, sizeof(num), "%.17g", item->valuedouble);
            }
            return append_string(buffer, length, offset, num);
        }
        case cJSON_String:
            return print_quoted_string(item->valuestring ? item->valuestring : "", buffer, length, offset);
        case cJSON_Array:
            return print_array_internal(item, depth, fmt, buffer, length, offset);
        case cJSON_Object:
            return print_object_internal(item, depth, fmt, buffer, length, offset);
        case cJSON_Raw:
            return append_string(buffer, length, offset, item->valuestring ? item->valuestring : "");
        default:
            return 0;
    }
}

static char *print_internal(const cJSON *item, int fmt) {
    char *buffer = NULL;
    int length = 0;
    int offset = 0;
    if (!print_value_internal(item, 0, fmt, &buffer, &length, &offset)) {
        if (buffer) cjson_free(buffer);
        return NULL;
    }
    if (!ensure_capacity(&buffer, &length, offset + 1)) {
        if (buffer) cjson_free(buffer);
        return NULL;
    }
    buffer[offset] = '\0';
    return buffer;
}

char *cJSON_Print(const cJSON *item) {
    return print_internal(item, 1);
}

char *cJSON_PrintUnformatted(const cJSON *item) {
    return print_internal(item, 0);
}

char *cJSON_PrintBuffered(const cJSON *item, int prebuffer, int fmt) {
    (void)prebuffer;
    return print_internal(item, fmt);
}

int cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const int format) {
    char *printed = print_internal(item, format);
    if (!printed) {
        return 0;
    }
    int needed = (int)strlen(printed);
    if (needed >= length) {
        cjson_free(printed);
        return 0;
    }
    memcpy(buffer, printed, (size_t)needed + 1);
    cjson_free(printed);
    return 1;
}

/* getters */
int cJSON_GetArraySize(const cJSON *array) {
    int count = 0;
    cJSON *child = array ? array->child : NULL;
    while (child) {
        count++;
        child = child->next;
    }
    return count;
}

cJSON *cJSON_GetArrayItem(const cJSON *array, int index) {
    cJSON *child = array ? array->child : NULL;
    while (child && index > 0) {
        child = child->next;
        index--;
    }
    return child;
}

static int case_insensitive_compare(const char *a, const char *b) {
    if (!a || !b) {
        return (a == b) ? 0 : 1;
    }
    while (*a && *b) {
        char ca = (char)tolower((unsigned char)*a++);
        char cb = (char)tolower((unsigned char)*b++);
        if (ca != cb) {
            return ca - cb;
        }
    }
    return (unsigned char)*a - (unsigned char)*b;
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string) {
    cJSON *child = object ? object->child : NULL;
    while (child) {
        if (child->string && strcmp(child->string, string) == 0) {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string) {
    cJSON *child = object ? object->child : NULL;
    while (child) {
        if (child->string && case_insensitive_compare(child->string, string) == 0) {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

int cJSON_HasObjectItem(const cJSON *object, const char *string) {
    return cJSON_GetObjectItem(object, string) != NULL;
}

int cJSON_IsInvalid(const cJSON *item) { return (item == NULL) || ((item->type & 0xFF) == cJSON_Invalid); }
int cJSON_IsFalse(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_False); }
int cJSON_IsTrue(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_True); }
int cJSON_IsBool(const cJSON *item) { return cJSON_IsTrue(item) || cJSON_IsFalse(item); }
int cJSON_IsNull(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_NULL); }
int cJSON_IsNumber(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_Number); }
int cJSON_IsString(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_String); }
int cJSON_IsArray(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_Array); }
int cJSON_IsObject(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_Object); }
int cJSON_IsRaw(const cJSON *item) { return item && ((item->type & 0xFF) == cJSON_Raw); }

/* creation helpers */
cJSON *cJSON_CreateNull(void) { cJSON *item = cjson_new_item(); if (item) item->type = cJSON_NULL; return item; }
cJSON *cJSON_CreateTrue(void) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_True; item->valueint = 1; } return item; }
cJSON *cJSON_CreateFalse(void) { cJSON *item = cjson_new_item(); if (item) item->type = cJSON_False; return item; }
cJSON *cJSON_CreateBool(int boolean) { return boolean ? cJSON_CreateTrue() : cJSON_CreateFalse(); }
cJSON *cJSON_CreateNumber(double number) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_Number; item->valuedouble = number; item->valueint = (int)number; } return item; }
cJSON *cJSON_CreateString(const char *string) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_String; item->valuestring = cjson_strdup(string ? string : ""); if (!item->valuestring) { cjson_delete_item(item); return NULL; } } return item; }
cJSON *cJSON_CreateRaw(const char *string) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_Raw; item->valuestring = cjson_strdup(string ? string : ""); if (!item->valuestring) { cjson_delete_item(item); return NULL; } } return item; }
cJSON *cJSON_CreateArray(void) { cJSON *item = cjson_new_item(); if (item) item->type = cJSON_Array; return item; }
cJSON *cJSON_CreateObject(void) { cJSON *item = cjson_new_item(); if (item) item->type = cJSON_Object; return item; }

cJSON *cJSON_CreateStringReference(const char *string) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_String | cJSON_IsReference; item->valuestring = (char *)string; } return item; }
cJSON *cJSON_CreateObjectReference(const cJSON *child) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_Object | cJSON_IsReference; item->child = (cJSON *)child; } return item; }
cJSON *cJSON_CreateArrayReference(const cJSON *child) { cJSON *item = cjson_new_item(); if (item) { item->type = cJSON_Array | cJSON_IsReference; item->child = (cJSON *)child; } return item; }

void cJSON_AddItemToArray(cJSON *array, cJSON *item) {
    if (!array || !item) return;
    if (!array->child) {
        array->child = item;
    } else {
        cJSON *child = array->child;
        while (child->next) child = child->next;
        child->next = item;
        item->prev = child;
    }
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) {
    if (!object || !string || !item) return;
    item->string = cjson_strdup(string);
    if (!item->string) {
        cjson_delete_item(item);
        return;
    }
    cJSON_AddItemToArray(object, item);
}

void cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item) {
    if (!object || !string || !item) return;
    item->string = (char *)string;
    item->type |= cJSON_StringIsConst;
    cJSON_AddItemToArray(object, item);
}

void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item) {
    cJSON_AddItemToArray(array, cJSON_CreateObjectReference(item));
}

void cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item) {
    cJSON_AddItemToObject(object, string, cJSON_CreateObjectReference(item));
}

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which) {
    if (!array || which < 0) return NULL;
    cJSON *child = array->child;
    while (child && which > 0) {
        child = child->next;
        which--;
    }
    if (!child) return NULL;

    if (child->prev) child->prev->next = child->next;
    if (child->next) child->next->prev = child->prev;
    if (array->child == child) array->child = child->next;
    child->next = child->prev = NULL;
    return child;
}

void cJSON_DeleteItemFromArray(cJSON *array, int which) {
    cJSON *item = cJSON_DetachItemFromArray(array, which);
    cjson_delete_item(item);
}

cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string) {
    if (!object || !string) return NULL;
    cJSON *child = object->child;
    while (child) {
        if (child->string && strcmp(child->string, string) == 0) {
            if (child->prev) child->prev->next = child->next;
            if (child->next) child->next->prev = child->prev;
            if (object->child == child) object->child = child->next;
            child->next = child->prev = NULL;
            return child;
        }
        child = child->next;
    }
    return NULL;
}

void cJSON_DeleteItemFromObject(cJSON *object, const char *string) {
    cJSON *item = cJSON_DetachItemFromObject(object, string);
    cjson_delete_item(item);
}

void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem) {
    if (!array || which < 0 || !newitem) return;
    if (which == 0) {
        newitem->next = array->child;
        if (array->child) array->child->prev = newitem;
        array->child = newitem;
        return;
    }
    cJSON *child = array->child;
    while (child && which > 1) {
        child = child->next;
        which--;
    }
    if (!child) {
        cJSON_AddItemToArray(array, newitem);
        return;
    }
    newitem->next = child->next;
    newitem->prev = child;
    if (child->next) child->next->prev = newitem;
    child->next = newitem;
}

void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem) {
    if (!array || which < 0 || !newitem) return;
    cJSON *old = cJSON_GetArrayItem(array, which);
    if (!old) return;
    newitem->next = old->next;
    newitem->prev = old->prev;
    if (newitem->next) newitem->next->prev = newitem;
    if (newitem->prev) newitem->prev->next = newitem;
    if (array->child == old) array->child = newitem;
    old->next = old->prev = NULL;
    cjson_delete_item(old);
}

void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem) {
    if (!object || !string || !newitem) return;
    cJSON *child = object->child;
    while (child) {
        if (child->string && strcmp(child->string, string) == 0) {
            newitem->string = cjson_strdup(string);
            if (!newitem->string) {
                cjson_delete_item(newitem);
                return;
            }
            newitem->next = child->next;
            newitem->prev = child->prev;
            if (newitem->next) newitem->next->prev = newitem;
            if (newitem->prev) newitem->prev->next = newitem;
            if (object->child == child) object->child = newitem;
            child->next = child->prev = NULL;
            cjson_delete_item(child);
            return;
        }
        child = child->next;
    }
}

cJSON *cJSON_Duplicate(const cJSON *item, int recurse) {
    if (!item) return NULL;
    cJSON *copy = cjson_new_item();
    if (!copy) return NULL;
    copy->type = item->type;
    copy->valueint = item->valueint;
    copy->valuedouble = item->valuedouble;
    if (item->valuestring) {
        copy->valuestring = cjson_strdup(item->valuestring);
        if (!copy->valuestring) {
            cjson_delete_item(copy);
            return NULL;
        }
    }
    if (item->string && !(item->type & cJSON_StringIsConst)) {
        copy->string = cjson_strdup(item->string);
        if (!copy->string) {
            cjson_delete_item(copy);
            return NULL;
        }
    } else {
        copy->string = item->string;
        copy->type |= (item->type & cJSON_StringIsConst);
    }

    if (!recurse || !item->child) {
        return copy;
    }

    cJSON *child = item->child;
    cJSON *new_child = cJSON_Duplicate(child, 1);
    if (!new_child) {
        cjson_delete_item(copy);
        return NULL;
    }
    copy->child = new_child;
    while (child->next) {
        child = child->next;
        cJSON *next = cJSON_Duplicate(child, 1);
        if (!next) {
            cjson_delete_item(copy);
            return NULL;
        }
        new_child->next = next;
        next->prev = new_child;
        new_child = next;
    }
    return copy;
}

char *cJSON_GetErrorPtr(void) {
    return (char *)global_error_pointer;
}

/* array helper creation */
cJSON *cJSON_CreateIntArray(const int *numbers, int count) {
    if (!numbers || count <= 0) return cJSON_CreateArray();
    cJSON *array = cJSON_CreateArray();
    if (!array) return NULL;
    for (int i = 0; i < count; ++i) {
        cJSON *num = cJSON_CreateNumber(numbers[i]);
        if (!num) {
            cJSON_Delete(array);
            return NULL;
        }
        cJSON_AddItemToArray(array, num);
    }
    return array;
}

cJSON *cJSON_CreateFloatArray(const float *numbers, int count) {
    if (!numbers || count <= 0) return cJSON_CreateArray();
    cJSON *array = cJSON_CreateArray();
    if (!array) return NULL;
    for (int i = 0; i < count; ++i) {
        cJSON *num = cJSON_CreateNumber(numbers[i]);
        if (!num) {
            cJSON_Delete(array);
            return NULL;
        }
        cJSON_AddItemToArray(array, num);
    }
    return array;
}

cJSON *cJSON_CreateDoubleArray(const double *numbers, int count) {
    if (!numbers || count <= 0) return cJSON_CreateArray();
    cJSON *array = cJSON_CreateArray();
    if (!array) return NULL;
    for (int i = 0; i < count; ++i) {
        cJSON *num = cJSON_CreateNumber(numbers[i]);
        if (!num) {
            cJSON_Delete(array);
            return NULL;
        }
        cJSON_AddItemToArray(array, num);
    }
    return array;
}

cJSON *cJSON_CreateStringArray(const char *const *strings, int count) {
    if (!strings || count <= 0) return cJSON_CreateArray();
    cJSON *array = cJSON_CreateArray();
    if (!array) return NULL;
    for (int i = 0; i < count; ++i) {
        cJSON *str = cJSON_CreateString(strings[i]);
        if (!str) {
            cJSON_Delete(array);
            return NULL;
        }
        cJSON_AddItemToArray(array, str);
    }
    return array;
}

void cJSON_Minify(char *json) {
    char *ptr = json;
    char *out = json;
    int in_string = 0;
    while (ptr && *ptr) {
        if (*ptr == '"' && (ptr == json || ptr[-1] != '\\')) {
            in_string = !in_string;
        }
        if (!in_string && isspace((unsigned char)*ptr)) {
            ptr++;
            continue;
        }
        if (!in_string && *ptr == '/' && ptr[1] == '/') {
            ptr += 2;
            while (*ptr && *ptr != '\n') ptr++;
            continue;
        }
        if (!in_string && *ptr == '/' && ptr[1] == '*') {
            ptr += 2;
            while (*ptr && !(*ptr == '*' && ptr[1] == '/')) ptr++;
            if (*ptr) ptr += 2;
            continue;
        }
        *out++ = *ptr++;
    }
    if (out) *out = '\0';
}

int cJSON_Compare(const cJSON *a, const cJSON *b, int case_sensitive) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    if ((a->type & 0xFF) != (b->type & 0xFF)) return 0;

    switch (a->type & 0xFF) {
        case cJSON_NULL:
        case cJSON_False:
        case cJSON_True:
            return 1;
        case cJSON_Number:
            return fabs(a->valuedouble - b->valuedouble) <= DBL_EPSILON;
        case cJSON_String:
        case cJSON_Raw:
            if (a->valuestring == NULL || b->valuestring == NULL) {
                return a->valuestring == b->valuestring;
            }
            return case_sensitive ? strcmp(a->valuestring, b->valuestring) == 0
                                  : case_insensitive_compare(a->valuestring, b->valuestring) == 0;
        case cJSON_Array: {
            const cJSON *a_child = a->child;
            const cJSON *b_child = b->child;
            while (a_child && b_child) {
                if (!cJSON_Compare(a_child, b_child, case_sensitive)) return 0;
                a_child = a_child->next;
                b_child = b_child->next;
            }
            return a_child == b_child;
        }
        case cJSON_Object: {
            const cJSON *a_child = a->child;
            while (a_child) {
                const cJSON *b_item = case_sensitive ? cJSON_GetObjectItem(b, a_child->string)
                                                     : cJSON_GetObjectItemCaseSensitive(b, a_child->string);
                if (!cJSON_Compare(a_child, b_item, case_sensitive)) return 0;
                a_child = a_child->next;
            }
            const cJSON *b_child = b->child;
            while (b_child) {
                const cJSON *a_item = case_sensitive ? cJSON_GetObjectItem(a, b_child->string)
                                                     : cJSON_GetObjectItemCaseSensitive(a, b_child->string);
                if (!cJSON_Compare(b_child, a_item, case_sensitive)) return 0;
                b_child = b_child->next;
            }
            return 1;
        }
        default:
            return 0;
    }
}
cJSON *cJSON_AddNullToObject(cJSON *object, const char *name) {
    cJSON *item = cJSON_CreateNull();
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON *cJSON_AddTrueToObject(cJSON *object, const char *name) {
    cJSON *item = cJSON_CreateTrue();
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON *cJSON_AddFalseToObject(cJSON *object, const char *name) {
    cJSON *item = cJSON_CreateFalse();
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON *cJSON_AddBoolToObject(cJSON *object, const char *name, int boolean) {
    cJSON *item = cJSON_CreateBool(boolean);
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON *cJSON_AddNumberToObject(cJSON *object, const char *name, double number) {
    cJSON *item = cJSON_CreateNumber(number);
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON *cJSON_AddStringToObject(cJSON *object, const char *name, const char *string) {
    cJSON *item = cJSON_CreateString(string);
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON *cJSON_AddRawToObject(cJSON *object, const char *name, const char *raw) {
    cJSON *item = cJSON_CreateRaw(raw);
    if (!item) return NULL;
    cJSON_AddItemToObject(object, name, item);
    return item;
}
