#ifndef INCLUDED_JITPP_MEMORY_H
#define INCLUDED_JITPP_MEMORY_H

#include <vector>         // std::vector
#include <pthread.h>      // pthread_rwlock_t
#include <jit++/mutex.h>  // jitpp::native_rw_mutex
// TODO: this has a number of internal details that can be pulled out and made into a class
// we should template off of that rather than code against these methods directly
// that would permit a bibop to enforce these constraints on more memory.
// also could generational gc perhaps give a better approximation of 'const' member lookups?

namespace jitpp { 
    typedef int memory_permissions;

    namespace memory_permission {
        static const memory_permissions empty = 0;
        static const memory_permissions read = 1;
        static const memory_permissions write = 2;
        static const memory_permissions execute = 4;
    }
 
    // forward declaration
    class memory_permission_model;

    template <typename T = memory_permission_model> class memory_permission_reader { 
    private:
        T & m_model;
    public:
        memory_permission_reader(T & model) : m_model(&model) { 
            m_model->rdlock();
        }
        memory_permission_reader(const memory_permission_reader & peer) : m_model(peer.m_model) { 
            m_model->rdlock();
        }
        ~memory_permission_reader() { 
            m_model->unlock(); 
        }
        inline virtual memory_permissions operator[](const void * addr) { 
            return *(m_model)[addr]; 
        }
        inline bool is_const(const void * addr) { 
            return (*m_model)[addr] & memory_permission::write == 0; 
        }
        inline bool is_executable(const void * addr) { 
            return (*m_model)[addr] & memory_permission::execute != 0; 
        }
    };


    class memory_permission_model { 
    protected:
        virtual memory_permissions operator[](const void * addr) = 0;
        virtual void rdlock() = 0;
        virtual void unlock() = 0;
    public:
        typedef memory_permission_reader<memory_permission_model> reader;
        friend class memory_permission_reader<memory_permission_model>;
    };

    // a trivial model that doesn't let you assume anything about memory usage
    class trivial_memory_permission_model : public memory_permission_model {
    public:
        typedef memory_permission_reader<trivial_memory_permission_model> reader;
        friend class memory_permission_reader<trivial_memory_permission_model>;

        trivial_memory_permission_model() {}
        trivial_memory_permission_model(const trivial_memory_permission_model &) {}
        trivial_memory_permission_model & operator=(const trivial_memory_permission_model &) { return *this; }
    protected:
        inline void rdlock() {}
        inline void unlock() {}
        inline virtual memory_permissions operator[](const void * addr) { 
            return memory_permission::read | memory_permission::write | memory_permission::execute;
        } 
    };
            
    // this engages in a pleasant fiction. it believes that if you can't mutate a range of memory then no one else can
    // however, in practice this is usually the case. A counter example would be that you have two mmaps of the same region of memory
    // one read-only. that doesn't prevent this from being useful it just says you may need to layer a better domain specific model on top
    // to accomodate those local strangenesses.
    class native_memory_permission_model : public memory_permission_model { 
    public:
        typedef memory_permission_reader<native_memory_permission_model> reader;
        friend class memory_permission_reader<native_memory_permission_model>;

        // POD type
        struct range {
            const void * start;
            const void * end;
            memory_permissions flags;
            inline operator const void * () const { return start; }
        };

        // you CAN create your own, but there is a singleton instance() 
        // since this is inherently sharable
        native_memory_permission_model();
        ~native_memory_permission_model();

        // call after something hinky is done to memory to ensure accurate permissions
        void clear();

        // ack! a singleton
        static native_memory_permission_model & instance();
    private:
	typedef native_rw_mutex mutex;
	typedef std::vector<range> vector;
        
	mutex m_mutex;
        vector m_ranges;

        range * find_entry(const void *);
        void rebuild();

    protected:
        void rdlock() { m_mutex.rdlock(); } 
        void unlock() { m_mutex.wrlock(); } 
        virtual memory_permissions operator[](const void *);
        
    };
} // namespace jitpp

#endif
