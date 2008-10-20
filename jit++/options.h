#ifndef INCLUDED_JITPP_OPTIONS_H
#define INCLUDED_JITPP_OPTIONS_H

namespace jitpp {
    class options { 
    public:
	options();
        options(int & argc, char ** & argv, bool remove_flags = true);
	void init(int & argc, char ** & argv, bool remove_flags = true);
        ~options();
    };
}

#endif
