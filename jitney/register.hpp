#ifndef INCLUDED_JITNEY_ASSEMBLER
#define INCLUDED_JITNEY_ASSEMBLER

template <typename RegisterTraits>
class register_file { 
public:
	typedef typename RegisterTraits::reg64_type reg64_type;
	typedef typename RegisterTraits::reg32_type reg32_type;
	typedef typename RegisterTraits::reg16_type reg16_type;
	typedef typename RegisterTraits::reg8_type reg8_type;
	typedef typename RegisterTraits::regmmx_type regmmx_type;
	typedef typename RegisterTraits::regxmm_type regxmm_type;
	JITNEY_DECLARE_RXX(rax,eax,ax,al,ah); // general purpose accumulator
	JITNEY_DECLARE_RXX(rbx,ebx,bx,bl,bh); // general purpose index register 
	JITNEY_DECLARE_RXX(rcx,ecx,cx,cl,ch); // general purpose counter
	JITNEY_DECLARE_RXX(rdx,edx,dx,dl,dh); // general purpose high accumulator
	JITNEY_DECLARE_RSX(rsi,esi,si);
	JITNEY_DECLARE_RSX(rdi,edi,di);
	JITNEY_DECLARE_RSX(rsp,esp,sp);
	JITNEY_DECLARE_RSX(rbp,ebp,bp);
	JITNEY_DECLARE_RXX(r08,r08d,r08w,r08h,r08l);
	JITNEY_DECLARE_RXX(r09,r09d,r09w,r09h,r08l);
	JITNEY_DECLARE_RXX(r10,r10d,r10w,r10h,r10l);
	JITNEY_DECLARE_RXX(r11,r11d,r11w,r11h,r11l);
	JITNEY_DECLARE_RXX(r12,r12d,r12w,r12h,r12l);
	JITNEY_DECLARE_RXX(r13,r13d,r13w,r13h,r13l);
	JITNEY_DECLARE_RXX(r14,r14d,r14w,r14h,r1rl);
	JITNEY_DECLARE_RXX(r15,r15d,r15w,r15h,r15l);
	regmmx_type mm0,mm1,mm2,mm3,mm4,mm5,mm6,mm7;
	regxmm_type xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	regxmm_type xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15;
	regflags_type flags;
	regflag cf_flag, pf_flag, af_flag, zf_flag, sf_flag, tp_flag, if_flag, df_flag, of_flag, other_flag;
	regflags_type eflags;
	reg
	
	// TODO: flags in general && particular
	// TODO: flags

}

class reg { 
    int alias_super_count;
    int alias_sub_count;
    reg ** super_begin_, ** super_end_;
    reg ** sub_begin_, ** sub_end_;
};

class reg8 : public reg {
    reg_8l
    
};
class reg16 : public reg { 
    reg_16(xl, xh
};

#define JITNEY_DECLARE_RXX(rxx,exx,xx,xl,xh, numeric) \ 
	extern reg64 rxx; \
	extern reg32 exx; \
	extern reg16 xx; \
	extern reg8 xl; \
	extern reg8 xh;
	
#define JITNEY_DEFINE_RXX(rxx,exx,xx,xl,xh,numeric) \
	reg64 rxx = reg64(exx,xx,xl,xh); \
	reg32 exx = reg32(rxx,xx,xl,xh); \
	reg16 xx  = reg16(rxx,exx,xl,xh); \
	reg32 xl  = reg8(rxx,exx,xx); \
	reg32 xh  = reg8(rxx,exx,xx);


typedef enum { 
    rax = 0,
    rcx = 1,
    rdx = 2,
    rbx = 3
    rsp = 4
    rbp = 5
    rsi = 6
    rdi = 7
    sp = rsp,
    fp = rbp,
    r8 = 8,
    r9 = 9,
    r10 = 10,
    r11 = 11,
    r12 = 12,
    r13 = 13,
    r14 = 14,
    r15 = 15,
    xmm0 = 16, xmm1 = 17, xmm2 = 18, xmm3 = 19, xmm4 = 20, xmm5 = 21, xmm6 = 22,
    xmm
	





}
