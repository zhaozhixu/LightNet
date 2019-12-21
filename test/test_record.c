/*
 * Copyright (c) 2018-2019 Zhao Zhixu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include "test_record.h"

test_record *test_record_create(void)
{
    test_record *record = malloc(sizeof(test_record));
    if (!record)
        err(EXIT_FAILURE, "create_test_record() failed");
    record->suite_names = NULL;
    record->suites_num = 0;
    record->tcase_names_array = NULL;
    record->tcase_ptrs_array = NULL;
    record->tcase_nums = NULL;

    return record;
}

void test_record_free(test_record *record)
{
    for (int i = 0; i < record->suites_num; i++) {
        free((char *)record->suite_names[i]);
        if (record->tcase_nums[i] <= 0)
            continue;
        for (int j = 0; j < record->tcase_nums[i]; j++) {
            free((char *)record->tcase_names_array[i][j]);
        }
        free(record->tcase_names_array[i]);
        free(record->tcase_ptrs_array[i]);
    }
    free(record->tcase_names_array);
    free(record->tcase_nums);
    free(record->suite_names);
    free(record);
}

int test_record_add_suite(test_record *record, const char *suite_name)
{
    for (const char *p = suite_name; *p; p++) {
        if (!(isalnum(*p) || *p == '_')) {
            fprintf(stderr, "suite_name must be in [a-zA-Z0-9_], but got %s\n",
                    suite_name);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < record->suites_num; i++) {
        if (!strcmp(record->suite_names[i], suite_name))
            return record->suites_num;
    }

    record->suite_names = realloc(record->suite_names,
                                  sizeof(char *) * (record->suites_num + 1));
    if (!record->suite_names)
        err(EXIT_FAILURE, "realloc() failed");

    record->suite_names[record->suites_num] = strdup(suite_name);
    if (!record->suite_names[record->suites_num])
        err(EXIT_FAILURE, "strdup() failed");

    record->tcase_nums = realloc(record->tcase_nums,
                                 sizeof(int) * (record->suites_num + 1));
    if (!record->tcase_nums)
        err(EXIT_FAILURE, "realloc() failed");
    record->tcase_nums[record->suites_num] = 0;

    record->tcase_names_array = realloc(
        record->tcase_names_array, sizeof(char **) * (record->suites_num + 1));
    if (!record->tcase_names_array)
        err(EXIT_FAILURE, "realloc() failed");
    record->tcase_names_array[record->suites_num] = NULL;

    record->tcase_ptrs_array = realloc(
        record->tcase_ptrs_array, sizeof(void *) * (record->suites_num + 1));
    if (!record->tcase_ptrs_array)
        err(EXIT_FAILURE, "realloc() failed");
    record->tcase_ptrs_array[record->suites_num] = NULL;

    return ++record->suites_num;
}

int test_record_add_tcase(test_record *record, const char *suite_name,
                          const char *tcase_name, const void * tcase_ptr)
{
    for (const char *p = tcase_name; *p; p++) {
        if (!(isalnum(*p) || *p == '_')) {
            fprintf(stderr, "tcase_name must be in [a-zA-Z0-9_], but got %s\n",
                    tcase_name);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < record->suites_num; i++) {
        if (strcmp(record->suite_names[i], suite_name))
            continue;

        for (int j = 0; j < record->tcase_nums[i]; j++) {
            if (!strcmp(record->tcase_names_array[i][j], tcase_name))
                return record->tcase_nums[i];
        }

        record->tcase_names_array[i] = realloc(
            record->tcase_names_array[i],
            sizeof(char *) * (record->tcase_nums[i] + 1));
        if (!record->tcase_names_array[i])
            err(EXIT_FAILURE, "realloc() failed");

        record->tcase_names_array[i][record->tcase_nums[i]] = strdup(tcase_name);
        if (!record->tcase_names_array[i][record->tcase_nums[i]])
            err(EXIT_FAILURE, "strdup() failed");

        record->tcase_ptrs_array[i] = realloc(
            record->tcase_ptrs_array[i],
            sizeof(void *) * (record->tcase_nums[i] + 1));
        if (!record->tcase_ptrs_array[i])
            err(EXIT_FAILURE, "realloc() failed");
        record->tcase_ptrs_array[i][record->tcase_nums[i]] = tcase_ptr;

        return ++record->tcase_nums[i];
    }

    fprintf(stderr, "no suite name '%s' found\n", suite_name);
    exit(EXIT_FAILURE);
}

static inline const char *inverse_find(const char *start, char g)
{
    const char *s = start;
    const char *end;

    while (*s++);
    end = s;
    while (s >= start && g != *s)
        s--;

    return s < start ? end : s;
}

static int glob_match(const char *glob, const char *str)
{
    const char *g = glob;
    const char *s = str;

    if (*g == '!')
        return !glob_match(g + 1, str);

    while (*g && *s) {
        if (*g == '*') {
            g++;
            s = inverse_find(s, *g);
            /* while (*s != *g && *s++); */
            continue;
        }
        if (*s != *g)
            return 0;
        g++, s++;
    }
    if (!*g && !*s)
        return 1;
    return 0;
}

static int glob_match_strs(const char *glob, const char **strs,
                           int start, int len)
{
    if (start < 0 || len <= 0) {
        return -1;
    }

    for (int i = start; i < len; i++) {
        if (glob_match(glob, strs[i]))
            return i;
    }
    return -1;
}

test_record *test_record_create_filtered(const test_record *record,
                                         const char *glob)
{
    test_record *filtered_record;
    char *saveptr;
    char *glob_cpy;
    char *token;
    char *s;
    int suite_match, tcase_match, filtered_suite_num;

    filtered_record = test_record_create();
    if (!glob)
        glob_cpy = strdup("*");
    else
        glob_cpy = strdup(glob);
    if (!glob_cpy)
        err(EXIT_FAILURE, "strdup() failed");
    for (s = glob_cpy; (token = strtok_r(s, "|", &saveptr)); s = NULL) {
        s = strchr(token, '.');
        if (s) {
            *s = '\0';
        }
        suite_match = -1;
        while (1) {
            suite_match = glob_match_strs(token, record->suite_names,
                                          suite_match + 1, record->suites_num);
            if (suite_match < 0)
                break;
            filtered_suite_num = test_record_add_suite(
                filtered_record, record->suite_names[suite_match]);
            if (!s) {
                filtered_record->tcase_nums[filtered_suite_num - 1] = -1;
                continue;
            }
            tcase_match = -1;
            while (1) {
                tcase_match = glob_match_strs(
                    s + 1, record->tcase_names_array[suite_match],
                    tcase_match + 1, record->tcase_nums[suite_match]);
                if (tcase_match < 0)
                    break;
                test_record_add_tcase(
                    filtered_record, record->suite_names[suite_match],
                    record->tcase_names_array[suite_match][tcase_match],
                    record->tcase_ptrs_array[suite_match][tcase_match]);
            }
        }
    }

    free(glob_cpy);
    return filtered_record;
}
