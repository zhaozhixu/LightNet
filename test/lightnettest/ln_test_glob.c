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

#include <ctype.h>
#include "ln_test_glob.h"
#include "ln_test_bitset.h"

static int glob_match(const char *glob, const char *str)
{
    char *save, *glob_cpy, *glob_cpy2, *substr;
    const char *s = str;
    int match = 1;

    for (glob_cpy2 = glob_cpy = ln_test_strdup(glob), s = str;;
         glob_cpy = NULL) {
        substr = strtok_r(glob_cpy, "*", &save);
        if (!substr) {
            match = 1;
            goto end;
        }
        s = strstr(s, substr);
        if (!s) {
            match = 0;
            goto end;
        }
        if (glob_cpy && glob[0] != '*' && s != str) {
            match = 0;
            goto end;
        }
        s += strlen(substr);
    }
    if (glob[strlen(glob)-1] != '*' && *s)
        match = 0;

end:
    ln_test_free(glob_cpy2);
    return match;
}

static void glob_match_strs(const char *glob, const char *const *strs, int len,
                            ln_test_bitset *result_set)
{
    for (int i = 0; i < len; i++) {
        if (glob_match(glob, strs[i])) {
            ln_test_bitset_set(result_set, i);
        }
    }
}

/* Grammer of the filter string
 * GLOB : [a-zA-Z0-9._*]+
 * OR   : '|'
 * AND  : '&'
 * NOT  : '!'
 * LEFT_BRAC : '('
 * RIGHT_BRAC: ')'
 *
 * goal : expr
 * expr : term OR expr
 *      | term
 * term : fact AND term
 *      | fact
 * fact : NOT fact
 *      | brac
 * brac : LEFT_BRAC expr RIGHT_BRAC
 *      | GLOB
 */

enum non_terminal {
    NT_INVALID = -1,
    NT_GOAL = 0,
    NT_EXPR,
    NT_TERM,
    NT_FACT,
    NT_BRAC,
    NON_TERMINAL_NUM
};

enum terminal {
    T_INVALID = -1,
    T_GLOB = 0,
    T_OR,
    T_AND,
    T_NOT,
    T_LEFT_BRAC,
    T_RIGHT_BRAC,
    TERMINAL_NUM
};

static const enum terminal FIRST[NON_TERMINAL_NUM][TERMINAL_NUM+1] = {
    /* goal */ { T_NOT, T_LEFT_BRAC, T_GLOB, T_INVALID },
    /* expr */ { T_NOT, T_LEFT_BRAC, T_GLOB, T_INVALID },
    /* term */ { T_NOT, T_LEFT_BRAC, T_GLOB, T_INVALID },
    /* fact */ { T_NOT, T_LEFT_BRAC, T_GLOB, T_INVALID },
    /* brac */ { T_LEFT_BRAC, T_GLOB, T_INVALID },
};

static enum terminal get_terminal(const char *word)
{
    const char *s;

    for (s = word; *s; s++) {
        if (!(isalnum(*s)) && *s != '_' && *s != '*' && *s != '.')
            goto NOT_GLOB;
    }
    return T_GLOB;

NOT_GLOB:
    if (*word == '|' && *(word + 1) == '\0')
        return T_OR;

    if (*word == '&' && *(word + 1) == '\0')
        return T_AND;

    if (*word == '!' && *(word + 1) == '\0')
        return T_NOT;

    if (*word == '(' && *(word + 1) == '\0')
        return T_LEFT_BRAC;

    if (*word == ')' && *(word + 1) == '\0')
        return T_RIGHT_BRAC;

    return T_INVALID;
}

static char **add_word(char **words, int *count, const char *word)
{
    char **new_words;

    new_words = realloc(words, sizeof(char *) * (*count + 1));
    new_words[*count] = ln_test_strdup(word);
    (*count)++;

    return new_words;
}

static char **add_char_as_word(char **words, int *count, char c)
{
    char **new_words;

    new_words = realloc(words, sizeof(char *) * (*count + 1));
    new_words[*count] = malloc(sizeof(char) * 2);
    new_words[*count][0] = c;
    new_words[*count][1] = '\0';
    (*count)++;

    return new_words;
}

static char **split(const char *str, const char *delims, int *count_ptr)
{
    char *save, *str_cpy, *str_cpy2, *token;
    char **tokens = NULL;
    int count = 0;

    for (str_cpy2 = str_cpy = ln_test_strdup(str);; str_cpy = NULL) {
        token = strtok_r(str_cpy, delims, &save);
        if (!token)
            break;
        tokens = add_word(tokens, &count, token);
    }
    free(str_cpy2);
    *count_ptr = count;

    return tokens;
}

static int in_word(const char *word, char c)
{
    for (const char *s = word; *s; s++) {
        if (c == *s)
            return 1;
    }
    return 0;
}

static char **tokenize(const char *code, const char *delims, int *count_ptr)
{
    char **words, **tokens = NULL;
    char *glob;
    char delim;
    int words_num, count = 0;

    words = split(code, " \t\n", &words_num);
    if (words_num == 0)
        words = add_word(words, &words_num, "*");
    for (int i = 0; i < words_num; i++) {
        glob = NULL;
        for (char *s = words[i]; *s; s++) {
            if (in_word(delims, *s)) {
                delim = *s;
                *s = '\0';
                if (glob) {
                    if (get_terminal(glob) != T_INVALID) {
                        tokens = add_word(tokens, &count, glob);
                        glob = NULL;
                    } else {
                        fprintf(stderr, "illegal glob: %s\n", glob);
                        exit(EXIT_FAILURE);
                    }
                }
                tokens = add_char_as_word(tokens, &count, delim);
            } else if (!glob) {
                glob = s;
            }
        }
        if (glob)
            tokens = add_word(tokens, &count, glob);
    }
    ln_test_free_arrays(words, words_num);
    *count_ptr = count;

    return tokens;
}

