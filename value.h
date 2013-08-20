/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

// rationale: 
// XMM/FPU values should be passed just like any other values, at the same places

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "strbuf.h"

#ifdef  __cplusplus
extern "C" {
#endif

// here should be for both 32 and 64-bit: type + (reg|ptr) (~5 and ~9 bytes)

union value_u
{
    uint64_t v;
    double d;
    uint8_t xmm[16]; // FIXME: ptr
};

enum value_t
{
    V_INVALID=0,
    V_BYTE,
    V_WORD,
    V_DWORD,
    V_QWORD,
    V_DOUBLE,
    V_XMM
};

typedef struct s_Value_t
{
    enum value_t t;
    union value_u u;
} s_Value;

int get_4th_bit(s_Value *t);
int is_zero(s_Value *i);
void create_Value(enum value_t t, uint64_t v, s_Value* out);
void create_Value_of_type (s_Value *t, uint64_t v, s_Value *out);
int get_most_significant_bit(s_Value *i);
int get_2nd_most_significant_bit(s_Value *i);
uint64_t get_as_QWORD (s_Value *i);
void create_XMM_Value(uint8_t * xmm, s_Value *out);
void create_double_Value(double d, s_Value *out);
uint8_t* get_xmm (s_Value* i);
uint64_t get_as_64(s_Value* i);
uint32_t get_as_32(s_Value* i);
uint16_t get_as_16(s_Value* i);
uint8_t get_as_8(s_Value* i);
double get_as_double (s_Value *i);
int compare_Values (s_Value *i1, s_Value *i2); // -1 - i1<i2, 0 - i1==i2, 1 - i1>i2
enum value_t bit_width_to_value_t(int i);
void create_Value_as_sign_extended(s_Value *i, enum value_t type_of_result, s_Value *out);
void decrement_Value(s_Value *v);
void Value_sign_extended_shift_right (s_Value *op1, s_Value *op2, s_Value *out);
void Value_free(s_Value *v);
void copy_Value (s_Value *dst, s_Value *src);
void Value_to_hex_str (s_Value *v, strbuf* out, bool is_asm);
REG get_as_REG(s_Value* i);

#ifdef  __cplusplus
}
#endif

/* vim: set expandtab ts=4 sw=4 : */
