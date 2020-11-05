/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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
#include "ln_test_record.h"
#include "ln_test_glob.h"

void ln_test_record_init(ln_test_record *record)
{
    record->suite_names = NULL;
    record->suite_ptrs = NULL;
    record->suites_num = 0;
    record->test_names_array = NULL;
    record->test_nums = NULL;
}

ln_test_record *ln_test_record_create(void)
{
    ln_test_record *record = ln_test_alloc(sizeof(ln_test_record));
    ln_test_record_init(record);

    return record;
}

void ln_test_record_finalize(ln_test_record *record)
{
    for (int i = 0; i < record->suites_num; i++) {
        free((char *)record->suite_names[i]);
        if (record->test_nums[i] <= 0)
            continue;
        for (int j = 0; j < record->test_nums[i]; j++) {
            free((char *)record->test_names_array[i][j]);
        }
        free(record->test_names_array[i]);
    }
    free(record->test_names_array);
    free(record->test_nums);
    free(record->suite_names);
    free(record->suite_ptrs);
}

void ln_test_record_free(ln_test_record *record)
{
    ln_test_record_finalize(record);
    free(record);
}

int ln_test_record_add_suite(ln_test_record *record, const char *suite_name,
                             const void *suite_ptr)
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

    record->suite_names = ln_test_realloc(
        record->suite_names, sizeof(char *) * (record->suites_num + 1));
    record->suite_names[record->suites_num] = ln_test_strdup(suite_name);
    record->suite_ptrs = ln_test_realloc(
        record->suite_ptrs, sizeof(void *) * (record->suites_num + 1));
    record->suite_ptrs[record->suites_num] = suite_ptr;
    record->test_nums = ln_test_realloc(
        record->test_nums, sizeof(int) * (record->suites_num + 1));
    record->test_nums[record->suites_num] = 0;
    record->test_names_array = ln_test_realloc(
        record->test_names_array, sizeof(char **) * (record->suites_num + 1));
    record->test_names_array[record->suites_num] = NULL;

    return ++record->suites_num;
}

int ln_test_record_add_test(ln_test_record *record, const char *suite_name,
                            const char *test_name)
{
    for (const char *p = test_name; *p; p++) {
        if (!(isalnum(*p) || *p == '_')) {
            fprintf(stderr, "test_name must be in [a-zA-Z0-9_], but got %s\n",
                    test_name);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < record->suites_num; i++) {
        if (strcmp(record->suite_names[i], suite_name))
            continue;

        for (int j = 0; j < record->test_nums[i]; j++) {
            if (!strcmp(record->test_names_array[i][j], test_name))
                return record->test_nums[i];
        }

        record->test_names_array[i] = ln_test_realloc(
            record->test_names_array[i],
            sizeof(char *) * (record->test_nums[i] + 1));

        record->test_names_array[i][record->test_nums[i]] =
                ln_test_strdup(test_name);

        return ++record->test_nums[i];
    }

    fprintf(stderr, "no suite name '%s' found\n", suite_name);
    exit(EXIT_FAILURE);
}

const void *ln_test_record_find_suite_ptr(const ln_test_record *record,
                                          const char *name)
{
    for (int i = 0; i < record->suites_num; i++) {
        if (!strcmp(name, record->suite_names[i]))
            return record->suite_ptrs[i];
    }

    return NULL;
}

static char **ln_test_record_create_names(const ln_test_record *record,
                                          int *count)
{
    char **names;
    int name_len;
    int index;

    *count = 0;
    for (int i = 0; i < record->suites_num; i++) {
        *count += record->test_nums[i];
    }

    index = 0;
    names = ln_test_alloc(sizeof(char *) * *count);
    for (int i = 0; i < record->suites_num; i++) {
        for (int j = 0; j < record->test_nums[i]; j++) {
            name_len = strlen(record->suite_names[i]) +
                    strlen(record->test_names_array[i][j]) + 2;
            names[index] = ln_test_alloc(sizeof(char) * name_len);
            snprintf(names[index], name_len, "%s.%s",
                     record->suite_names[i], record->test_names_array[i][j]);
            index++;
        }
    }

    return names;
}

ln_test_record *ln_test_record_create_filtered(const ln_test_record *record,
                                               const char *glob)
{
    ln_test_record *filtered_record;
    char **names;
    int num_names;
    int *match_indexes;
    int num_matches;
    char *suite_name, *test_name;

    filtered_record = ln_test_record_create();
    names = ln_test_record_create_names(record, &num_names);
    match_indexes = ln_test_glob_match(glob, (const char *const *)names,
                                       num_names, &num_matches);

    for (int i = 0; i < num_matches; i++) {
        suite_name = names[match_indexes[i]];
        test_name = strchr(suite_name, '.');
        if (!test_name) {
            fprintf(stderr, "%s(): invalid test name '%s'\n",
                    __func__, suite_name);
            exit(EXIT_FAILURE);
        }
        *test_name = '\0';
        test_name += 1;
        ln_test_record_add_suite(
            filtered_record, suite_name,
            ln_test_record_find_suite_ptr(record, suite_name));
        ln_test_record_add_test(filtered_record, suite_name, test_name);
    }

    ln_test_free_arrays(names, num_names);
    ln_test_free(match_indexes);

    return filtered_record;
}

void ln_test_record_print(const ln_test_record *record)
{
    for (int i = 0; i < record->suites_num; i++) {
        printf("suite: %s\n", record->suite_names[i]);
        for (int j = 0; j < record->test_nums[i]; j++) {
            printf("\ttest: %s\n", record->test_names_array[i][j]);
        }
    }
}
