#define BOOST_TEST_MODULE main
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_framework_operational ) {
    BOOST_CHECK_EQUAL(0,0);
}
