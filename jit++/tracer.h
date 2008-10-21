#ifndef INCLUDED_JITPP_TRACER_H
#define INCLUDED_JITPP_TRACER_H

#include <sys/types.h>
#include <stdint.h>        // int64_t, etc

namespace jitpp { 
    union mmx_t {
	long double ld;
	float   f[2];
	double  df[1];
	int64_t q[1];
 	int32_t d[2];
	int16_t w[4];
	int8_t  b[8],reserved[8];
    } __attribute__((aligned(8)));

    union xmm_t { 
	double df[2];
	float f[4];
	int64_t q[2];
	int32_t d[4];
	int16_t w[8];
	int8_t b[16];
    } __attribute__((aligned(16)));

    // fixed structure used by tracer_start.S
    class tracer {
    protected:
	// vtable 		    /* 0 */
	virtual void run() = 0;     /* 0; 0 */

	int64_t m_reserved_0;       /* 8 */
        int64_t m_reg[16];          /* 16 */
	int64_t m_rip;              /* 144 */
	int64_t m_rflags;           /* 152 */
	// fxsave format start
	int16_t m_fx_fcw;           /* 160 */
	int16_t m_fx_fsw;           
	int16_t m_fx_ftw;           
	int16_t m_fx_fop;           
	int32_t m_fx_ip;            
	int16_t m_fx_cs;            
	int16_t m_fx_reserved_1;    
	int32_t m_fx_dp;            
	int16_t m_fx_ds;            
	int16_t m_fx_reserved_2;    
	int32_t m_fx_mxcsr;         
	int32_t m_fx_reserved_3;    
        mmx_t m_fx_mmx[8];          
	xmm_t m_fx_xmm[16];         
	int64_t m_fx_reserved_4[12];
	// end fxsave format
	uint8_t * m_stack;	     /* 572 */
	size_t m_stack_size;	     /* 580 */
	/* end of fixed structure */

    public:
	tracer(size_t stack_size = default_stack_size());
	~tracer();

        static size_t default_stack_size();

	__attribute__((nothrow)) void start();

    private:
	// hide copy and assignment
	tracer(const tracer & peer);
	tracer & operator=(const tracer & peer);
    } __attribute__((aligned(16)));

} // jitpp

#endif
