#ifndef _HPP_TEST_
#define _HPP_TEST_

#include <stddef.h>

#include "structures/utf8.hpp"
#include "main/cpp.hpp"
#include "structures/list.hpp"

/** Allows registering and executing of tests
 *
 * To register a test, the @ref TEST macro should be used. It is given a test name, a test class name (which must be
 *  unique across all tests) and a function body. This function body will be the body of a test::TestCase::run_test
 *  function, and have access as appropriate. For example:
 *
 * @code
TEST("My Module Test", MyModuleTest, {
    test("Foo Test");
    assert(foo != bar);
    assert(bar != foo);

    test("Foobar Test");
    assert(foobar);
})
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
        Utf8 name;
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
        /** Create a new test case with the given name
         *
         * @param name The name of the test case
         */
        TestCase(Utf8 name) : name(name) {};
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
     * Its constructor creates a new instance of T, and adds it to test::tests
     */
    template<class T> class AddTestCase {
    public:
        AddTestCase() {
            T *t = new T;
            tests.push_front(t);
        }
    };


/** @def TEST(name, className, fn)
 *
 * Registers a new test in the tests list if `TESTS` is defined as a compile time constant
 *
 * If `TESTS` is not defined, then the macro evaluates to nothing, and does nothing.
 *
 * To create and add a test, it does the following:
 *  * Creates a new subclass of test::TestCase
 *  * Sets the test function appropriately
 *  * Creates an instance of test::AddTestCase with the new class
 *
 * @param name The name of the test case
 * @param className The name to use as the class of the test case
 * @param fn The body of the function to use as the test case's test::TestCase::run_test function.
 */
#ifdef TESTS
#define TEST(name, className, fn) namespace _test_zone {class className: public test::TestCase {\
public:\
    className(Utf8 testName = name) : TestCase(testName) {};\
protected:\
    virtual void run_test() override fn;\
};\
test::AddTestCase<className> className##_adder;}

#else
#define TEST(name, className, fn)
#endif
}

#endif
