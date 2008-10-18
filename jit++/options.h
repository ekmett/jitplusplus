#ifndef INCLUDED_JITPP_OPTIONS_H
#define INCLUDED_JITPP_OPTIONS_H

namespace jitpp {
    class options { 
    public:
	options();
        options(int & argv, char ** & argv, bool remove_flags = true);
	void init(int & argv, char ** & argv, bool remove_flags = true);
        ~options();
    };
}

#endif
