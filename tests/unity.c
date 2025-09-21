/**
 * @file unity.c
 * @brief Unity Testing Framework Implementation
 */

#include "unity.h"
#include <stdlib.h>
#include <stdarg.h>

/* Unity Storage */
struct UNITY_STORAGE_T Unity;

/* Function Prototypes */
static void UnityPrint(const char* string);
static void UnityPrintLen(const char* string, const UNITY_COUNTER_TYPE length);
static void UnityPrintNumberByStyle(const long number, const UNITY_DISPLAY_STYLE_T style);
static void UnityPrintNumber(const long number);

/* Basic Output Functions */
static void UnityPrint(const char* string) {
    if (string != NULL) {
        printf("%s", string);
    }
}

static void UnityPrintLen(const char* string, const UNITY_COUNTER_TYPE length) {
    if (string != NULL) {
        for (UNITY_COUNTER_TYPE i = 0; i < length; i++) {
            putchar(string[i]);
        }
    }
}

static void UnityPrintNumber(const long number) {
    printf("%ld", number);
}

static void UnityPrintNumberByStyle(const long number, const UNITY_DISPLAY_STYLE_T style) {
    switch (style) {
        case UNITY_DISPLAY_STYLE_INT:
            printf("%ld", number);
            break;
        case UNITY_DISPLAY_STYLE_UINT:
            printf("%lu", (unsigned long)number);
            break;
        case UNITY_DISPLAY_STYLE_HEX8:
            printf("0x%02X", (unsigned char)number);
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
            printf("0x%04X", (unsigned short)number);
            break;
        case UNITY_DISPLAY_STYLE_HEX32:
            printf("0x%08lX", (unsigned long)number);
            break;
        case UNITY_DISPLAY_STYLE_CHAR:
            if (number >= 32 && number <= 126) {
                printf("'%c'", (char)number);
            } else {
                printf("'\\x%02X'", (unsigned char)number);
            }
            break;
        default:
            printf("%ld", number);
            break;
    }
}

/* Test Framework Functions */
void UnityBegin(const char* filename) {
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    printf("Unity test run begins\n");
    printf("-----------------------\n");
}

int UnityEnd(void) {
    printf("\n-----------------------\n");
    UnityPrint("Tests run: ");
    UnityPrintNumber(Unity.NumberOfTests);
    
    printf("\nFailures: ");
    UnityPrintNumber(Unity.TestFailures);
    
    printf("\nIgnored: ");
    UnityPrintNumber(Unity.TestIgnores);
    
    if (Unity.TestFailures == 0) {
        printf("\n\nOK\n");
    } else {
        printf("\n\nFAIL\n");
    }
    
    return (int)(Unity.TestFailures);
}

void UnityConcludeTest(void) {
    if (Unity.CurrentTestIgnored) {
        Unity.TestIgnores++;
        printf("IGNORE");
    } else if (Unity.CurrentTestFailed) {
        Unity.TestFailures++;
        printf("FAIL");
    } else {
        printf("PASS");
    }
    
    printf(" (");
    if (Unity.CurrentTestName) {
        printf("%s", Unity.CurrentTestName);
    } else {
        printf("Unknown Test");
    }
    printf(")\n");
}

void UnityDefaultTestRun(void (*Func)(void), const char* FuncName, const int FuncLineNum) {
    Unity.CurrentTestName = FuncName;
    Unity.CurrentTestLineNumber = FuncLineNum;
    Unity.NumberOfTests++;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    setUp();
    
    if (setjmp(Unity.AbortFrame) == 0) {
        Func();
    }
    
    tearDown();
    
    UnityConcludeTest();
}

/* Assert Functions */
void UnityAssertEqualNumber(const long expected, const long actual,
                           const char* msg, const UNITY_COUNTER_TYPE lineNumber,
                           const UNITY_DISPLAY_STYLE_T style) {
    if (expected != actual) {
        Unity.CurrentTestFailed = 1;
        printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
        if (msg) {
            printf("%s", msg);
        }
        printf("\n  Expected: ");
        UnityPrintNumberByStyle(expected, style);
        printf("\n  Actual:   ");
        UnityPrintNumberByStyle(actual, style);
        printf("\n");
        longjmp(Unity.AbortFrame, 1);
    }
}

void UnityAssertEqualIntArray(const int* expected, const int* actual,
                             const UNITY_COUNTER_TYPE num_elements,
                             const char* msg, const UNITY_COUNTER_TYPE lineNumber,
                             const UNITY_DISPLAY_STYLE_T style) {
    UNITY_COUNTER_TYPE i;
    
    if (expected == NULL || actual == NULL) {
        UnityFail("Array pointer is null", lineNumber);
        return;
    }
    
    for (i = 0; i < num_elements; i++) {
        if (expected[i] != actual[i]) {
            Unity.CurrentTestFailed = 1;
            printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
            if (msg) {
                printf("%s", msg);
            }
            printf("\n  Array element [%d] differs", i);
            printf("\n  Expected: ");
            UnityPrintNumberByStyle(expected[i], style);
            printf("\n  Actual:   ");
            UnityPrintNumberByStyle(actual[i], style);
            printf("\n");
            longjmp(Unity.AbortFrame, 1);
        }
    }
}

