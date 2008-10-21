#ifndef INCLUDED_JITPP_TRACER_H
#define INCLUDED_JITPP_TRACER_H

#include <sys/types.h>
#include <stdint.h>        // int64_t, etc
#include <jit++/context.h> // context

namespace jitpp { 

    class tracer : protected context { 
    private:
	// we defer the syscall for these until they are used.
	mutable int64_t m_fs_base; 
	mutable int64_t m_gs_base; 

	mutable bool m_fs_base_known;
	mutable bool m_gs_base_known;

	// these are for the interpreter, not the code being traced!
	uint8_t * m_stack;
	size_t m_stack_size;

    public:
	tracer(size_t stack_size = default_stack_size());
	~tracer();

        static size_t default_stack_size();

	void start();

    protected:
	int64_t fs_base() const;
	int64_t gs_base() const;
	int32_t eip() const { return static_cast<int32_t>(rip()); } 

	virtual void run() = 0;

    private:
	static void run_tracer(context & t);

	tracer(const tracer & peer);
	tracer & operator=(const tracer & peer);
    } __attribute__((aligned(16)));

} // jitpp

#endif
