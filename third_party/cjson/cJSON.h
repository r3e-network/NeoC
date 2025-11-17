/*
  cJSON
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

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 15

#define cJSON_Invalid (0)
#define cJSON_False   (1 << 0)
#define cJSON_True    (1 << 1)
#define cJSON_NULL    (1 << 2)
#define cJSON_Number  (1 << 3)
#define cJSON_String  (1 << 4)
#define cJSON_Array   (1 << 5)
#define cJSON_Object  (1 << 6)
#define cJSON_Raw     (1 << 7)

#define cJSON_IsReference     256
#define cJSON_StringIsConst   512

typedef struct cJSON {
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;

    int type;

    char *valuestring;
    int valueint;
    double valuedouble;

    char *string;
} cJSON;

typedef struct cJSON_Hooks {
    void *(*malloc_fn)(size_t size);
    void (*free_fn)(void *pointer);
} cJSON_Hooks;

void cJSON_InitHooks(cJSON_Hooks *hooks);
void *cJSON_malloc(size_t size);
void cJSON_free(void *pointer);

/* Supply a block of JSON, and this returns a cJSON object you can interrogate. */
cJSON *cJSON_Parse(const char *value);
cJSON *cJSON_ParseWithLength(const char *value, size_t buffer_length);
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/* Render a cJSON entity to text for transfer/storage. */
char *cJSON_Print(const cJSON *item);
char *cJSON_PrintUnformatted(const cJSON *item);
char *cJSON_PrintBuffered(const cJSON *item, int prebuffer, int fmt);
int cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const int format);

/* Delete a cJSON entity and all subentities. */
void cJSON_Delete(cJSON *item);

/* Returns the number of items in an array (or object). */
int cJSON_GetArraySize(const cJSON *array);

/* Retrieve item number "item" from array or object. */
cJSON *cJSON_GetArrayItem(const cJSON *array, int index);
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string);
cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string);
int cJSON_HasObjectItem(const cJSON *object, const char *string);

/* Helper for iterating over an array/object */
#define cJSON_ArrayForEach(element, array) for(element = (array) ? (array)->child : NULL; element != NULL; element = element->next)

/* These helpers check the type of an item */
int cJSON_IsInvalid(const cJSON *item);
int cJSON_IsFalse(const cJSON *item);
int cJSON_IsTrue(const cJSON *item);
int cJSON_IsBool(const cJSON *item);
int cJSON_IsNull(const cJSON *item);
int cJSON_IsNumber(const cJSON *item);
int cJSON_IsString(const cJSON *item);
int cJSON_IsArray(const cJSON *item);
int cJSON_IsObject(const cJSON *item);
int cJSON_IsRaw(const cJSON *item);

/* Creates a cJSON item of the appropriate type. */
cJSON *cJSON_CreateNull(void);
cJSON *cJSON_CreateTrue(void);
cJSON *cJSON_CreateFalse(void);
cJSON *cJSON_CreateBool(int boolean);
cJSON *cJSON_CreateNumber(double number);
cJSON *cJSON_CreateString(const char *string);
cJSON *cJSON_CreateRaw(const char *raw);
cJSON *cJSON_CreateArray(void);
cJSON *cJSON_CreateObject(void);

cJSON *cJSON_CreateStringReference(const char *string);
cJSON *cJSON_CreateObjectReference(const cJSON *child);
cJSON *cJSON_CreateArrayReference(const cJSON *child);

/* Appending items to arrays/objects */
void cJSON_AddItemToArray(cJSON *array, cJSON *item);
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
void cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item);
void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
void cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item);

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which);
void cJSON_DeleteItemFromArray(cJSON *array, int which);
cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string);
void cJSON_DeleteItemFromObject(cJSON *object, const char *string);

void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem);
void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem);
void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem);

cJSON *cJSON_Duplicate(const cJSON *item, int recurse);

char *cJSON_GetErrorPtr(void);

/* Macros for creating things quickly. */
cJSON *cJSON_AddNullToObject(cJSON *object, const char *name);
cJSON *cJSON_AddTrueToObject(cJSON *object, const char *name);
cJSON *cJSON_AddFalseToObject(cJSON *object, const char *name);
cJSON *cJSON_AddBoolToObject(cJSON *object, const char *name, int boolean);
cJSON *cJSON_AddNumberToObject(cJSON *object, const char *name, double number);
cJSON *cJSON_AddStringToObject(cJSON *object, const char *name, const char *string);
cJSON *cJSON_AddRawToObject(cJSON *object, const char *name, const char *raw);

void cJSON_Minify(char *json);

/* Comparison */
int cJSON_Compare(const cJSON *a, const cJSON *b, int case_sensitive);

/* Array helpers */
cJSON *cJSON_CreateIntArray(const int *numbers, int count);
cJSON *cJSON_CreateFloatArray(const float *numbers, int count);
cJSON *cJSON_CreateDoubleArray(const double *numbers, int count);
cJSON *cJSON_CreateStringArray(const char *const *strings, int count);

#ifdef __cplusplus
}
#endif

#endif /* cJSON__h */
