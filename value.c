/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#include <assert.h>
#include "value.h"
//#include "printf-utils.h"
#include "dmalloc.h"
#include "strbuf.h"

static uint64_t get_type_mask (enum value_t t)
{
    switch (t)
    {
        case V_QWORD:
            return 0xFFFFFFFFFFFFFFFF;
        case V_DWORD:
            return 0xFFFFFFFF;
        case V_WORD:
            return 0xFFFF;
        case V_BYTE:
            return 0xFF;
        default:
            assert(0);
    };
    return 0; // make compiler happy
};

void create_Value(enum value_t t, uint64_t v, s_Value *out)
{
    out->t=t;
    assert (t!=V_XMM && t!=V_DOUBLE);
    out->u.v=v & get_type_mask(t);
    //L (__FUNCTION__"() rt=0x%p\n", rt);
};

void create_Value_of_type (s_Value *t, uint64_t v, s_Value *out)
{
    create_Value (t->t, v, out);
};

int get_4th_bit(s_Value *t)
{
    assert (t->t!=V_XMM && t!=V_DOUBLE);
    return (t->u.v >> 4)&1;
};

int is_zero(s_Value *i)
{
    // v is always should be zero padded!
    return i->u.v ? 0 : 1;
};

int get_most_significant_bit(s_Value *i)
{
    switch (i->t)
    {
        case V_QWORD:
            return (i->u.v & 0x8000000000000000) ? 1 : 0;
        case V_DWORD:
            return (i->u.v & 0x80000000) ? 1 : 0;
        case V_WORD:
            return (i->u.v & 0x8000) ? 1 : 0;
        case V_BYTE:
            return (i->u.v & 0x80) ? 1 : 0;
        default:
            assert(0);
            return 0; // make compiler happy
    };
};

int get_2nd_most_significant_bit(s_Value *i)
{
    switch (i->t)
    {
        case V_QWORD:
            return (i->u.v & 0x4000000000000000) ? 1 : 0;
        case V_DWORD:
            return (i->u.v & 0x40000000) ? 1 : 0;
        case V_WORD:
            return (i->u.v & 0x4000) ? 1 : 0;
        case V_BYTE:
            return (i->u.v & 0x40) ? 1 : 0;
        default:
            assert(0);
            return 0; // make compiler happy
    };
};

uint64_t get_as_QWORD (s_Value *i)
{
    assert (i->t!=V_INVALID && i->t!=V_XMM && i->t!=V_DOUBLE);
    return i->u.v;
}

double get_as_double (s_Value *i)
{
    assert (i->t==V_DOUBLE);
    return i->u.d;
}

void create_XMM_Value(uint8_t * xmm, s_Value *out)
{
    out->t=V_XMM;
    memcpy (out->u.xmm, xmm, 16);
}

void create_double_Value(double d, s_Value *out)
{
    out->t=V_DOUBLE;
    out->u.d=d;
}

uint8_t* get_xmm (s_Value* i)
{
    assert (i->t==V_XMM);
    return &i->u.xmm[0];
};

uint64_t get_as_64(s_Value* i)
{
    assert (i->t!=V_INVALID && i->t!=V_XMM && i->t!=V_DOUBLE);
    return i->u.v;
};

uint32_t get_as_32(s_Value* i)
{
    assert (i->t!=V_INVALID && i->t!=V_XMM && i->t!=V_DOUBLE);
    if (i->u.v>UINT32_MAX)
        fprintf (stderr, "%s() warning: u.v is bigger than uint32_t\n", __FUNCTION__);
    return (uint32_t)(i->u.v & get_type_mask (i->t));
};

uint16_t get_as_16(s_Value* i)
{
    assert (i->t!=V_INVALID && i->t!=V_XMM && i->t!=V_DOUBLE);
    if (i->u.v>UINT16_MAX)
        fprintf (stderr, "%s() warning: u.v is bigger than uint16_t\n", __FUNCTION__);
    return (uint16_t)(i->u.v & get_type_mask (i->t));
};

uint8_t get_as_8(s_Value* i)
{
    assert (i->t!=V_INVALID && i->t!=V_XMM && i->t!=V_DOUBLE);
    if (i->u.v>UINT8_MAX)
        fprintf (stderr, "%s() warning: u.v is bigger than uint8_t\n", __FUNCTION__);
    return (uint8_t)(i->u.v & get_type_mask (i->t));
};

