#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

typedef unsigned short instr_t;

// Integer types
typedef unsigned char int1;
typedef unsigned short int2;
typedef unsigned int int4;
typedef unsigned long int8;

typedef unsigned int addr;

// nop
const instr_t nop 				= 0x0000 + 0x0;

// alloc	const
const instr_t alloc_const 		= 0x0008 + 0x0;
// free		const
const instr_t free_const		= 0x0008 + 0x1;


// mov		const, {stp}, {stp}
const instr_t mov_stp_stp		= 0x0010 + 0x0;
// mov		const, {stp}, const
const instr_t mov_stp_const		= 0x0010 + 0x1;
// mov		const, [{stp}], {stp}
const instr_t mov_m_stp_stp		= 0x0010 + 0x2;
// mov		const, {stp}, [{stp}]
const instr_t mov_stp_m_stp		= 0x0010 + 0x3;


// add		const, {stp}, {stp}
const instr_t add_stp_stp		= 0x0020 + 0x0;
// add		const, {stp}, const
const instr_t add_stp_const		= 0x0020 + 0x1;

// sub		const, {stp}, {stp}
const instr_t sub_stp_stp		= 0x0030 + 0x0;
// sub		const, {stp}, const
const instr_t sub_stp_const		= 0x0030 + 0x1;

// mul		const, {stp}, {stp}
const instr_t mul_stp_stp		= 0x0040 + 0x0;
// mul		const, {stp}, const
const instr_t mul_stp_const		= 0x0040 + 0x1;

// div		const, {stp}, {stp}
const instr_t div_stp_stp		= 0x0050 + 0x0;
// div		const, {stp}, const
const instr_t div_stp_const		= 0x0050 + 0x1;

// mod		const, {stp}, {stp}
const instr_t mod_stp_stp		= 0x0060 + 0x0;
// mod		const, {stp}, const
const instr_t mod_stp_const		= 0x0060 + 0x1;


// not		const, {stp}
const instr_t not_stp			= 0x0070 + 0x0;

// and		const, {stp}, {stp}
const instr_t and_stp_stp		= 0x0078 + 0x0;
// and		const, {stp}, const
const instr_t and_stp_const		= 0x0078 + 0x1;

// or		const, {stp}, {stp}
const instr_t or_stp_stp		= 0x0088 + 0x0;
// or		const, {stp}, const
const instr_t or_stp_const		= 0x0088 + 0x1;

// xor		const, {stp}, {stp}
const instr_t xor_stp_stp		= 0x0098 + 0x0;
// xor		const, {stp}, const
const instr_t xor_stp_const		= 0x0098 + 0x1;


// if		const, {stp}, flow
const instr_t if_stp_flow		= 0x00A8 + 0x0;
// ifp		const, {stp}, flow
const instr_t ifp_stp_flow		= 0x00A8 + 0x1;

// call		{stp}
const instr_t call_stp		= 0x00B8 + 0x0;
// call		flow
const instr_t call_flow			= 0x00B8 + 0x1;

// ret
const instr_t ret_stp			= 0x00C0 + 0x0;
// hret
const instr_t hret_stp			= 0x00C0 + 0x1;

// jmp		flow
const instr_t jmp_flow			= 0x00D0 + 0x0;

// out		const
const instr_t out_const			= 0x00D8 + 0x0;
// out		{stp}
const instr_t out_stp			= 0x00D8 + 0x1;

// regin	const, flow
const instr_t regin_const_stp	= 0x00E0 + 0x0;
// uregin	const
const instr_t uregin_const		= 0x00E8 + 0x0;

// halt
const instr_t halt				= 0x00F0 + 0x0;

// setcont {stp}
const instr_t setcont_stp		= 0x0100 + 0x0;
// setcont [{stp}]
const instr_t setcont_m_stp		= 0x0100 + 0x1;

// getcont {stp}
const instr_t getcont_stp		= 0x0108 + 0x0;
// getcont [{stp}]
const instr_t getcont_m_stp		= 0x0108 + 0x1;

#endif
