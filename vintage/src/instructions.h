#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

typedef unsigned char instr_t;

// Integer types
typedef signed char int1;
typedef signed short int2;
typedef signed int int4;
typedef signed long int8;

// nop
const instr_t nop 				= 0;

// alloc	const
const instr_t alloc_const 		= 8 + 0;
// free		const
const instr_t free_const		= 8 + 1;


// mov		const, {stp}, {stp}
const instr_t mov_stp_stp		= 16 + 0;
// mov		const, {stp}, const
const instr_t mov_stp_const		= 16 + 1;
// mov		const, [{stp}], {stp}
const instr_t mov_m_stp_stp		= 16 + 2;
// mov		const, {stp}, [{stp}]
const instr_t mov_stp_m_stp		= 16 + 3;


// add		const, {stp}, {stp}
const instr_t add_stp_stp		= 24 + 0;
// sub		const, {stp}, {stp}
const instr_t sub_stp_stp		= 24 + 1;
// mul		const, {stp}, {stp}
const instr_t mul_stp_stp		= 24 + 2;
// div		const, {stp}, {stp}
const instr_t div_stp_stp		= 24 + 3;
// mod		const, {stp}, {stp}
const instr_t mod_stp_stp		= 24 + 4;
// add		const, {stp}, const
const instr_t add_stp_const		= 24 + 5;
// sub		const, {stp}, const
const instr_t sub_stp_const		= 24 + 6;
// mul		const, {stp}, const
const instr_t mul_stp_const		= 24 + 7;
// div		const, {stp}, const
const instr_t div_stp_const		= 24 + 8;
// mod		const, {stp}, const
const instr_t mod_stp_const		= 24 + 9;


// not		const, {stp}
const instr_t not_stp			= 48 + 0;
// and		const, {stp}, {stp}
const instr_t and_stp_stp		= 48 + 1;
// or		const, {stp}, {stp}
const instr_t or_stp_stp		= 48 + 2;
// xor		const, {stp}, {stp}
const instr_t xor_stp_stp		= 48 + 3;
// and		const, {stp}, const
const instr_t and_stp_const		= 48 + 4;
// or		const, {stp}, const
const instr_t or_stp_const		= 48 + 5;
// xor		const, {stp}, const
const instr_t xor_stp_const		= 48 + 6;


// if		const, {stp}, flow
const instr_t if_stp_flow		= 72 + 0;
// ifp		const, {stp}, flow
const instr_t ifp_stp_flow		= 72 + 1;
// call		{stp}
const instr_t call_m_stp		= 72 + 2;
// call		flow
const instr_t call_flow			= 72 + 3;
// ret
const instr_t ret_stp			= 72 + 4;
// hret
const instr_t hret_stp			= 72 + 5;
// jmp		flow
const instr_t jmp_flow			= 72 + 6;
// out		const
const instr_t out_const			= 72 + 7;

// regin	const, flow
const instr_t regin_const_flow	= 72 + 8;
// uregin	const
const instr_t uregin_const		= 72 + 9;

// halt
const instr_t halt				= 92 + 0;


#endif
