#include "test_lightnet.h"
#include "../src/ln_util.h"
#include "../src/ln_list.h"

static int *data;
static size_t data_len;
static ln_list *list;

static void free_int(int *i)
{
     free(i);
}

static void free_int_wrapper(void *p)
{
     free_int(p);
}

static void setup(void)
{
     int i;
     list = NULL;
     data_len = 5;
     data = ln_alloc(sizeof(int) * data_len);
     for (i = 0; i < 5; i++) {
          data[i] = i;
          list = ln_list_append(list, &data[i]);
     }
}

static void teardown(void)
{
     ln_free(data);
     ln_list_free(list);
}

START_TEST(test_list_append_nth)
{
     ln_list *l;

     l = ln_list_append(NULL, &data[0]);
     l = ln_list_append(l, &data[1]);
     ck_assert_int_eq(*(int *)ln_list_nth_data(l, 0), 0);
     ck_assert_int_eq(*(int *)ln_list_nth_data(l, 1), 1);
     ln_list_free(l);
}
END_TEST

START_TEST(test_list_remove)
{
     int num, i;

     list = ln_list_remove(list, &data[0]);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 0), 1);

     list = ln_list_insert_nth(list, &data[0], 0);
     list = ln_list_remove(list, &data[4]);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 3), 3);
     ck_assert_ptr_eq(ln_list_nth_data(list, 4), NULL);

     list = ln_list_insert_nth(list, &data[4], 4);
     num = -1;
     list = ln_list_remove(list, &num);
     for (i = 0; i < data_len; i++)
          ck_assert_int_eq(*(int *)ln_list_nth_data(list, i), data[i]);

     num = 5;
     list = ln_list_remove(list, &num);
     for (i = 0; i < data_len; i++)
          ck_assert_int_eq(*(int *)ln_list_nth_data(list, i), data[i]);
}
END_TEST

START_TEST(test_list_remove_insert_nth)
{
     int i;

     list = ln_list_remove_nth(list, 2);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 2), 3);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 1), 1);

     list = ln_list_insert_nth(list, &data[2], 2);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 2), 2);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 3), 3);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 1), 1);

     list = ln_list_remove_nth(list, 0);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 0), 1);

     list = ln_list_insert_nth(list, &data[0], 0);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 0), 0);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 1), 1);

     list = ln_list_remove_nth(list, 4);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 3), 3);
     ck_assert_ptr_eq(ln_list_nth_data(list, 4), NULL);

     list = ln_list_insert_nth(list, &data[4], 4);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 4), 4);
     ck_assert_int_eq(*(int *)ln_list_nth_data(list, 3), 3);

     list = ln_list_remove_nth(list, -1);
     for (i = 0; i < data_len; i++)
          ck_assert_int_eq(*(int *)ln_list_nth_data(list, i), data[i]);

     list = ln_list_remove_nth(list, 6);
     for (i = 0; i < data_len; i++)
          ck_assert_int_eq(*(int *)ln_list_nth_data(list, i), data[i]);
}
END_TEST

START_TEST(test_list_find)
{
     ln_list *l;
     int n1 = 6;
     int n2 = -1;

     l = ln_list_find(list, &data[3]);
     ck_assert_int_eq(*(int *)l->data, data[3]);

     l = ln_list_find(list, &n1);
     ck_assert_ptr_eq(l, NULL);

     l = ln_list_find(list, &n2);
     ck_assert_ptr_eq(l, NULL);
}
END_TEST

static int cmp(void *a, void *b)
{
     return *(int *)a - *(int *)b;
}

START_TEST(test_list_find_custom)
{
     ln_list *l;
     int n1 = 6;
     int n2 = -1;

     l = ln_list_find_custom(list, &data[3], &cmp);
     ck_assert_int_eq(*(int *)l->data, data[3]);

     l = ln_list_find_custom(list, &n1, &cmp);
     ck_assert_ptr_eq(l, NULL);

     l = ln_list_find_custom(list, &n2, &cmp);
     ck_assert_ptr_eq(l, NULL);
}
END_TEST

START_TEST(test_list_position)
{
     int pos;
     ln_list *l;

     pos = ln_list_position(list, list->next);
     ck_assert_int_eq(pos, 1);

     l = (ln_list *)ln_alloc(sizeof(ln_list));
     pos = ln_list_position(list, l);
     ck_assert_int_eq(pos, -1);

     pos = ln_list_position(list, NULL);
     ck_assert_int_eq(pos, -1);
}
END_TEST

START_TEST(test_list_index)
{
     int pos;
     int n;

     pos = ln_list_index(list, list->next->data);
     ck_assert_int_eq(pos, 1);

     n = 6;
     pos = ln_list_index(list, &n);
     ck_assert_int_eq(pos, -1);

     pos = ln_list_index(list, NULL);
     ck_assert_int_eq(pos, -1);
}
END_TEST

START_TEST(test_list_length)
{
     ln_list *l;
     int data;

     l = NULL;
     data = 0;
     ck_assert_int_eq(ln_list_length(l), 0);
     l = ln_list_append(l, &data);
     ck_assert_int_eq(ln_list_length(l), 1);
     ck_assert_int_eq(ln_list_length(list), 5);
}
END_TEST

START_TEST(test_list_from_array_size_t)
{
     ln_list *l;
     size_t i;
     size_t array[3];

     for (i = 0; i < 3; i++)
          array[i] = i;
     l = ln_list_from_array_size_t(NULL, 0);
     ck_assert_ptr_eq(l, NULL);
     l = ln_list_from_array_size_t(array, 3);
     for (i = 0; i < 3; i++) {
          ck_assert_int_eq((size_t)ln_list_nth_data(l, i), i);
     }
}
END_TEST

START_TEST(test_ln_list_free_deep)
{
     ln_list *l;
     int *int1, *int2;

     int1 = ln_alloc(sizeof(int));
     int2 = ln_alloc(sizeof(int));
     *int1 = 1;
     *int2 = 2;
     l = ln_list_append(NULL, int1);
     l = ln_list_append(l, int2);
     ln_list_free_deep(l, free_int_wrapper);
}
END_TEST
/* end of tests */

Suite *make_list_suite(void)
{
     Suite *s;
     s = suite_create("list");

     TCase *tc_list;
     tc_list = tcase_create("tc_list");
     tcase_add_checked_fixture(tc_list, setup, teardown);

     tcase_add_test(tc_list, test_list_append_nth);
     tcase_add_test(tc_list, test_list_remove);
     tcase_add_test(tc_list, test_list_remove_insert_nth);
     tcase_add_test(tc_list, test_list_find);
     tcase_add_test(tc_list, test_list_find_custom);
     tcase_add_test(tc_list, test_list_position);
     tcase_add_test(tc_list, test_list_index);
     tcase_add_test(tc_list, test_list_length);
     tcase_add_test(tc_list, test_list_from_array_size_t);
     tcase_add_test(tc_list, test_ln_list_free_deep);
     /* end of adding tests */

     suite_add_tcase(s, tc_list);

     return s;
}
