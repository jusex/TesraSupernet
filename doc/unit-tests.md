Compiling/running unit tests
------------------------------------

Unit tests will be automatically compiled if dependencies were met in configure
and tests weren't explicitly disabled.

After configuring, they can be run with 'make check'.

To run the tesrad tests manually, launch src/test/test_tesra .

To add more tesrad tests, add `BOOST_AUTO_TEST_CASE` functions to the existing
.cpp files in the test/ directory or add new .cpp files that
implement new BOOST_AUTO_TEST_SUITE sections.

To run the tesra-qt tests manually, launch src/qt/test/tesra-qt_test

To add more tesra-qt tests, add them to the `src/qt/test/` directory and
the `src/qt/test/test_main.cpp` file.
