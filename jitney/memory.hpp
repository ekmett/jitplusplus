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
	namespace linux { 
	    namespace details { 
	        struct maps_entry {
	            void * start;
	            void * end;
	            flags flags;
	        };
	    
	        inline operator void * (const maps_entry & e) { return e.start; }
	    
	        typedef std::vector<const maps_entry> maps_vector;
	    
	        class table_parser {
	        private:
	            const char *p;
	        public:
	            table_parser(const char * p_) : p(p_) {}
	            inline bool eof() const { return p == NULL || *p == '\0' }
	            maps_entry parse();
	        };
	    }
	
	    // uses /proc/self/maps on linux
	    class proc_manager : public manager { 
	    public:
	        proc_manager();
	        ~proc_manager();
	        flags operator[](void * addr);
		void invalidate_maps();
		// get_maps() result is valid until next call to invalidate_maps
		const char * get_maps();
	    private:
	        char * buf_;
	        size_t buf_size_;
	        char   init_buf_[1];
	        details::maps_vector vector_;
	
		details::maps_entry * find_entry(void * p);
		void rebuild_maps();
	    };
        }
    }
} // namespace jitney::memory::linux

#endif
