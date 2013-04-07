#pragma once

#include "value.h"
#include <windows.h>

typedef enum X86_register_t
{
    R_ABSENT=0, // should always be zero!

    R_AH,
    R_AL,
    R_AX,
    R_BH,
    R_BL,
    R_BP,
    R_BX,
    R_CH,
    R_CL,
    R_CS,
    R_CX,
    R_DH,
    R_DI,
    R_DL,
    R_DS,
    R_DX,
    R_EAX,
    R_EBX,
    R_ECX,
    R_EDX,
    R_ESI,
    R_EDI,
    R_EBP,
    R_ESP,
    R_EIP,
    R_ES,
    R_FS,
    R_GS,
    R_SI,
    R_SP,
    R_SS,
    R_ST0,
    R_ST1,
    R_ST2,
    R_ST3,
    R_ST4,
    R_ST5,
    R_ST6,
    R_ST7,
    R_XMM0,
    R_XMM1,
    R_XMM2,
    R_XMM3,
    R_XMM4,
    R_XMM5,
    R_XMM6,
    R_XMM7,
    R_XMM8,
    R_XMM9,
    R_XMM10,
    R_XMM11,
    R_XMM12,
    R_XMM13,
    R_XMM14,
    R_XMM15,
    R_MM0,
    R_MM1,
    R_MM2,
    R_MM3,
    R_MM4,
    R_MM5,
    R_MM6,
    R_MM7,
    R_RAX,
    R_RBX,
    R_RCX,
    R_RDX,
    R_RSI,
    R_RDI,
    R_RSP,
    R_RBP,
    R_RIP,
    R_R8,
    R_R9,
    R_R10,
    R_R11,
    R_R12,
    R_R13,
    R_R14,
    R_R15,

    R_R8D,
    R_R9D,
    R_R10D,
    R_R11D,
    R_R12D,
    R_R13D,
    R_R14D,
    R_R15D,

    R_R8W,
    R_R9W,
    R_R10W,
    R_R11W,
    R_R12W,
    R_R13W,
    R_R14W,
    R_R15W,

    R_R8L,
    R_R9L,
    R_R10L,
    R_R11L,
    R_R12L,
    R_R13L,
    R_R14L,
    R_R15L,

    R_SPL,
    R_BPL,
    R_SIL,
    R_DIL,

    R_PF,
    R_SF,
    R_AF,
    R_ZF,
    R_OF,
    R_CF,
    R_DF,
    R_TF
} X86_register;

X86_register X86_register_from_string (const char* s);
//BOOL X86_register_from_string (const char* s, X86_register *out);
BOOL X86_register_is_flag (X86_register r);
BOOL X86_register_is_STx (X86_register r);
const char* X86_register_ToString (X86_register r);

BOOL X86_register_is_ExX_ExI(X86_register r);
BOOL X86_register_is_xX_xI(X86_register r);
BOOL X86_register_is_xH(X86_register r);
BOOL X86_register_is_xL(X86_register r);
BOOL X86_register_is_segment(X86_register r);
BOOL X86_register_is_XMMx(X86_register r);
X86_register X86_register_get_32bit_part_of(X86_register r);
