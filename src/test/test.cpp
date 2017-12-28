#include "test/test.hpp"
#include "structures/list.hpp"
#include "main/printk.hpp"

namespace test {
    list<TestCase *> tests;

    list<TestResult> run_tests() {
        list<TestResult> results;

        for(TestCase *t : tests) {
            results.push_back(t->do_test());
        }

        return results;
    }

    void print_results(list<TestResult> &results, bool fail_only) {
        for(TestResult &r : results) {
            if(r.pass) {
                if(!fail_only) {
                    printk("PASS: %s\n", r.test_case_name.to_string());
                }
            }else{
                kwarn("FAIL: %s\n", r.test_case_name.to_string());
                kwarn("> %s [Assert #%d]\n", r.test_name.to_string(), r.asserts);
            }
        }
    }

    void TestCase::assert(bool test) {
        if(!test) {
            fail();
        }
        asserts ++;
    }

    void TestCase::test(const char *name) {
        current_test = name;
        asserts = 0;
    }

    TestResult TestCase::do_test() {
        tr.test_case_name = name;
        tr.pass = true;

        run_test();

        return tr;
    }

    void TestCase::fail() {
        if(tr.pass) {
            __asm__ __volatile__ (""); // For giving something for gdb to latch on to
            tr.test_name = Utf8(current_test);
            tr.pass = false;
            tr.asserts = asserts;
        }
    }
}
