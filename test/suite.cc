#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <jit++/internal.h>
#include <jit++/exception.h>
#include <jit++/interpreter.h>
#include <jit++/memory.h>

BOOST_AUTO_TEST_CASE( unsupported_opcode_exception_disassembly ) {
    uint8_t buffer[] = { 0x65, 0x67, 0x89, 0x87, 0x76, 0x65, 0x00, 0x00 };
    jitpp::unsupported_opcode_exception ex(buffer);
    BOOST_CHECK_EQUAL(ex.what(),ex.what());
    BOOST_CHECK(ex.what()[0] != '\0');
    std::string what(ex.what() + 20);
    std::string expected_what("6567898776650000                 mov %eax, %gs:0x6576(%edi)\n");
    BOOST_CHECK_EQUAL(what,expected_what);
}

BOOST_AUTO_TEST_CASE( likely_identity ) {
    using namespace jitpp;
    BOOST_CHECK_EQUAL(likely(true), true);
    BOOST_CHECK_EQUAL(likely(false), false);
    BOOST_CHECK_EQUAL(unlikely(false), false);
    BOOST_CHECK_EQUAL(unlikely(true), true);
}

/*
BOOST_AUTO_TEST_CASE( start_interpreter ) { 
    jitpp::interpreter i;
    BOOST_MESSAGE("starting interpreter");
    i();
    BOOST_CHECK_EQUAL(1,1);
}
*/

/*
int executable_test(int j) { 
    int i = j + 2;
    return i;
}

BOOST_AUTO_TEST_CASE( native_is_executable_test ) { 
    typedef jitpp::native_memory_permission_model model;
    model m;
    model::reader reader = model::reader(m);
    BOOST_CHECK(reader.is_executable(reinterpret_cast<const void *>(&executable_test)));
}
*/
