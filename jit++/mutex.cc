#include <pthread.h>
#include <jit++/common.h> // CHECK_*
#include <jit++/semantics.h>
#include <jit++/mutex.h>

namespace jitpp { 

    native_rw_mutex::native_rw_mutex() { 
	pthread_rwlock_init(&m_self,0); 
    } 

    native_rw_mutex::~native_rw_mutex() { 
	pthread_rwlock_destroy(&m_self); 
    }

    void native_rw_mutex::rdlock() { 
	CHECK_EQ(pthread_rwlock_rdlock(&m_self),0) << "read lock failed";
    }
    void native_rw_mutex::wrlock() { 
	CHECK_EQ(pthread_rwlock_wrlock(&m_self),0) << "write lock failed";
    }
    void native_rw_mutex::unlock() { 
	pthread_rwlock_unlock(&m_self); 
    }

    native_rw_mutex::scoped_lock::scoped_lock() : m_lock(0) {}

    native_rw_mutex::scoped_lock::scoped_lock(pthread_rwlock_t & lock, bool write) : m_lock(&lock) {
	if (write) 
	    CHECK_EQ(pthread_rwlock_wrlock(m_lock),0) << "scoped write lock failed";
	else 
	    CHECK_EQ(pthread_rwlock_rdlock(m_lock),0) << "scoped read lock failed";
    }

    native_rw_mutex::scoped_lock::scoped_lock(scoped_lock & peer, move) : m_lock(peer.m_lock) { 
	peer.m_lock = 0;
    }
	
    native_rw_mutex::scoped_lock::~scoped_lock() { 
	if (m_lock != 0) pthread_rwlock_unlock(m_lock);
    }

    void native_rw_mutex::scoped_lock::acquire(pthread_rwlock_t & lock, bool write) {
	if (m_lock != 0) pthread_rwlock_unlock(m_lock);

	m_lock = &lock;

	if (write) CHECK_EQ(pthread_rwlock_wrlock(m_lock),0) << "scoped write lock acquire failed";
	else CHECK_EQ(pthread_rwlock_rdlock(m_lock),0) << "scoped read lock acquire failed";
    }

    void native_rw_mutex::scoped_lock::release() { 
	if (m_lock != 0) pthread_rwlock_unlock(m_lock);
	m_lock = 0;
    }
  
} // namespace jitpp