void UnityAssertEqualString(const char* expected, const char* actual,
                           const char* msg, const UNITY_COUNTER_TYPE lineNumber) {
    if (expected == NULL && actual == NULL) {
        return; /* Both are NULL, that's equal */
    }
    
    if (expected == NULL || actual == NULL || strcmp(expected, actual) != 0) {
        Unity.CurrentTestFailed = 1;
        printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
        if (msg) {
            printf("%s", msg);
        }
        printf("\n  Expected: ");
        if (expected == NULL) {
            printf("NULL");
        } else {
            printf("\"%s\"", expected);
        }
        printf("\n  Actual:   ");
        if (actual == NULL) {
            printf("NULL");
        } else {
            printf("\"%s\"", actual);
        }
        printf("\n");
        longjmp(Unity.AbortFrame, 1);
    }
}

void UnityAssertEqualStringLen(const char* expected, const char* actual,
                              const UNITY_COUNTER_TYPE length,
                              const char* msg, const UNITY_COUNTER_TYPE lineNumber) {
    if (expected == NULL && actual == NULL) {
        return; /* Both are NULL, that's equal */
    }
    
    if (expected == NULL || actual == NULL || strncmp(expected, actual, length) != 0) {
        Unity.CurrentTestFailed = 1;
        printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
        if (msg) {
            printf("%s", msg);
        }
        printf("\n  Expected: ");
        if (expected == NULL) {
            printf("NULL");
        } else {
            printf("\"");
            UnityPrintLen(expected, length);
            printf("\"");
        }
        printf("\n  Actual:   ");
        if (actual == NULL) {
            printf("NULL");
        } else {
            printf("\"");
            UnityPrintLen(actual, length);
            printf("\"");
        }
        printf("\n");
        longjmp(Unity.AbortFrame, 1);
    }
}

void UnityAssertEqualMemory(const void* expected, const void* actual,
                           const UNITY_COUNTER_TYPE length,
                           const UNITY_COUNTER_TYPE num_elements,
                           const char* msg, const UNITY_COUNTER_TYPE lineNumber) {
    const unsigned char* ptr_expected = (const unsigned char*)expected;
    const unsigned char* ptr_actual = (const unsigned char*)actual;
    UNITY_COUNTER_TYPE elements;
    UNITY_COUNTER_TYPE bytes;
    UNITY_COUNTER_TYPE which;

    if (expected == NULL && actual == NULL) {
        return; /* Both are NULL, that's equal */
    }
    
    if (expected == NULL || actual == NULL) {
        UnityFail("Memory pointer is null", lineNumber);
        return;
    }
    
    if (memcmp(expected, actual, length * num_elements) != 0) {
        Unity.CurrentTestFailed = 1;
        printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
        if (msg) {
            printf("%s", msg);
        }
        
        /* Find the first differing byte */
        for (elements = 0; elements < num_elements; elements++) {
            for (bytes = 0; bytes < length; bytes++) {
                which = elements * length + bytes;
                if (ptr_expected[which] != ptr_actual[which]) {
                    printf("\n  Memory differs at element %d, byte %d", elements, bytes);
                    printf("\n  Expected: 0x%02X", ptr_expected[which]);
                    printf("\n  Actual:   0x%02X", ptr_actual[which]);
                    printf("\n");
                    longjmp(Unity.AbortFrame, 1);
                }
            }
        }
    }
}

void UnityAssertPointersEqual(const void* expected, const void* actual,
                             const char* msg, const UNITY_COUNTER_TYPE lineNumber) {
    if (expected != actual) {
        Unity.CurrentTestFailed = 1;
        printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
        if (msg) {
            printf("%s", msg);
        }
        printf("\n  Expected: 0x%p", expected);
        printf("\n  Actual:   0x%p", actual);
        printf("\n");
        longjmp(Unity.AbortFrame, 1);
    }
}

void UnityFail(const char* msg, const UNITY_COUNTER_TYPE lineNumber) {
    Unity.CurrentTestFailed = 1;
    printf("\nFAILURE: %s:%d: ", Unity.TestFile, lineNumber);
    if (msg) {
        printf("%s", msg);
    }
    printf("\n");
    longjmp(Unity.AbortFrame, 1);
}

void UnityIgnore(const char* msg, const UNITY_COUNTER_TYPE lineNumber) {
    Unity.CurrentTestIgnored = 1;
    printf("\nIGNORE: %s:%d: ", Unity.TestFile, lineNumber);
    if (msg) {
        printf("%s", msg);
    }
    printf("\n");
    longjmp(Unity.AbortFrame, 1);
}

/* Weak functions for setUp/tearDown */
__attribute__((weak)) void setUp(void) {
    /* Default empty implementation */
}


__attribute__((weak)) void tearDown(void) {
    /* Default empty implementation */
}


