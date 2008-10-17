#ifndef INCLUDED_JITNEY_MUTEX_H
#define INCLUDED_JITNEY_MUTEX_H

#include <pthread.h>	      // pthread_rwlock_t
#include <algorithm>          // std::swap
#include <jitney/semantics.h> // move

namespace jitney { 

    class native_rw_mutex { 
    public: 
        native_rw_mutex();
        ~native_rw_mutex();
	operator pthread_rwlock_t & () { return m_self; } 
	operator const pthread_rwlock_t & () const { return m_self; } 
	static const bool is_rw_mutex = true;
	static const bool is_fair_mutex = false;
	static const bool is_recursive_mutex = false;
	void rdlock();
	void wrlock();
	void unlock();
    private: 
	pthread_rwlock_t m_self;

	// disable copy & assignment constructors
	native_rw_mutex(const native_rw_mutex &);
	void operator=(const native_rw_mutex &);

    public:
	class scoped_lock { 
	public:
	    friend void std::swap<scoped_lock>(scoped_lock &, scoped_lock &);
	    scoped_lock();
	    scoped_lock(scoped_lock & peer, move);
	    scoped_lock(pthread_rwlock_t & lock, bool write = true);
	    ~scoped_lock();

	    void acquire(pthread_rwlock_t & lock, bool write = true);
	    void release();
	private:
	    pthread_rwlock_t * m_lock;

	    // disable copy & assignment constructors
	    scoped_lock(const scoped_lock &);
	    void operator=(const scoped_lock &);
	};
    };
}

namespace std { 
    template<> inline void swap(
	jitney::native_rw_mutex::scoped_lock & a, 
	jitney::native_rw_mutex::scoped_lock & b
    ) { 
	swap(a.m_lock,b.m_lock);
    }
}

#endif
