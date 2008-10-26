#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <jit++/common.h>
#include <jit++/exception.h>

BOOST_AUTO_TEST_CASE( likely_identity ) {
    using namespace jitpp;
    BOOST_CHECK_EQUAL(likely(true), true);
    BOOST_CHECK_EQUAL(likely(false), false);
    BOOST_CHECK_EQUAL(unlikely(false), false);
    BOOST_CHECK_EQUAL(unlikely(true), true);
}

