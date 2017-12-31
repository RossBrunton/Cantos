#ifndef _HPP_TEST_
#define _HPP_TEST_

#include <stddef.h>

#include "structures/utf8.hpp"
#include "main/cpp.hpp"
#include "structures/list.hpp"

/** Allows registering and executing of tests
 *
 * To register a test, a subclass of test::TestCase should be created, and its test::TestCase::run_test method must
 *  be implemented with the tests to run. An object of type test::AddTestCase should then be created with the class
 *  as its template parameter; this will add the test to the tests lists. All tests should be in the _tests namespace.
 *
 * As an example of how to create tests:
 *
 * @code
namespace _tests {
class FooTest : public test::TestCase {
public:
    FooTest() : test::TestCase("Foo Test") {};

    void run_test() override {
        test("Foo != Bar Test");
        assert(foo != bar);
        assert(bar != foo);

        test("Foobar Test");
        assert(foobar);
    }
};

test::AddTestCase<FooTest> fooTest;
}
 * @endcode
 *
 * To run these tests, the test::run_tests function should be called, which executes all the tests and provides the
 *  result as a list of test::TestResult.
 *
 * A test::TestCase contains a number of different individual tests. A new test is marked by a test::TestCase::test
 *  call.
 *
 * If the compile time constant `TESTS` is not defined, then no tests will be automatically added to the tests list or
 *  included in the kernel.
 */
namespace test {
    /** A result of a single test::TestCase
     *
     * If multiple tests in a single TestCase fail, this provides details only of the first.
     */
    struct TestResult {
        Utf8 test_case_name; /**< The name of the test case that this is the result of */
        Utf8 test_name; /**< The name of the test that failed, will be undefined if the test case passed */
        bool pass; /**< Whether or not the test failed */
        uint32_t asserts; /**< If the test failed, how many asserts of the current test were successful */
    };

    /** A single test case
     *
     * These should be created using the TEST macro, which will automatically add them to the list of tests.
     *
     * A test case contains a number of tests, which are provided by base classes via an override of its
     *  TestCase::run_test function. This function should call TestCase::test to delimit tests, and TestCase::assert and
     *  TestCase::fail to report results.
     */
    class TestCase {
    private:
        TestResult tr;
        const char *current_test;
        uint32_t asserts;

    protected:
        /** Method for subclasses to specify the body of the test to perform
         *
         * See the description in the clas docs of how to use this.
         */
        virtual void run_test()=0;

    public:
        Utf8 name;
        /** Create a new test case with the given name
         *
         * @param name The name of the test case
         */
        TestCase(const char *name) : name(Utf8(name)) {};
        virtual ~TestCase() {};

        /** Run the test, and give a test::TestResult entry based on the result
         *
         * @return The result of running the test
         */
        TestResult do_test();

        /** If the condition is false, then the test fails
         *
         * @param test The value to assert
         */
        void assert(bool test);
        /** Start a new test
         *
         * @param name The name of the test to start
         */
        void test(const char *name);
        /** Fail the currently running test */
        void fail();
    };

    /** A list of all the currently installed tests */
    extern list<TestCase *> tests;
    /** Run every installed test, and return a list of the results
     *
     * @return The results of testing
     */
    list<TestResult> run_tests();
    /** Given a list of test results, print each of them using printk or kwarn
     *
     * @param results The list of tests to report
     * @param fail_only Report tests only if they are failures
     */
    void print_results(list<TestResult> &results, bool fail_only=true);

    /** Class that, as a side effect of its constructor, adds a test
     *
     * Will do nothing if the macro TESTS is undefined.
     *
     * Its constructor creates a new instance of T, and adds it to test::tests
     */
    template<class T> class AddTestCase {
    public:
        AddTestCase() {
#if TESTS
            T *t = new T;
            tests.push_front(t);
#endif
        }
    };
}

#endif
