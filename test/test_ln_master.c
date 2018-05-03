#include "test_lightnet.h"

static void setup(void)
{
}

static void teardown(void)
{
}

/* end of tests */

Suite *make_master_suite(void)
{
     Suite *s;
     TCase *tc_master;

     s = suite_create("master");
     tc_master = tcase_create("master");
     tcase_add_checked_fixture(tc_master, setup, teardown);


     /* end of adding tests */

     suite_add_tcase(s, tc_master);

     return s;
}
