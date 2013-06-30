/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#pragma once

#include <stdint.h>
#include "X86_register.h"
#include "value.h"
#include "x86_ins_codes.h"
#include "logging.h"
#include "strbuf.h"
#include "fuzzybool.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _WIN64
typedef uint64_t disas_address;
#else
typedef uint32_t disas_address;
#endif

// http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-2a-manual.html
#define X86_MAXIMAL_INS_LEN 15

#define PREFIX_LOCK  1<<0
#define PREFIX_FS    1<<1
#define PREFIX_SS    1<<2
#define PREFIX_GS    1<<3

typedef enum _Da_coded_result_op_type
{
    DA_OP_TYPE_ABSENT=0,
    DA_OP_TYPE_REGISTER=1,
    DA_OP_TYPE_VALUE=2,
    DA_OP_TYPE_VALUE_IN_MEMORY=3
} Da_coded_result_op_type;

struct _Da_stage1;
typedef struct _Da_stage1 Da_stage1;

//#pragma pack(push)
//#pragma pack(1)
typedef struct _Da_op
{
    Da_coded_result_op_type type:8;

    // in any case 
    uint8_t value_width_in_bits; // FIXME: should be _in_bytes

    union
    {
        // type=DA_OP_TYPE_REGISTER
        X86_register reg:8;
        struct
        {
            // type=DA_OP_TYPE_VALUE
            s_Value v;

            disas_address value32_pos; // FIXME: here should be offset stored in uint8_t
            disas_address value64_pos; // FIXME: dangling value?
        } val;
        struct
        {
            // type=DA_OP_TYPE_VALUE_IN_MEMORY
            X86_register adr_base;
            X86_register adr_index;
            unsigned adr_index_mult;
            uint8_t adr_disp_width_in_bits;
            int64_t adr_disp; // signed
            unsigned adr_disp_is_absolute:1; // for MOV (opcodes A0, A1...)

            // это костыль для случаев mod=0, rm=4/5: disp32 может быть адресом в памяти, 
            // поэтому disp32>0x80000000 должен отображаться так же
            unsigned adr_disp_is_not_negative:1;

            disas_address adr_disp_pos;
            // shared_ptr<class Symbol> adr_disp_sym; тут будет расширено
        } adr;
    } u;
} Da_op;
//#pragma pack(pop)

// functions to work with Da_op struct
bool Da_op_equals(Da_op *op1, Da_op *op2);
bool Da_op_is_reg(Da_op *op, X86_register reg);
void Da_op_ToString (Da_op* op, strbuf* out);
void Da_op_DumpString (fds* s, Da_op* op);
bool Da_op_is_adr_disp_negative(Da_op *op);
void Da_op_free(Da_op* op);

//#pragma pack(push)
//#pragma pack(1)
typedef struct _Da
{
    unsigned prefix_codes;
    Ins_codes ins_code;

    // наверное можно было бы добавить поле вроде "сколько операндов присутствует",
    // но это было бы redundant - эту инфу можно получить из op[]
    Da_op* _op[3];
    // int ops_total; это тут делать не надо, ибо есть operands_total()
    unsigned len:4; // remember: X86_MAXIMAL_INS_LEN is 15
} Da;
//#pragma pack(pop)

typedef bool (*callback_read_byte)(void* param, disas_address adr, uint8_t* out);
typedef bool (*callback_read_word)(void* param, disas_address adr, uint16_t* out);
typedef bool (*callback_read_dword)(void* param, disas_address adr, uint32_t* out);
typedef bool (*callback_read_oword)(void* param, disas_address adr, uint64_t* out);

// functions to work with Da struct

Da* Da_Da (TrueFalseUndefined x64_code, uint8_t* ptr_to_ins, disas_address adr_of_ins);
Da* Da_Da_callbacks (TrueFalseUndefined x64_code, disas_address adr_of_ins, 
        callback_read_byte rb, callback_read_word rw, callback_read_dword rd, callback_read_oword ro, 
        void *param);

void Da_ToString (Da *d, strbuf *out);
void Da_DumpString(fds *s, Da *d);
bool Da_is_MOV_EBP_ESP(Da* d);
bool Da_is_PUSH_EBP(Da* d);
bool Da_ins_is_Jcc (Da* d);
bool Da_ins_is_FPU (Da* d);
const char* Da_ins_code_ToString(Da *d);
int Da_operands_total(Da* d);

// FIXME: 32-bit only?
bool Da_is_ADD_ESP_X (Da* d, uint32_t * out_X);
bool Da_is_SUB_ESP_X (Da* d, uint32_t * out_X);
bool Da_is_RET (Da* d, uint16_t * out_X);

void Da_free (Da* d);

Da *Da_copy(Da *da);

const char* disas1_ins_code_to_string (Ins_codes ins_code);

#ifdef _DEBUG
void print_unused_tbl_entries();
#endif

#ifdef  __cplusplus
}
#endif
