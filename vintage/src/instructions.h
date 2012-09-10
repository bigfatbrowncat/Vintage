#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

typedef unsigned short instr_t;

// Integer types
typedef signed char int1;
typedef signed short int2;
typedef signed int int4;
typedef signed long int8;

typedef unsigned int addr;

// *** 0x000 group -- specials ***

// nop
const instr_t nop 				= 0x0000 + 0x0;
// halt
const instr_t halt				= 0x0000 + 0x1;

// *** 0x100 group -- memory operations ***

// alloc	const
const instr_t alloc_const 		= 0x0100 + 0x1;
// free		const
const instr_t free_const		= 0x0100 + 0x2;
// mov		const, {stp}, {stp}

const instr_t mov_stp_stp		= 0x0100 + 0x3;
// mov		const, {stp}, const
const instr_t mov_stp_const		= 0x0100 + 0x4;
// mov		const, [{stp}], {stp}
const instr_t mov_m_stp_stp		= 0x0100 + 0x5;
// mov		const, {stp}, [{stp}]
const instr_t mov_stp_m_stp		= 0x0100 + 0x6;


// *** 0x200 group -- arithmetic operations ***

// add		const, {stp}, {stp}
const instr_t add_stp_stp		= 0x0200 + 0x0;
// add		const, {stp}, const
const instr_t add_stp_const		= 0x0200 + 0x1;

// sub		const, {stp}, {stp}
const instr_t sub_stp_stp		= 0x0200 + 0x2;
// sub		const, {stp}, const
const instr_t sub_stp_const		= 0x0200 + 0x3;

// mul		const, {stp}, {stp}
const instr_t mul_stp_stp		= 0x0200 + 0x4;
// mul		const, {stp}, const
const instr_t mul_stp_const		= 0x0200 + 0x5;

// div		const, {stp}, {stp}
const instr_t div_stp_stp		= 0x0200 + 0x6;
// div		const, {stp}, const
const instr_t div_stp_const		= 0x0200 + 0x7;

// mod		const, {stp}, {stp}
const instr_t mod_stp_stp		= 0x0200 + 0x8;
// mod		const, {stp}, const
const instr_t mod_stp_const		= 0x0200 + 0x9;


// *** 0x300 group -- logical operations ***

// not		const, {stp}
const instr_t not_stp			= 0x0300 + 0x0;

// and		const, {stp}, {stp}
const instr_t and_stp_stp		= 0x0300 + 0x1;
// and		const, {stp}, const
const instr_t and_stp_const		= 0x0300 + 0x2;

// or		const, {stp}, {stp}
const instr_t or_stp_stp		= 0x0300 + 0x3;
// or		const, {stp}, const
const instr_t or_stp_const		= 0x0300 + 0x4;

// xor		const, {stp}, {stp}
const instr_t xor_stp_stp		= 0x0300 + 0x5;
// xor		const, {stp}, const
const instr_t xor_stp_const		= 0x0300 + 0x6;

// gr		const, {stp}, {stp}
const instr_t gr_stp_stp		= 0x0300 + 0x7;
// gr		const, {stp}, const
const instr_t gr_stp_const		= 0x0300 + 0x8;

// greq		const, {stp}, {stp}
const instr_t greq_stp_stp		= 0x0300 + 0x9;
// greq		const, {stp}, const
const instr_t greq_stp_const	= 0x0300 + 0x10;

// lw		const, {stp}, {stp}
const instr_t lw_stp_stp		= 0x0300 + 0x11;
// lw		const, {stp}, const
const instr_t lw_stp_const		= 0x0300 + 0x12;

// lweq		const, {stp}, {stp}
const instr_t lweq_stp_stp		= 0x0300 + 0x13;
// lweq		const, {stp}, const
const instr_t lweq_stp_const	= 0x0300 + 0x14;

// eq		const, {stp}, {stp}
const instr_t eq_stp_stp		= 0x0300 + 0x15;
// eq		const, {stp}, const
const instr_t eq_stp_const		= 0x0300 + 0x16;

// neq		const, {stp}, {stp}
const instr_t neq_stp_stp		= 0x0300 + 0x17;
// neq		const, {stp}, const
const instr_t neq_stp_const		= 0x0300 + 0x18;


// *** 0x400 group -- jumps ***

// jz		const, {stp}, flow
const instr_t jz_stp_flow		= 0x0400 + 0x0;
// jnz		const, {stp}, flow
const instr_t jnz_stp_flow		= 0x0400 + 0x1;
// jp		const, {stp}, flow
const instr_t jp_stp_flow		= 0x0400 + 0x2;
// jnp		const, {stp}, flow
const instr_t jnp_stp_flow		= 0x0400 + 0x3;
// jn		const, {stp}, flow
const instr_t jn_stp_flow		= 0x0400 + 0x4;
// jnn		const, {stp}, flow
const instr_t jnn_stp_flow		= 0x0400 + 0x5;
// jmp		flow
const instr_t jmp_flow			= 0x0400 + 0x6;
// call		{stp}
const instr_t call_stp			= 0x0400 + 0x7;
// call		flow
const instr_t call_flow			= 0x0400 + 0x8;
// ret
const instr_t ret_stp			= 0x0400 + 0x9;

// *** 0x500 group -- contexts ***

// hret
const instr_t hret_stp			= 0x0500 + 0x0;
// setcont {stp}
const instr_t setcont_stp		= 0x0500 + 0x1;
// setcont [{stp}]
const instr_t setcont_m_stp		= 0x0500 + 0x2;
// getcont {stp}
const instr_t getcont_stp		= 0x0500 + 0x3;
// getcont [{stp}]
const instr_t getcont_m_stp		= 0x0500 + 0x4;

// *** 0x600 group -- input/output ***

// out		const
const instr_t out_const			= 0x0600 + 0x0;
// out		{stp}
const instr_t out_stp			= 0x0600 + 0x1;

#endif
