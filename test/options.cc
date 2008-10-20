
#include <string.h>

#define BOOST_TEST_MODULE main
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <gflags/gflags.h>
#include <jit++.h>

BOOST_AUTO_TEST_CASE( test_framework_operational ) {
    BOOST_CHECK_EQUAL(0,0);
}

DEFINE_bool(boost_build_info,false,"makes the framework print build information: platform, compiler, STL implementation, boost version");
DEFINE_bool(boost_catch_system_errors,true,"=false prohibits the framework from catching asynchronous system events.");
DEFINE_int32(boost_detect_memory_leak,1,"detect memory leaks. 0 means don't try. n means every n memory alocations set a breakpoint");
DEFINE_string(boost_log_format,"HRF","XML or HRF (Human readable format)");
DEFINE_string(boost_output_format,"HRF","XML or HRF (Human readable format)");
DEFINE_string(boost_report_format,"HRF","XML or HRF (Human readable format)");
DEFINE_string(boost_log_level,"error","all, success, test_suite, message, warning, error, cpp_exception, system_error, fatal_error, nothing");
DEFINE_bool(boost_result_code,true,"Should the test suite return a result code or suppress it.");
DEFINE_int32(boost_random,0,"0 means don't randomize test order, 1 means do randomize tests, n>1 means use random seed n for test order");
DEFINE_string(boost_report_level,"confirm","no, confirm, short, details");
DEFINE_bool(boost_show_progress,false,"print progress information");


int main(int argc, char ** argv) { 
    std::string usage("./tests [--output_format=XML] [--log_level=success] ");
    google::SetUsageMessage(usage);

    jitpp::options opt(argc,argv,true);  

    for (int i=0;i<argc;++i) {
	printf("%s\n",argv[i]);
    }
    static const int buffer_size = 160;

    char * boost_argv[12];

    char buffer[buffer_size];
    buffer[buffer_size-1] = '\0';

    int boost_argc =0;
    if (FLAGS_boost_build_info) boost_argv[boost_argc++] = strdup("--build_info=yes");
    if (!FLAGS_boost_catch_system_errors) boost_argv[boost_argc++] = strdup("--catch_system_errors=no");
    snprintf(buffer,buffer_size-1, "--detect_memory_leak=%d",FLAGS_boost_detect_memory_leak);
    boost_argv[boost_argc++] = strdup(buffer);
    snprintf(buffer,buffer_size-1, "--log_format=%s",FLAGS_boost_log_format.c_str());
    boost_argv[boost_argc++] = strdup(buffer);
    snprintf(buffer,buffer_size-1, "--output_format=%s",FLAGS_boost_output_format.c_str());
    boost_argv[boost_argc++] = strdup(buffer);
    snprintf(buffer,buffer_size-1, "--report_format=%s",FLAGS_boost_report_format.c_str());
    boost_argv[boost_argc++] = strdup(buffer);
    snprintf(buffer,buffer_size-1,"--log_level=%s",FLAGS_boost_log_level.c_str());
    boost_argv[boost_argc++] = strdup(buffer);
    if (!FLAGS_boost_result_code) boost_argv[boost_argc++] = strdup("--result_code=no");
    snprintf(buffer,buffer_size-1,"--random=%d",FLAGS_boost_random);
    boost_argv[boost_argc++] = strdup(buffer);
    snprintf(buffer,buffer_size-1,"--report_level=%s",FLAGS_boost_report_level.c_str());
    boost_argv[boost_argc++] = strdup(buffer);
    if (FLAGS_boost_show_progress) boost_argv[boost_argc++] = strdup("--show_progress=yes");
    
    int result = boost::unit_test::unit_test_main(&init_unit_test, boost_argc, boost_argv);
    // for pedanticness
    // for (int i = 0; i < boost_argc; ++i ) 
    //    free(boost_argv[i]);
    return result;
}
