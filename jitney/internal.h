#ifndef INCLUDED_JITNEY_INTERNAL_H
#define INCLUDED_JITNEY_INTERNAL_H

#include <jitney/config.h>	 

#ifdef HAVE_GLOG_LOGGING_H
#include <glog/logging.h>	 // logging used in cpp only, not headers
#endif 

#ifdef HAVE_GFLAGS_GFLAGS_H
#include <gflags/gflags.h>	 // cmd line support, cpp only, not used by public headers
#endif

namespace jitney { 
    ALWAYS_INLINE bool likely(bool x) {
#ifdef __GNUC__
	return static_cast<bool>(__builtin_expect(static_cast<long>(x),1L));
#else
	return x;
#endif
    }
    ALWAYS_INLINE bool unlikely(bool x) { 
#ifdef __GNUC__
	return static_cast<bool>(__builtin_expect(static_cast<long>(x),0L));
#else
	return x;
#endif
    }
} // namespace jitney

#endif
