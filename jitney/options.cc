
#include <jitney/internal.h>
#include <jitney/options.h>

namespace {
    bool initialized = false;
}

namespace jitney { 
    options::options() {} 
    options::options(int & argc, char ** & argv, bool remove_flags) { 
	init(argc,argv,remove_flags);
    }
    void options::init(int & argc, char ** & argv, bool remove_flags) { 
	if (!initialized) { 
	    initialized = true;
	    google::ParseCommandLineFlags(&argc,&argv,true);
	    google::InitGoogleLogging(argv[0]);
	} else { 
	    LOG(DFATAL) << "Multiple initialization attempts";
	} 
    }  
    options::~options() {}
}
