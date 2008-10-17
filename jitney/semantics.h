#ifndef INCLUDED_JITNEY_SEMANTICS_H
#define INCLUDED_JITNEY_SEMANTICS_H

#ifdef HAVE_TBB_TBB_STDDEF_H
#include <tbb/tbb_stddef.h>
#endif

// TODO: if we have Intel's Thread Building Blocks, alias split to the split in tbb/tbb_stddef.h

namespace jitney { 

#ifdef HAVE_TBB_TBB_STDDEF_H
    using split = tbb::split;
#else 
    class split {};
#endif

    class move {};
}

#endif