static int in_first(enum terminal look_ahead, enum non_terminal nt)
{
    for (int i = 0; FIRST[nt][i] != T_INVALID; i++) {
        if (FIRST[nt][i] == look_ahead)
            return 1;
    }
    return 0;
}

struct glob_info {
    ln_test_bitset  *result_set;
    const char  *const*strs;
    char        **tokens;
    int           num_strs;
    int           num_tokens;
    int           look_ahead_index;
};
typedef struct glob_info glob_info;

static enum terminal get_look_ahead_terminal(glob_info *info)
{
    if (info->look_ahead_index == info->num_tokens)
        return T_INVALID;
    return get_terminal(info->tokens[info->look_ahead_index]);
}

static void parse_goal(glob_info *info);
static void parse_expr(glob_info *info);
static void parse_term(glob_info *info);
static void parse_fact(glob_info *info);
static void parse_brac(glob_info *info);

static void parse_brac(glob_info *info)
{
    enum terminal look_ahead_term;
    const char *token;

    token = info->tokens[info->look_ahead_index];
    look_ahead_term = get_terminal(token);
    ln_test_bitset_zero(info->result_set);
    if (look_ahead_term == T_LEFT_BRAC) {
        info->look_ahead_index++;
        parse_expr(info);
        if (get_look_ahead_terminal(info) != T_RIGHT_BRAC) {
            fprintf(stderr, "expect ): %s\n",
                    info->tokens[info->look_ahead_index]);
            exit(EXIT_FAILURE);
        }
        info->look_ahead_index++;
    } else if (look_ahead_term == T_GLOB) {
        info->look_ahead_index++;
        glob_match_strs(token, info->strs, info->num_strs, info->result_set);
    } else {
        fprintf(stderr, "expect ( or GLOB: %s\n", token);
        exit(EXIT_FAILURE);
    }
}

static void parse_fact(glob_info *info)
{
    enum terminal look_ahead_term;
    const char *token;

    token = info->tokens[info->look_ahead_index];
    look_ahead_term = get_terminal(token);
    ln_test_bitset_zero(info->result_set);
    if (look_ahead_term == T_NOT) {
        info->look_ahead_index++;
        parse_fact(info);
        ln_test_bitset_not_assign(info->result_set);
    } else if (in_first(look_ahead_term, NT_BRAC)) {
        parse_brac(info);
    } else {
        fprintf(stderr, "expect brac or !: %s\n", token);
        exit(EXIT_FAILURE);
    }
}

static void parse_term(glob_info *info)
{
    enum terminal look_ahead_term;
    const char *token;
    ln_test_bitset *result;

    token = info->tokens[info->look_ahead_index];
    look_ahead_term = get_terminal(token);
    ln_test_bitset_zero(info->result_set);
    if (in_first(look_ahead_term, NT_FACT)) {
        parse_fact(info);
        if (get_look_ahead_terminal(info) == T_AND) {
            info->look_ahead_index++;
            result = ln_test_bitset_copy(info->result_set);
            parse_term(info);
            ln_test_bitset_and_assign(info->result_set, result);
            ln_test_bitset_free(result);
        }
    } else {
        fprintf(stderr, "expect fact: %s\n", token);
        exit(EXIT_FAILURE);
    }
}

static void parse_expr(glob_info *info)
{
    enum terminal look_ahead_term;
    const char *token;
    ln_test_bitset *result;

    token = info->tokens[info->look_ahead_index];
    look_ahead_term = get_terminal(token);
    ln_test_bitset_zero(info->result_set);
    if (in_first(look_ahead_term, NT_TERM)) {
        parse_term(info);
        if (get_look_ahead_terminal(info) == T_OR) {
            info->look_ahead_index++;
            result = ln_test_bitset_copy(info->result_set);
            parse_expr(info);
            ln_test_bitset_or_assign(info->result_set, result);
            ln_test_bitset_free(result);
        }
    } else {
        fprintf(stderr, "expect term: %s\n", token);
        exit(EXIT_FAILURE);
    }
}

static void parse_goal(glob_info *info)
{
    enum terminal look_ahead_term;
    const char *token;

    token = info->tokens[info->look_ahead_index];
    look_ahead_term = get_terminal(token);
    ln_test_bitset_zero(info->result_set);
    if (in_first(look_ahead_term, NT_EXPR)) {
        parse_expr(info);
    } else {
        fprintf(stderr, "expect expr: %s\n", token);
        exit(EXIT_FAILURE);
    }
}

int *ln_test_glob_match(const char *glob, const char *const *strs, int num_strs,
                        int *num_matches)
{
    glob_info info;
    ln_test_bitset *bitset_ones;
    int *result;

    info.strs = strs;
    info.num_strs = num_strs;
    info.result_set = ln_test_bitset_create(64);
    info.tokens = tokenize(glob, "|&!()", &info.num_tokens);
    info.look_ahead_index = 0;

    parse_goal(&info);
    bitset_ones = ln_test_bitset_ones(info.num_strs);
    ln_test_bitset_and_assign(info.result_set, bitset_ones);
    result = ln_test_bitset_create_one_indexes(info.result_set, num_matches);

    ln_test_free_arrays(info.tokens, info.num_tokens);
    ln_test_bitset_free(info.result_set);
    ln_test_bitset_free(bitset_ones);

    return result;
}
