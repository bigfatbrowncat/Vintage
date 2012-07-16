#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

typedef unsigned short instr_t;

// Integer types
typedef signed char int1;
typedef signed short int2;
typedef signed int int4;
typedef signed long int8;

typedef unsigned int addr;

// nop
const instr_t nop 				= 0x00 + 0;

// alloc	const
const instr_t alloc_const 		= 0x08 + 0x0;
// free		const
const instr_t free_const		= 0x08 + 0x1;


// mov		const, {stp}, {stp}
const instr_t mov_stp_stp		= 0x10 + 0x0;
// mov		const, {stp}, const
const instr_t mov_stp_const		= 0x10 + 0x1;
// mov		const, [{stp}], {stp}
const instr_t mov_m_stp_stp		= 0x10 + 0x2;
// mov		const, {stp}, [{stp}]
const instr_t mov_stp_m_stp		= 0x10 + 0x3;


// add		const, {stp}, {stp}
const instr_t add_stp_stp		= 0x20 + 0x0;
// add		const, {stp}, const
const instr_t add_stp_const		= 0x20 + 0x1;

// sub		const, {stp}, {stp}
const instr_t sub_stp_stp		= 0x30 + 0x0;
// sub		const, {stp}, const
const instr_t sub_stp_const		= 0x30 + 0x1;

// mul		const, {stp}, {stp}
const instr_t mul_stp_stp		= 0x40 + 0x0;
// mul		const, {stp}, const
const instr_t mul_stp_const		= 0x40 + 0x1;

// div		const, {stp}, {stp}
const instr_t div_stp_stp		= 0x50 + 0x0;
// div		const, {stp}, const
const instr_t div_stp_const		= 0x50 + 0x1;

// mod		const, {stp}, {stp}
const instr_t mod_stp_stp		= 0x60 + 0x0;
// mod		const, {stp}, const
const instr_t mod_stp_const		= 0x60 + 0x1;


// not		const, {stp}
const instr_t not_stp			= 0x70 + 0x0;

// and		const, {stp}, {stp}
const instr_t and_stp_stp		= 0x78 + 0x0;
// and		const, {stp}, const
const instr_t and_stp_const		= 0x78 + 0x1;

// or		const, {stp}, {stp}
const instr_t or_stp_stp		= 0x88 + 0x0;
// or		const, {stp}, const
const instr_t or_stp_const		= 0x88 + 0x1;

// xor		const, {stp}, {stp}
const instr_t xor_stp_stp		= 0x98 + 0x0;
// xor		const, {stp}, const
const instr_t xor_stp_const		= 0x98 + 0x1;


// if		const, {stp}, flow
const instr_t if_stp_flow		= 0xA8 + 0x0;
// ifp		const, {stp}, flow
const instr_t ifp_stp_flow		= 0xA8 + 0x1;

// call		{stp}
const instr_t call_m_stp		= 0xB8 + 0x0;
// call		flow
const instr_t call_flow			= 0xB8 + 0x1;

// ret
const instr_t ret_stp			= 0xC0 + 0x0;
// hret
const instr_t hret_stp			= 0xC0 + 0x1;

// jmp		flow
const instr_t jmp_flow			= 0xD0 + 0x0;

// out		const
const instr_t out_const			= 0xD8 + 0x0;

// regin	const, flow
const instr_t regin_const_flow	= 0xE0 + 0x0;
// uregin	const
const instr_t uregin_const		= 0xE0 + 0x1;

// halt
const instr_t halt				= 0xF0 + 0x0;


#endif
