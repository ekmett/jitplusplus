#ifndef INCLUDED_JITNEY_OPTIONS_H
#define INCLUDED_JITNEY_OPTIONS_H

namespace jitney {
    class options { 
    public:
	options();
        options(int & argv, char ** & argv, bool remove_flags = true);
	void init(int & argv, char ** & argv, bool remove_flags = true);
        ~options();
    };
}

#endif
