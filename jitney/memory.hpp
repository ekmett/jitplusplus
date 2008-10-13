#ifndef INCLUDED_JITNEY_MEMORY
#define INCLUDED_JITNEY_MEMORY

// TODO: this has a number of internal details that can be pulled out and made into a class
// we should template off of that rather than code against these methods directly
// that would permit a bibop to enforce these constraints on more memory.
// also could generational gc perhaps give a better approximation of 'const' member lookups?

namespace jitney { 
    namespace memory {
        typedef int flags;
	namespace flag { 
    	    static const Flags empty = 0;
    	    static const Flags read = 1;
    	    static const Flags write = 2;
            static const Flags execute = 4;
	}

	class manager { 
	public:
	    virtual flags operator[](void * addr);
            bool is_const(void * addr) { 
		return (*this)[addr] & write_flag == 0;
	    }
	    bool is_executable(void * addr) {
		return (*this)[addr] & execute_flag != 0;
	    }
	};

	class trivial_manager { 
	public:
	    trivial_manager() {}
	    trivial_manager(const trivial_manager &) {}
	    trivial_manager & operator=(const trivial_manager &) { return *this; }
	    flags operator[](void * addr) { return flag::read | flag::write | flag::execute } 
	};
    }
}

#endif
