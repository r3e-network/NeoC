#ifndef NEOC_STRING_H
#define NEOC_STRING_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// String builder structure for efficient string concatenation
typedef struct neoc_string_builder {
    char* buffer;
    size_t length;
    size_t capacity;
} neoc_string_builder_t;

// String utilities
char* neoc_string_trim(const char* str);
char* neoc_string_ltrim(const char* str);
char* neoc_string_rtrim(const char* str);

// String case conversion
char* neoc_string_to_upper(const char* str);
char* neoc_string_to_lower(const char* str);

// String comparison
bool neoc_string_equals_ignore_case(const char* a, const char* b);
bool neoc_string_starts_with(const char* str, const char* prefix);
bool neoc_string_ends_with(const char* str, const char* suffix);
bool neoc_string_contains(const char* str, const char* substr);

// String manipulation
char* neoc_string_replace(const char* str, const char* old_substr, const char* new_substr);
char* neoc_string_replace_all(const char* str, const char* old_substr, const char* new_substr);
char** neoc_string_split(const char* str, const char* delimiter, size_t* count);
char* neoc_string_join(const char** strings, size_t count, const char* separator);
char* neoc_string_substring(const char* str, size_t start, size_t length);
char* neoc_string_concat(const char* str1, const char* str2);
char* neoc_string_concat_n(const char** strings, size_t count);

// String padding
char* neoc_string_pad_left(const char* str, size_t total_width, char pad_char);
char* neoc_string_pad_right(const char* str, size_t total_width, char pad_char);
char* neoc_string_pad_center(const char* str, size_t total_width, char pad_char);

// String formatting
char* neoc_string_format(const char* format, ...);
char* neoc_string_format_bytes(const uint8_t* bytes, size_t length, const char* separator);

// String validation
bool neoc_string_is_empty(const char* str);
bool neoc_string_is_whitespace(const char* str);
bool neoc_string_is_numeric(const char* str);
bool neoc_string_is_hex(const char* str);
bool neoc_string_is_base64(const char* str);

// String builder functions
neoc_string_builder_t* neoc_string_builder_create(void);
neoc_string_builder_t* neoc_string_builder_create_with_capacity(size_t capacity);
void neoc_string_builder_free(neoc_string_builder_t* builder);
bool neoc_string_builder_append(neoc_string_builder_t* builder, const char* str);
bool neoc_string_builder_append_char(neoc_string_builder_t* builder, char c);
bool neoc_string_builder_append_format(neoc_string_builder_t* builder, const char* format, ...);
bool neoc_string_builder_append_bytes(neoc_string_builder_t* builder, const uint8_t* bytes, size_t length);
bool neoc_string_builder_insert(neoc_string_builder_t* builder, size_t index, const char* str);
bool neoc_string_builder_delete(neoc_string_builder_t* builder, size_t start, size_t length);
void neoc_string_builder_clear(neoc_string_builder_t* builder);
char* neoc_string_builder_to_string(const neoc_string_builder_t* builder);
size_t neoc_string_builder_length(const neoc_string_builder_t* builder);

// String conversion
int64_t neoc_string_to_int64(const char* str, bool* success);
uint64_t neoc_string_to_uint64(const char* str, bool* success);
double neoc_string_to_double(const char* str, bool* success);
char* neoc_int64_to_string(int64_t value);
char* neoc_uint64_to_string(uint64_t value);
char* neoc_double_to_string(double value, int precision);

// Free string array
void neoc_string_array_free(char** array, size_t count);

// URL encoding/decoding
char* neoc_string_url_encode(const char* str);
char* neoc_string_url_decode(const char* str);

// HTML escaping
char* neoc_string_html_escape(const char* str);
char* neoc_string_html_unescape(const char* str);

#ifdef __cplusplus
}
#endif

#endif // NEOC_STRING_H