/**
 * @file unity.h
 * @brief Unity Testing Framework Header
 * 
 * Lightweight C unit testing framework for NeoC SDK
 */

#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Unity Configuration */
#ifndef UNITY_COUNTER_TYPE
#define UNITY_COUNTER_TYPE unsigned int
#endif

#ifndef UNITY_POINTER_WIDTH
#define UNITY_POINTER_WIDTH 64
#endif

/* Test Status */
typedef enum {
    UNITY_DISPLAY_STYLE_INT,
    UNITY_DISPLAY_STYLE_HEX8,
    UNITY_DISPLAY_STYLE_HEX16,
    UNITY_DISPLAY_STYLE_HEX32,
    UNITY_DISPLAY_STYLE_UINT,
    UNITY_DISPLAY_STYLE_CHAR
} UNITY_DISPLAY_STYLE_T;

typedef enum {
    UNITY_WITHIN,
    UNITY_EQUAL_TO,
    UNITY_GREATER_THAN,
    UNITY_GREATER_THAN_OR_EQUAL,
    UNITY_SMALLER_THAN,
    UNITY_SMALLER_THAN_OR_EQUAL,
    UNITY_UNKNOWN
} UNITY_COMPARISON_T;

typedef enum {
    UNITY_DISPLAY_RANGE_INT,
    UNITY_DISPLAY_RANGE_UINT,
    UNITY_DISPLAY_RANGE_HEX
} UNITY_DISPLAY_RANGE_T;

typedef enum {
    UNITY_FLOAT_IS_NOT_INF,
    UNITY_FLOAT_IS_INF,
    UNITY_FLOAT_IS_NOT_NEG_INF,
    UNITY_FLOAT_IS_NEG_INF,
    UNITY_FLOAT_IS_NOT_NAN,
    UNITY_FLOAT_IS_NAN,
    UNITY_FLOAT_IS_NOT_DET,
    UNITY_FLOAT_IS_DET,
    UNITY_FLOAT_INVALID_TRAIT
} UNITY_FLOAT_TRAIT_T;

/* Unity Test Results Structure */
struct UNITY_STORAGE_T {
    const char* TestFile;
    const char* CurrentTestName;
    UNITY_COUNTER_TYPE CurrentTestLineNumber;
    UNITY_COUNTER_TYPE NumberOfTests;
    UNITY_COUNTER_TYPE TestFailures;
    UNITY_COUNTER_TYPE TestIgnores;
    UNITY_COUNTER_TYPE CurrentTestFailed;
    UNITY_COUNTER_TYPE CurrentTestIgnored;
    jmp_buf AbortFrame;
};

extern struct UNITY_STORAGE_T Unity;

/* Unity Public Interface */
void UnityBegin(const char* filename);
int UnityEnd(void);
void UnityConcludeTest(void);
void UnityDefaultTestRun(void (*Func)(void), const char* FuncName, const int FuncLineNum);

/* Test Setup and Teardown */
void setUp(void);
void tearDown(void);

/* Assert Functions */
void UnityAssertEqualNumber(const long expected, const long actual,
                           const char* msg, const UNITY_COUNTER_TYPE lineNumber,
                           const UNITY_DISPLAY_STYLE_T style);

void UnityAssertEqualIntArray(const int* expected, const int* actual,
                             const UNITY_COUNTER_TYPE num_elements,
                             const char* msg, const UNITY_COUNTER_TYPE lineNumber,
                             const UNITY_DISPLAY_STYLE_T style);

void UnityAssertBits(const long mask, const long expected, const long actual,
                     const char* msg, const UNITY_COUNTER_TYPE lineNumber);

void UnityAssertEqualString(const char* expected, const char* actual,
                           const char* msg, const UNITY_COUNTER_TYPE lineNumber);

void UnityAssertEqualStringLen(const char* expected, const char* actual,
                              const UNITY_COUNTER_TYPE length,
                              const char* msg, const UNITY_COUNTER_TYPE lineNumber);

void UnityAssertEqualStringArray(const char** expected, const char** actual,
                                const UNITY_COUNTER_TYPE num_elements,
                                const char* msg, const UNITY_COUNTER_TYPE lineNumber);

void UnityAssertEqualMemory(const void* expected, const void* actual,
                           const UNITY_COUNTER_TYPE length,
                           const UNITY_COUNTER_TYPE num_elements,
                           const char* msg, const UNITY_COUNTER_TYPE lineNumber);

void UnityAssertPointersEqual(const void* expected, const void* actual,
                             const char* msg, const UNITY_COUNTER_TYPE lineNumber);

void UnityAssertNumbersWithin(const long delta, const long expected, const long actual,
                             const char* msg, const UNITY_COUNTER_TYPE lineNumber,
                             const UNITY_DISPLAY_STYLE_T style);

void UnityFail(const char* msg, const UNITY_COUNTER_TYPE lineNumber);
void UnityIgnore(const char* msg, const UNITY_COUNTER_TYPE lineNumber);

/* Test Macros */
#define TEST_FAIL_MESSAGE(message)                          UnityFail((message), __LINE__)
#define TEST_FAIL()                                         UnityFail(NULL, __LINE__)
#define TEST_IGNORE_MESSAGE(message)                        UnityIgnore((message), __LINE__)
#define TEST_IGNORE()                                       UnityIgnore(NULL, __LINE__)

