#include "ln_error.h"

int main(int argc, char **argv)
{
    ln_error *error;

    error = ln_error_create(LN_INFO, "test error %s", "test_str");
    ln_error_handle(&error);
}
