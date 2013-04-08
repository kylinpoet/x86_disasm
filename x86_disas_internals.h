#pragma once

#include "X86_register.h"
#include "x86_disas.h"

//#pragma pack(push)
//#pragma pack(1)
typedef struct _Da_stage1
{
    BOOL use_callbacks:1;
    // case 1
    uint8_t* cur_ptr;
    // case 2
    callback_read_byte read_byte_fn;
    callback_read_word read_word_fn;
    callback_read_dword read_dword_fn;
    callback_read_oword read_oword_fn;
    void *callback_param;

    // case 1 and 2:
    disas_address cur_adr;

    unsigned PREFIXES;
    unsigned ESCAPE_0F:1, ESCAPE_F2:1, ESCAPE_F3:1;
    unsigned PREFIX_66_is_present:1, PREFIX_67:1;
    Ins_codes ins_code;
    unsigned len;
    unsigned tbl_p;
    uint64_t new_flags; // including promoted F_IMM64, etc

    uint8_t REG_FROM_LOWEST_PART_OF_1ST_BYTE; // lowest 3 bits
    unsigned REG_FROM_LOWEST_PART_OF_1ST_BYTE_loaded:1;

    unsigned x64:1;
    unsigned REX_prefix_seen:1;
    unsigned REX_W:1, REX_R:1, REX_X:1, REX_B:1;

    // MODRM = MOD(2) | REG(3) | RM(3)
    union
    {
        struct
        {
            unsigned RM:3;
            unsigned REG:3;
            unsigned MOD:2;
        } s;
        uint8_t as_byte;
    } MODRM;
    BOOL MODRM_loaded;

    // SIB
    union
    {
        struct
        {
            unsigned base:3;
            unsigned index:3;
            unsigned scale:2;
        } s;
        uint8_t as_byte;
    } SIB;
    BOOL SIB_loaded;

    // DISP8/16/32
    int8_t DISP8; unsigned DISP8_loaded:1;
    int16_t DISP16; unsigned DISP16_loaded:1;
    int32_t DISP32; unsigned DISP32_loaded:1; disas_address DISP32_pos;

    // IMM8/16/32
    // 8 и 16 могут быть одновременно загружены, вот как при ENTER
    uint8_t IMM8;  unsigned IMM8_loaded:1;
    uint16_t IMM16; unsigned IMM16_loaded:1;
    uint32_t IMM32; unsigned IMM32_loaded:1; disas_address IMM32_pos;
    uint64_t IMM64; unsigned IMM64_loaded:1; disas_address IMM64_pos;

    uint64_t PTR; unsigned PTR_loaded; disas_address PTR_pos;
} Da_stage1;
//#pragma pack(pop)

// "methods"
uint8_t Da_stage1_get_next_byte(Da_stage1* p);
void Da_stage1_unget_byte(Da_stage1 *p);
uint16_t Da_stage1_get_next_word(Da_stage1 *p);
uint32_t Da_stage1_get_next_dword(Da_stage1 *p);
uint64_t Da_stage1_get_next_qword (Da_stage1 *p);
uint8_t Da_stage1_load_prefixes_escapes_opcode (Da_stage1 *p, disas_address adr_of_ins);
void Da_stage1_dump (Da_stage1 *p, disas_address adr, int len);
BOOL Da_stage1_Da_stage1 (Da_stage1 *p, TrueFalseUndefined x64_code, disas_address adr_of_ins);

// flags:

#define F_MODRM 1i64<<0
#define F_IMM8  1i64<<1
#define F_IMM16 1i64<<2
#define F_IMM32 1i64<<3

#define F_PREFIX66_ALLOWED 1i64<<4
#define F_PREFIX66_IS_PART_OF_OPCODE 1i64<<5
#define F_PREFIX66_APPLIED_TO_OP1_ONLY 1i64<<6

#define F_REG32_IS_LOWEST_PART_OF_1ST_BYTE 1i64<<7

#define F_REG64_IS_LOWEST_PART_OF_1ST_BYTE 1i64<<9

// including promoting F_IMM32 to F_IMM64 ... (кажется)
#define F_REXW_PROMOTE_ALL_32_OPS_TO_64 1i64<<11

#define F_X32_ONLY 1i64<<12
#define F_X64_ONLY 1i64<<13

#define F_IMM64 1i64<<14

#define F_REXW_ABSENT 1i64<<16
#define F_REXW_PRESENT 1i64<<17

#define F_X64_PROMOTE_OP1_32_TO_64 1i64<<18
#define F_REXW_SIGN_EXTEND_OP2_32_TO_64 1i64<<19
#define F_REXW_PROMOTE_OP1_32_TO_64 1i64<<20

#define F_WHEN_MOD3_TREAT_RM_AS_STx 1i64<<21

// 0F is part of opcode?
#define F_0F 1i64<<22

#define F_MODRM_REG_0 1i64<<23
#define F_MODRM_REG_1 1i64<<24
#define F_MODRM_REG_2 1i64<<25
#define F_MODRM_REG_3 1i64<<26
#define F_MODRM_REG_4 1i64<<27
#define F_MODRM_REG_5 1i64<<28
#define F_MODRM_REG_6 1i64<<29
#define F_MODRM_REG_7 1i64<<30

#define F_MODRM_RM_2  1i64<<31
#define F_MODRM_RM_3  1i64<<32
#define F_MODRM_RM_0  1i64<<33

// F3 is part of opcode? (it was REP instruction also)
#define F_F3          1i64<<34