int compare_Values (s_Value *i1, s_Value *i2) // -1 - i1<i2, 0 - i1==i2, 1 - i1>i2
{
    assert (i1->t==i2->t);

    if ((i1->u.v & get_type_mask(i1->t)) < (i2->u.v & get_type_mask (i2->t)))
        return -1;
    if ((i1->u.v & get_type_mask (i1->t)) > (i2->u.v & get_type_mask (i2->t)))
        return 1;
    return 0;
};

enum value_t bit_width_to_value_t(int i)
{
    switch (i)
    {
        case 8: 
            return V_BYTE;
        case 16: 
            return V_WORD;
        case 32: 
            return V_DWORD;
        case 64: 
            return V_QWORD;
        case 128: 
            return V_XMM;
        default:
            assert (0);
    };
    return V_INVALID; // make compiler happy
};

static void dump_Value (s_Value *i)
{
    switch (i->t)
    {
        case V_INVALID:
            printf ("INVALID");
            break;
        case V_QWORD:
            printf ("QWORD: 0x%I64x", i->u.v);
            break;
        case V_DWORD:
            printf ("DWORD: 0x%08X", (tetrabyte)i->u.v);
            break;
        case V_WORD:
            printf ("WORD: 0x%04X", (wyde)i->u.v);
            break;
        case V_BYTE:
            printf ("BYTE: 0x%02X", (byte)i->u.v);
            break;
        default:
            assert(0);
    };
};

void create_Value_as_sign_extended(s_Value *i, enum value_t type_of_result, s_Value *out)
{
    // i can be byte, word, dword
    // type_of_result can be word, dword, qword

    //L (__FUNCTION__"() i="); dump_Value(i); L("\n");

    if (get_most_significant_bit (i))
    {
        //L("get_most_significant_bit()!=0\n");
        uint64_t to_be_set=(UINT64_MAX & (~get_type_mask(i->t))) | i->u.v;
        //L ("to_be_set=0x%I64x\n", to_be_set);
        create_Value (type_of_result, to_be_set, out); // that function trim high unwanted bits automatically
    };

    create_Value (type_of_result, i->u.v, out);
};

void decrement_Value(s_Value *i)
{
    assert (i->t!=V_INVALID && i->t!=V_XMM && i->t!=V_DOUBLE);
    i->u.v--;
    i->u.v &= get_type_mask(i->t); // trim -1 value (high bits of 64-bit v should be always zero!)
};

void Value_sign_extended_shift_right (s_Value *op1, s_Value *op2, s_Value *out)
{
    switch (op1->t)
    {
        case V_QWORD:
            create_Value (V_QWORD, (uint64_t)(((int64_t)op1->u.v) >> op2->u.v), out);
            break;
        case V_DWORD:
            create_Value (V_DWORD, (uint32_t)(((int32_t)op1->u.v) >> op2->u.v), out); 
            break;
        case V_WORD:
            create_Value (V_WORD, (uint16_t)(((int16_t)op1->u.v) >> op2->u.v), out);
            break;
        case V_BYTE:
            create_Value (V_BYTE, (uint8_t)(((int8_t)op1->u.v) >> op2->u.v), out);
            break;
        default:
            assert(0);
            break;
    };
};

void Value_free(s_Value *v)
{
    // here will be free pointers to XMM, etc...
    //L (__FUNCTION__"() v=0x%p\n", v);
};

void copy_Value (s_Value *dst, s_Value *src)
{
    // when no pointers in structure, it is possible yet
    memcpy (dst, src, sizeof(s_Value));
};

void Value_to_hex_str (s_Value *val, strbuf* out, bool is_asm)
{
    uint64_t v=val->u.v;
    
    if (is_asm)
    {
        strbuf_asmhex (out, v);
        return;
    };

#if 0
    if (v<10)
    {
        strbuf_addf(out, "%d", v);
        return;
    };
#endif

    switch (val->t)
    {
        case V_BYTE:
            strbuf_addf (out, "0x%02X", v);
            break;
        case V_WORD:
            strbuf_addf (out, "0x%04X", v);
            break;
        case V_DWORD:
            strbuf_addf (out, "0x%08X", v);
            break;
        case V_QWORD:
            strbuf_addf (out, "0x%016I64X", v);
            break;
        case V_XMM:
        case V_DOUBLE:
            assert (!"not implemented");
            break;
        case V_INVALID:
            assert(0);
            break;
    };
};

REG get_as_REG(s_Value* i)
{
#ifdef _WIN64
    return get_as_64(i);
#else
    return get_as_32(i);
#endif
};
