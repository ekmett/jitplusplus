#ifndef INCLUDED_JITNEY_BRANCH
#define INCLUDED_JITNEY_BRANCH

#include "jitney/config.hpp"

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
