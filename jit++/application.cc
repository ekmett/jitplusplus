#include <jit++/common.h>
#include <jit++/application.h>

namespace {
    bool initialized = false;
}

namespace jitpp { 
    application::application() {} 
    application::application(int & argc, char ** & argv, bool remove_flags) { 
	init(argc,argv,remove_flags);
    }
    void application::init(int & argc, char ** & argv, bool remove_flags) { 
	if (!initialized) { 
	    initialized = true;
	    google::ParseCommandLineFlags(&argc,&argv,true);
	    google::InitGoogleLogging(argv[0]);
	} else { 
	    LOG(DFATAL) << "Multiple initialization attempts";
	} 
    }  
    application::~application() {}
}
