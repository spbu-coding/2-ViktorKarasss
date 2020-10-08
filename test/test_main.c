#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "cmocka.h"

#ifdef _WIN32
/* Compatibility with the Windows standard C library. */
#define vsnprintf _vsnprintf
#endif /* _WIN32 */

#if defined(scanf) || defined(printf) || defined(fprintf)
#error "Not handled yet: redefinition of stdio functions"
#endif

int __real_main();

#define array_length(x) (sizeof(x) / sizeof((x)[0]))

static char temporary_buffer[256];
static char temporary_buffer_stdout[256];
static char temporary_buffer_stderr[256];
static int num_of_test;
static int test_input1[] = {2, 4, 1, 5, 8, 4};
static int test_input2[] = {4, 7, 10, 5, 1, 6, 8};
static int idx;

/* A mock fprintf function that checks the value of strings printed to the
 * standard error stream or output stream. */
static void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

int __wrap_fprintf(FILE* const file, const char *format, ...) {
    int return_value;
    va_list args;
    assert_true(file == stdout || file == stderr);
    va_start(args, format);
    return_value = vsnprintf(temporary_buffer, sizeof(temporary_buffer),
                             format, args);

    remove_spaces(temporary_buffer);

    if (file == stdout) {
        strcpy(temporary_buffer_stdout, temporary_buffer);
        check_expected_ptr(temporary_buffer_stdout);
    }
    else {
        strcpy(temporary_buffer_stderr, temporary_buffer);
        check_expected_ptr(temporary_buffer_stderr);
    }

    va_end(args);
    return return_value;
}

/* A mock printf function that checks the value of strings printed to the
 * standard output stream. */
int __wrap_printf(const char *format, ...) {
    int return_value;
    va_list args;
    va_start(args, format);
    return_value = vsnprintf(temporary_buffer, sizeof(temporary_buffer),
                             format, args);
    remove_spaces(temporary_buffer);
    check_expected_ptr(temporary_buffer);
    va_end(args);
    return return_value;
}

int __wrap_scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    switch (num_of_test) {
        case 1: *va_arg(args, int*) = test_input1[idx++];
            if (array_length(test_input1) == idx) *va_arg(args, char*) = '\n';
            break;
        case 2: *va_arg(args, int*) = test_input2[idx++];
            if (array_length(test_input2) == idx) *va_arg(args, char*) = '\n';
            break;
        default: 
            va_end(args);
	     return -1;
    }

    va_end(args);
    return 1; // TODO: Return proper count
}


int __wrap___isoc99_scanf(const char *restrict format, ...) {
     void *args = __builtin_apply_args();
     void *ret = __builtin_apply((void (*)())__wrap_scanf, args, 1000);
     __builtin_return(ret);
}

static void test_example_main_no_args(void **state) {
    const char *args[] = {
            "example",
    };

    (void) state; /* unused */

    assert_int_equal(__real_main(array_length(args), (char **) args), -1);
}

static void test_example_main_many_args(void **state) {
    const char *args[] = {
            "example", "--from=3", "--to=3", "something bad"
    };

    (void) state; /* unused */

    assert_int_equal(__real_main(array_length(args), (char **) args), -2);
}

static void test_main_1(void **state) {
    num_of_test = 1;
    idx = 0;
    const char *args[] = {
            "example", "--from=3",
    };

    (void) state; /* unused */

    expect_string(__wrap_fprintf, temporary_buffer_stdout, "2");
    expect_string(__wrap_fprintf, temporary_buffer_stdout, "1");

    assert_int_equal(__real_main(array_length(args), args), 3);
}

static void test_main_2(void **state){
    num_of_test = 2;
    idx = 0;
    const char *args[] = {
            "example", "--to=9", "--from=3"
    };

    (void) state; /* unused */

    expect_string(__wrap_fprintf, temporary_buffer_stderr, "10");
    expect_string(__wrap_fprintf, temporary_buffer_stdout, "1");

    assert_int_equal(__real_main(array_length(args), (char **) args), 3);
}

int __wrap_main()
{
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_example_main_no_args),
            cmocka_unit_test(test_example_main_many_args),
            cmocka_unit_test(test_main_1),
            cmocka_unit_test(test_main_2),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