// F2 is part of opcode? (it was REPNE instruction also)
#define F_F2          1i64<<35

#define F_MODRM_MOD_IS_3     1i64<<36
#define F_MODRM_MOD_IS_NOT_3 1i64<<37
#define F_MODRM_RM_1         1i64<<38
#define F_MODRM_RM_5         1i64<<39
#define F_MODRM_RM_4       1i64<<40
#define F_MODRM_RM_6       1i64<<41
#define F_MODRM_RM_7       1i64<<42

#define F_PTR              1i64<<43
#define F_OPC2             1i64<<44

// the flag is to be set in table in DEBUG build if disasm used the entry at least once
// this information will be used in print_unused_tbl_entries() while testing
#define F_HIT_DURING_EXECUTION  1i64<<45

typedef enum _op_source
{
    OP_REG32_FROM_LOWEST_PART_OF_1ST_BYTE,
    OP_REG64_FROM_LOWEST_PART_OF_1ST_BYTE,
    OP_REG8_FROM_LOWEST_PART_OF_1ST_BYTE,

    OP_1, // operand = 1
    OP_AH,
    OP_AL, // operand is AL
    OP_AX,
    OP_BH,
    OP_BL,
    OP_BX,
    OP_CH,
    OP_CL,
    OP_CX,
    OP_DH,
    OP_DL,
    OP_DX,
    OP_EAX, // = EAX
    OP_EBP, // = EBP
    OP_EBX, // = EBX
    OP_ECX, // = ECX
    OP_EDI, // = EDI
    OP_EDX, // = EDX
    OP_ESI, // = ESI
    OP_ESP,

    OP_RAX,
    OP_RBP,
    OP_RBX,
    OP_RCX,
    OP_RDI,
    OP_RDX,
    OP_RSI,
    OP_RSP,

    OP_SP,
    OP_BP,
    OP_SI,
    OP_DI,

    OP_ST0,
    OP_ST1,
    OP_ST2,
    OP_ST3,
    OP_ST4,
    OP_ST5,
    OP_ST6,
    OP_ST7,

    OP_ES, // 0
    OP_CS, // 1
    OP_SS, // 2
    OP_DS, // 3
    OP_FS, // 4
    OP_GS, // 5

    OP_MODRM_R64, // take operand from REG field of MODRM
    OP_MODRM_RM64, // take operand from MODRM field of MODRM

    OP_MODRM_R32, // take operand from REG field of MODRM (39)
    OP_MODRM_RM32, // take operand from MODRM field of MODRM (40)

    OP_MODRM_R16, // take operand from REG field of MODRM
    OP_MODRM_RM16, // take operand from MODRM field of MODRM

    OP_MODRM_SREG, // take operand from REG field of MODRM as Sreg3 (segment registers)

    OP_MODRM_R8, // take operand from REG field of MODRM as r8
    OP_MODRM_RM8, // take operand from MODRM field of MODRM as r/m8

    OP_MODRM_R_MM, // take operand from REG field of MODRM as MMX register
    OP_MODRM_RM_MM, // take operand from MODRM field of MODRM as MMX register

    OP_MODRM_R_XMM, // take operand from REG field of MODRM as XMM register
    OP_MODRM_RM_XMM, // take operand from MODRM field of MODRM as XMM register

    OP_MOFFS32,
    OP_MOFFS16,
    OP_MOFFS8,

    OP_MODRM_RM_M64FP,

    OP_IMM8, // take operand from IMM8 field
    OP_IMM8_SIGN_EXTENDED_TO_IMM32, // take byte (imm8) and sign-extend it to imm32
    OP_IMM8_SIGN_EXTENDED_TO_IMM16, // take byte (imm8) and sign-extend it to imm16
    OP_IMM8_SIGN_EXTENDED_TO_IMM64,
    OP_IMM16, // take operand from IMM16 field
    OP_IMM16_SIGN_EXTENDED_TO_IMM32,
    OP_IMM16_SIGN_EXTENDED_TO_IMM64,
    OP_IMM32, // take operand from IMM32 field
    OP_IMM32_SIGN_EXTENDED_TO_IMM64, // as it in PUSH (68...)
    OP_IMM64,

    OP_IMM64_AS_OFS8,
    OP_IMM64_AS_OFS32,
    OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_BYTE, // for MOV (A0, A1...)
    OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_DWORD, // for MOV (A0, A1...)

    OP_IMM32_AS_OFS32, // take IMM32 as offset, e.g. MOV DWORD PTR [IMM32], reg
    OP_IMM32_AS_OFS16, // take IMM32 as offset, e.g. MOV WORD PTR [IMM32], reg
    OP_IMM32_AS_OFS8, // take IMM32 as offset, e.g. MOV BYTE PTR [IMM32], reg

    OP_IMM32_AS_REL32, // take IMM32 as rel32 for JMP rel32
    OP_IMM32_SIGN_EXTENDED_TO_REL64,

    OP_IMM8_AS_REL32, // take IMM8 as rel32 for JMP imm8
    OP_IMM8_AS_REL64,

    OP_ABSENT
} op_source;

typedef struct _Ins_definition
{
    uint8_t opc;
    uint8_t opc2;
    uint64_t flags;
    enum op_source op1; // source of first operand
    enum op_source op2; // second
    enum op_source op3; // third

    const char *name;

    Ins_codes ins_code;
} Ins_definition;

Ins_definition ins_tbl[];