#define TEST_ASSERT_TRUE(condition)                         UNITY_TEST_ASSERT(((condition) ? 1 : 0), 1, __LINE__, " Expression Evaluated To FALSE")
#define TEST_ASSERT_FALSE(condition)                        UNITY_TEST_ASSERT(((condition) ? 1 : 0), 0, __LINE__, " Expression Evaluated To TRUE")
#define TEST_ASSERT_UNLESS(condition)                       TEST_ASSERT_FALSE(condition)
#define TEST_ASSERT(condition)                              TEST_ASSERT_TRUE(condition)

#define TEST_ASSERT_NULL(pointer)                           UNITY_TEST_ASSERT_NULL((pointer), __LINE__, " Expected NULL")
#define TEST_ASSERT_NOT_NULL(pointer)                       UNITY_TEST_ASSERT_NOT_NULL((pointer), __LINE__, " Expected Non-NULL")

#define TEST_ASSERT_EQUAL_INT(expected, actual)             UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT8(expected, actual)            UNITY_TEST_ASSERT_EQUAL_INT8((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT16(expected, actual)           UNITY_TEST_ASSERT_EQUAL_INT16((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT32(expected, actual)           UNITY_TEST_ASSERT_EQUAL_INT32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT64(expected, actual)           UNITY_TEST_ASSERT_EQUAL_INT64((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_UINT(expected, actual)            UNITY_TEST_ASSERT_EQUAL_UINT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT8(expected, actual)           UNITY_TEST_ASSERT_EQUAL_UINT8((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT16(expected, actual)          UNITY_TEST_ASSERT_EQUAL_UINT16((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT32(expected, actual)          UNITY_TEST_ASSERT_EQUAL_UINT32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT64(expected, actual)          UNITY_TEST_ASSERT_EQUAL_UINT64((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_HEX(expected, actual)             UNITY_TEST_ASSERT_EQUAL_HEX32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX8(expected, actual)            UNITY_TEST_ASSERT_EQUAL_HEX8((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX16(expected, actual)           UNITY_TEST_ASSERT_EQUAL_HEX16((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX32(expected, actual)           UNITY_TEST_ASSERT_EQUAL_HEX32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX64(expected, actual)           UNITY_TEST_ASSERT_EQUAL_HEX64((expected), (actual), __LINE__, NULL)

#define TEST_ASSERT_EQUAL_PTR(expected, actual)             UNITY_TEST_ASSERT_EQUAL_PTR((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_STRING(expected, actual)          UNITY_TEST_ASSERT_EQUAL_STRING((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len) UNITY_TEST_ASSERT_EQUAL_STRING_LEN((expected), (actual), (len), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, len)     UNITY_TEST_ASSERT_EQUAL_MEMORY((expected), (actual), (len), __LINE__, NULL)

/* Internal macros - do not use directly */
#define UNITY_TEST_ASSERT(condition, expected, line, message) \
    if (condition != expected) UnityFail((message), (UNITY_COUNTER_TYPE)(line))

#define UNITY_TEST_ASSERT_NULL(pointer, line, message) \
    UNITY_TEST_ASSERT(((pointer) == NULL), 1, (line), (message))

#define UNITY_TEST_ASSERT_NOT_NULL(pointer, line, message) \
    UNITY_TEST_ASSERT(((pointer) != NULL), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((expected) == (actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_INT8(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((int8_t)(expected) == (int8_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_INT16(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((int16_t)(expected) == (int16_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_INT32(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((int32_t)(expected) == (int32_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_INT64(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((int64_t)(expected) == (int64_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_UINT(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((unsigned int)(expected) == (unsigned int)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_UINT8(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint8_t)(expected) == (uint8_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_UINT16(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint16_t)(expected) == (uint16_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_UINT32(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint32_t)(expected) == (uint32_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_UINT64(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint64_t)(expected) == (uint64_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_HEX8(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint8_t)(expected) == (uint8_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_HEX16(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint16_t)(expected) == (uint16_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_HEX32(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint32_t)(expected) == (uint32_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_HEX64(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((uint64_t)(expected) == (uint64_t)(actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_PTR(expected, actual, line, message) \
    UNITY_TEST_ASSERT(((expected) == (actual)), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_STRING(expected, actual, line, message) \
    UNITY_TEST_ASSERT((strcmp((expected), (actual)) == 0), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len, line, message) \
    UNITY_TEST_ASSERT((strncmp((expected), (actual), (len)) == 0), 1, (line), (message))

#define UNITY_TEST_ASSERT_EQUAL_MEMORY(expected, actual, len, line, message) \
    UNITY_TEST_ASSERT((memcmp((expected), (actual), (len)) == 0), 1, (line), (message))

/* Test Runner Macros */
#define RUN_TEST(TestFunc) UnityDefaultTestRun(TestFunc, #TestFunc, __LINE__)

/* Test Suite Management */
#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() return UnityEnd()

#ifdef __cplusplus
}
#endif

#endif /* UNITY_H */
