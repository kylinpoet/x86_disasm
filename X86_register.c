/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#include "X86_register.h"
#include <stdint.h>
#include <assert.h>

bool X86_register_is_flag (X86_register r)
{
    switch (r)
    {
    case R_PF:
    case R_SF:
    case R_AF:
    case R_ZF:
    case R_OF:
    case R_CF:
    case R_DF:
    case R_TF:
        return true;
    default:
        return false;
    };
};

bool X86_register_is_STx (X86_register r)
{
    switch (r)
    {
    case R_ST0:
    case R_ST1:
    case R_ST2:
    case R_ST3:
    case R_ST4:
    case R_ST5:
    case R_ST6:
    case R_ST7:
        return true;
    default:
        return false;
    };
};

/*
bool X86_register_from_string (const char* s, X86_register *out)
{
    *out=X86_register_from_string (s);
    if (*out==R_ABSENT)
        return false;
    return true;
};
*/

X86_register X86_register_from_string (const char* s)
{
#if 0
    L ("%s:%d s=%s, this=0x%x\n", __FUNCTION__, __LINE__, s, this);
#endif

    X86_register r=R_ABSENT;

    if (_stricmp (s, "eax")==0) r=R_EAX;
    if (_stricmp (s, "ebx")==0) r=R_EBX;
    if (_stricmp (s, "ecx")==0) r=R_ECX;
    if (_stricmp (s, "edx")==0) r=R_EDX;
    if (_stricmp (s, "esi")==0) r=R_ESI;
    if (_stricmp (s, "edi")==0) r=R_EDI;
    if (_stricmp (s, "ebp")==0) r=R_EBP;
    if (_stricmp (s, "esp")==0) r=R_ESP;
    if (_stricmp (s, "eip")==0) r=R_EIP;

    if (_stricmp (s, "rax")==0) r=R_RAX;
    if (_stricmp (s, "rbx")==0) r=R_RBX;
    if (_stricmp (s, "rcx")==0) r=R_RCX;
    if (_stricmp (s, "rdx")==0) r=R_RDX;
    if (_stricmp (s, "rsi")==0) r=R_RSI;
    if (_stricmp (s, "rdi")==0) r=R_RDI;
    if (_stricmp (s, "rbp")==0) r=R_RBP;
    if (_stricmp (s, "rsp")==0) r=R_RSP;
    if (_stricmp (s, "rip")==0) r=R_RIP;
    if (_stricmp (s, "r8")==0)  r=R_R8;
    if (_stricmp (s, "r9")==0)  r=R_R9;
    if (_stricmp (s, "r10")==0) r=R_R10;
    if (_stricmp (s, "r11")==0) r=R_R11;
    if (_stricmp (s, "r12")==0) r=R_R12;
    if (_stricmp (s, "r13")==0) r=R_R13;
    if (_stricmp (s, "r14")==0) r=R_R14;
    if (_stricmp (s, "r15")==0) r=R_R15;

    if (_stricmp (s, "st0")==0) r=R_ST0;
    if (_stricmp (s, "st1")==0) r=R_ST1;
    if (_stricmp (s, "st2")==0) r=R_ST2;
    if (_stricmp (s, "st3")==0) r=R_ST3;
    if (_stricmp (s, "st4")==0) r=R_ST4;
    if (_stricmp (s, "st5")==0) r=R_ST5;
    if (_stricmp (s, "st6")==0) r=R_ST6;
    if (_stricmp (s, "st7")==0) r=R_ST7;

    if (_stricmp (s, "pf")==0) r=R_PF;
    if (_stricmp (s, "sf")==0) r=R_SF;
    if (_stricmp (s, "af")==0) r=R_AF;
    if (_stricmp (s, "zf")==0) r=R_ZF;
    if (_stricmp (s, "of")==0) r=R_OF;
    if (_stricmp (s, "cf")==0) r=R_CF;
    if (_stricmp (s, "df")==0) r=R_DF;
    if (_stricmp (s, "tf")==0) r=R_TF;

    return r;
};

