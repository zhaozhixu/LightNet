/*
 * Copyright (c) 2018 Zhao Zhixu
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

#include <sys/stat.h>
#include "test_lightnet.h"
#include "../src/ln_op.h"
#include "../src/ln_parse.h"

static char *json_str;
static ln_list *registered_ops;
static ln_error *error = NULL;
extern ln_op *ln_init_ops;

static void setup(void)
{
     struct stat buf;
     FILE *fp;
     size_t n;

     if (stat("test_ln_parse.json", &buf) < 0) {
          perror("Cannot stat test_ln_parse.json");
          exit(EXIT_FAILURE);
     }

     json_str = ln_alloc(buf.st_size);
     if (!(fp = fopen("test_ln_parse.json", "rb"))) {
          perror("Cannot open test_ln_parse.json");
          exit(EXIT_FAILURE);
     }
     n = fread(json_str, buf.st_size, 1, fp);
     if (ferror(fp)) {
          perror("Error reading test_ln_parse.json");
          exit(EXIT_FAILURE);
     }

     fclose(fp);

     registered_ops = ln_
}

static void teardown(void)
{
     ln_free(json_str);
}

START_TEST(test_ln_parse_ops)
{
     ln_list *ops;


}
END_TEST
/* end of tests */

Suite *make_parse_suite(void)
{
     Suite *s;
     TCase *tc_parse;

     s = suite_create("parse");
     tc_parse = tcase_create("parse");
     tcase_add_checked_fixture(tc_parse, setup, teardown);

     tcase_add_test(tc_parse, test_ln_parse_ops);
     /* end of adding tests */

     suite_add_tcase(s, tc_parse);

     return s;
}
