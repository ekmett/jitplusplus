#ifndef INCLUDED_JITNEY_MEMORY_LINUX
#define INCLUDED_JITNEY_MEMORY_LINUX

#include <jitney/memory.hpp>

// defines jitney::memory::linux::proc_manager 

namespace jitney { namespace memory { namespace linux { 
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
}}} // namespace jitney::memory::linux

#endif
