gcc test.c -D EXECUTE_LOOP_CODE -o test_loop
gcc test.c -o test
gcc test.c -D EXECUTE_LOOP_CODE -D VERBOSE -o test_loop_verbose
gcc test.c -D VERBOSE -o test_verbose
gcc support_test.c -o support_test
gcc test_signal.c -D EXECUTE_LOOP_CODE -o test_signal
