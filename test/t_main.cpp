#define BOOST_TEST_MODULE main
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <jitney.h>

BOOST_AUTO_TEST_CASE( test_framework_operational ) {
    BOOST_CHECK_EQUAL(0,0);
}

int main(int argc, char ** argv) { 
    // parse jitney command line options and establish the scope alowing jitney to run.
    jitney::options opt(argc,argv,false);  

    // parse boost unit test framework command line options
    using namespace boost::unit_test;
    return unit_test_main(&init_unit_test, argc, argv);
}
