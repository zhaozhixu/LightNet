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

#ifndef _LN_OPTION_H_
#define _LN_OPTION_H_

struct ln_option {
    const char  *source;
    const char  *outfile;
    const char  *target;
    const char  *datafile;
    int          compile;
    int          run;
    int          Winter;
    int          Wwarn;
    int          debug;
};
typedef struct ln_option ln_option;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_option *ln_option_create(int argc, char **argv);
void ln_option_free(ln_option *option);

/* those getters are for the convinence of python bindings */
const char *ln_option_get_source(ln_option *option);
const char *ln_option_get_outfile(ln_option *option);
const char *ln_option_get_target(ln_option *option);
const char *ln_option_get_datafile(ln_option *option);
int ln_option_get_compile(ln_option *option);
int ln_option_get_run(ln_option *option);
int ln_option_get_Winter(ln_option *option);
int ln_option_get_Wwarn(ln_option *option);
int ln_option_get_debug(ln_option *option);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_OPTION_H_ */
