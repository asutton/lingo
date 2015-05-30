# Copyright (c) 2015 Andrew Sutton
# All rights reserved

macro(add_test_program target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} lingo)
endmacro()

# Testing tools
add_test_program(json-parser json-parser.cpp)
add_test_program(arg-parser arg-parser.cpp)

# JSON tests
# TODO: Validate the output? Add negative tests?
add_test(test_null json-parser "null")
add_test(test_true json-parser "true")
add_test(test_false json-parser "false")

add_test(test_int_0 json-parser "0")
add_test(test_int_1 json-parser "1")
add_test(test_int_2 json-parser "-1")

add_test(test_string_0 json-parser "\"\"")
add_test(test_string_1 json-parser "\"hello\"")

add_test(test_array_0 json-parser "[]")
add_test(test_array_1 json-parser "[1, 2, 3]")
add_test(test_array_2 json-parser "[\"a\", \"b\", 3]")

add_test(test_object_0 json-parser "{}")
add_test(test_object_0 json-parser "{\"a\":1, \"b\":2}")


# CLI tests
# TODO: Validate the output? Add negative tests?
add_test(test_short_args_0 arg-parser)
add_test(test_short_args_1 arg-parser -v)
add_test(test_short_args_2 arg-parser -v -h)
add_test(test_short_args_3 arg-parser -vh)
add_test(test_short_args_4 arg-parser -f foo)
add_test(test_short_args_5 arg-parser -f=foo)
add_test(test_short_args_6 arg-parser -vhf foo)
add_test(test_short_args_7 arg-parser -vhf=foo)

# FIXME: Add long name tests.
add_test(test_long_args_0 arg-parser --version)
add_test(test_long_args_1 arg-parser --version --help)
add_test(test_long_args_2 arg-parser --file foo)
add_test(test_long_args_3 arg-parser --file=foo)
add_test(test_long_args_4 arg-parser "--file=[1, 2, 3]")