const char* X86_register_ToString (X86_register r)
{
#if 0
    L ("%s:%d this=0x%x\n", __FUNCTION__, __LINE__, this);
#endif

    switch (r)
    {
    case R_AL: return "AL";
    case R_BL: return "BL";
    case R_CL: return "CL";
    case R_DL: return "DL";

    case R_AH: return "AH";
    case R_BH: return "BH";
    case R_CH: return "CH";
    case R_DH: return "DH";

    case R_AX: return "AX";
    case R_BX: return "BX";
    case R_CX: return "CX";
    case R_DX: return "DX";

    case R_BP: return "BP";
    case R_DI: return "DI";
    case R_SI: return "SI";
    case R_SP: return "SP";

    case R_FS: return "FS";
    case R_GS: return "GS";
    case R_SS: return "SS";
    case R_ES: return "ES";
    case R_DS: return "DS";
    case R_CS: return "CS";

    case R_EAX: return "EAX";
    case R_EBX: return "EBX";
    case R_ECX: return "ECX";
    case R_EDI: return "EDI";
    case R_EDX: return "EDX";
    case R_ESI: return "ESI";
    case R_ESP: return "ESP";
    case R_EBP: return "EBP";
    case R_EIP: return "EIP";

    case R_RAX: return "RAX";
    case R_RBX: return "RBX";
    case R_RCX: return "RCX";
    case R_RDI: return "RDI";
    case R_RDX: return "RDX";
    case R_RSI: return "RSI";
    case R_RSP: return "RSP";
    case R_RBP: return "RBP";
    case R_RIP: return "RIP";

    case R_R8:  return "R8";
    case R_R9:  return "R9";
    case R_R10: return "R10";
    case R_R11: return "R11";
    case R_R12: return "R12";
    case R_R13: return "R13";
    case R_R14: return "R14";
    case R_R15: return "R15";

    case R_R8D:  return "R8D";
    case R_R9D:  return "R9D";
    case R_R10D: return "R10D";
    case R_R11D: return "R11D";
    case R_R12D: return "R12D";
    case R_R13D: return "R13D";
    case R_R14D: return "R14D";
    case R_R15D: return "R15D";

    case R_R8W:  return "R8W";
    case R_R9W:  return "R9W";
    case R_R10W: return "R10W";
    case R_R11W: return "R11W";
    case R_R12W: return "R12W";
    case R_R13W: return "R13W";
    case R_R14W: return "R14W";
    case R_R15W: return "R15W";

#ifdef DISASM_IDA_STYLE
    case R_R8L:  return "R8B";
    case R_R9L:  return "R9B";
    case R_R10L: return "R10B";
    case R_R11L: return "R11B";
    case R_R12L: return "R12B";
    case R_R13L: return "R13B";
    case R_R14L: return "R14B";
    case R_R15L: return "R15B";
#else
    case R_R8L:  return "R8L";
    case R_R9L:  return "R9L";
    case R_R10L: return "R10L";
    case R_R11L: return "R11L";
    case R_R12L: return "R12L";
    case R_R13L: return "R13L";
    case R_R14L: return "R14L";
    case R_R15L: return "R15L";
#endif

    case R_SPL: return "SPL";
    case R_BPL: return "BPL";
    case R_SIL: return "SIL";
    case R_DIL: return "DIL";

    case R_ST0: return "ST0";
    case R_ST1: return "ST1";
    case R_ST2: return "ST2";
    case R_ST3: return "ST3";
    case R_ST4: return "ST4";
    case R_ST5: return "ST5";
    case R_ST6: return "ST6";
    case R_ST7: return "ST7";

    case R_XMM0: return "XMM0";
    case R_XMM1: return "XMM1";
    case R_XMM2: return "XMM2";
    case R_XMM3: return "XMM3";
    case R_XMM4: return "XMM4";
    case R_XMM5: return "XMM5";
    case R_XMM6: return "XMM6";
    case R_XMM7: return "XMM7";
    case R_XMM8: return "XMM8";
    case R_XMM9: return "XMM9";
    case R_XMM10: return "XMM10";
    case R_XMM11: return "XMM11";
    case R_XMM12: return "XMM12";
    case R_XMM13: return "XMM13";
    case R_XMM14: return "XMM14";
    case R_XMM15: return "XMM15";

    case R_MM0: return "MM0";
    case R_MM1: return "MM1";
    case R_MM2: return "MM2";
    case R_MM3: return "MM3";
    case R_MM4: return "MM4";
    case R_MM5: return "MM5";
    case R_MM6: return "MM6";
    case R_MM7: return "MM7";

    case R_PF: return "PF";
    case R_SF: return "SF";
    case R_AF: return "AF";
    case R_ZF: return "ZF";
    case R_OF: return "OF";
    case R_CF: return "CF";
    case R_DF: return "DF";
    case R_TF: return "TF";

    case R_ABSENT: return "ABSENT";
    default: 
        assert(!"unknown register");
    };
    assert(0);
    return ""; // make compiler happy
};

bool X86_register_is_ExX_ExI(X86_register r)
{
    return (r==R_EAX || r==R_EBX || r==R_ECX || r==R_EDX || r==R_ESI || r==R_EDI || r==R_EBP || r==R_ESP);
};
bool X86_register_is_xX_xI(X86_register r)
{
    return (r==R_AX || r==R_BX || r==R_CX || r==R_DX || r==R_SI || r==R_DI || r==R_BP || r==R_SP);
};
bool X86_register_is_xH(X86_register r)
{
    return (r==R_AH || r==R_BH || r==R_CH || r==R_DH);
};
bool X86_register_is_xL(X86_register r)
{
    return (r==R_AL || r==R_BL || r==R_CL || r==R_DL);
};

bool X86_register_is_segment(X86_register r)
{
    return (r==R_CS || r==R_DS || r==R_SS || r==R_ES || r==R_FS || r==R_GS);
};

X86_register X86_register_get_32bit_part_of(X86_register r)
{
    switch (r)
    {
    case R_EAX: case R_AX: case R_AL: case R_AH: return R_EAX;
    case R_EBX: case R_BX: case R_BL: case R_BH: return R_EBX;
    case R_ECX: case R_CX: case R_CL: case R_CH: return R_ECX;
    case R_EDX: case R_DX: case R_DL: case R_DH: return R_EDX;
    case R_ESI: case R_SI: return R_ESI;
    case R_EDI: case R_DI: return R_EDI;
    case R_EBP: case R_BP: return R_EBP;
    case R_ESP: case R_SP: return R_ESP;
    default:
        assert(0);
    };

    assert(0);
    return R_EAX; // make compiler happy
};

bool X86_register_is_XMMx(X86_register r)
{
    switch (r)
    {
    case R_XMM0:
    case R_XMM1:
    case R_XMM2:
    case R_XMM3:
    case R_XMM4:
    case R_XMM5:
    case R_XMM6:
    case R_XMM7:
    case R_XMM8:
    case R_XMM9:
    case R_XMM10:
    case R_XMM11:
    case R_XMM12:
    case R_XMM13:
    case R_XMM14:
    case R_XMM15:
        return true;
    default:
        return false;
    };
};
