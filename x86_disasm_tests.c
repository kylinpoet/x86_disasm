/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#include "memutils.h"
#include "oassert.h"
#include "stuff.h"
#include "strbuf.h"
#include "x86_disas.h"

#include "XOR_tests.h"

void disas_test1(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be)
{
    strbuf t=STRBUF_INIT;
    Da d;
    bool b=Da_Da(x64, (BYTE*)code, adr, &d);
    size_t i;

    //printf (__FUNCTION__"(x64=%d) begin\n", x64);
    if (b==false)
    {
        printf ("%s(x64=%d, ); must_be=%s\n", __FUNCTION__, x64, must_be);
        printf ("can't disassemble!\n");
        exit(0);
    };
    Da_ToString(&d, &t);
    if (_stricmp (t.buf, must_be)!=0)
    {
        printf ("%s(x64=%d, )->[%s]\n", __FUNCTION__, x64, t.buf);
        printf ("must_be=[%s]\n", must_be);
        for (i=0; i<d.ins_len; i++)
            printf ("code[%d]=0x%02X\n", i, code[i]);
        //debugger_breakpoint();
        exit(0);
    };
    strbuf_deinit(&t);
};

void disas_test2_2op(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be, int value2_must_be)
{
    strbuf t=STRBUF_INIT;
    Da d;
    bool b;
    size_t i;

    b=Da_Da(x64, (BYTE*)code, adr, &d);
    //printf (__FUNCTION__"() begin\n");
    oassert(b);
    Da_ToString(&d, &t);
    if (_stricmp (t.buf, must_be)!=0
        || d.op[0].value_width_in_bits!=value1_must_be
        || d.op[1].value_width_in_bits!=value2_must_be)
    {
        printf ("%s(%s, )->[%s]\n", __FUNCTION__, bool_to_string(x64), t.buf);
        printf ("must_be=[%s]\n", must_be);
        printf ("d.op[0]->value_width_in_bits=%d\n", d.op[0].value_width_in_bits);
        printf ("d.op[1]->value_width_in_bits=%d\n", d.op[1].value_width_in_bits);
        printf ("value1_must_be=%d\n", value1_must_be);
        printf ("value2_must_be=%d\n", value2_must_be);
        for (i=0; i<d.ins_len; i++)
            printf ("code[%d]=0x%02X\n", i, code[i]);
        exit(0);
    };
    strbuf_deinit(&t);
};

void disas_test2_1op(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be)
{
    strbuf t=STRBUF_INIT;
    Da d;
    bool b;
    
    b=Da_Da(x64, (BYTE*)code, adr, &d);

    //printf (__FUNCTION__"() begin\n");
    oassert (b);
    Da_ToString(&d, &t);
    if (_stricmp(t.buf, must_be)!=0 || d.op[0].value_width_in_bits!=value1_must_be)
    {
        printf ("disas_test1(Fuzzy_False, )->[%s]\n", t.buf);
        printf ("must_be=[%s]\n", must_be);
        printf ("d.op[0]->value_width_in_bits=%d\n", d.op[0].value_width_in_bits);
        printf ("value1_must_be=%d\n", value1_must_be);
        oassert(0); fatal_error();
    };
    strbuf_deinit(&t);
};

void x86_disas_test_1()
{
    Da d;
    bool b;

    //printf (__FUNCTION__"() begin\n");

#ifdef _WIN64
    b=Da_Da(Fuzzy_True, (BYTE*)"\x74\x2F", 0x14000114c, &d);
#else
    b=Da_Da(Fuzzy_True, (BYTE*)"\x74\x2F", 0x4000114c, &d);
#endif
    oassert (b);
    Da_DumpString(&cur_fds, &d);
    if (d.ops_total>=1) printf ("op0 value width=%d\n", d.op[0].value_width_in_bits);
    if (d.ops_total>=2) printf ("op1 value width=%d\n", d.op[1].value_width_in_bits);
    exit(0);
};

void x86_disas_test_32()
{
    int modrm, SIB;
    uint8_t buf[0x10];

    //printf (__FUNCTION__"() begin\n");

#include "test32_1.h"

#include "test32_2.h"

    for (modrm=0; modrm<=0xff; modrm++)
    {
        bytefill (buf, sizeof(buf), 0x90);
        buf[0]=0x33;
        buf[1]=(char)modrm;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_modrm_test[modrm]);
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        bytefill (buf, sizeof(buf), 0x90);
        buf[0]=0x33;
        buf[1]=0x04; // modrm
        buf[2]=(char)SIB;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_SIB_04_test[SIB]);
        //cout << disas_test1(Fuzzy_False, buf, 0x200) << endl;
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        bytefill (buf, sizeof(buf), 0x90);
        buf[0]=0x33;
        buf[1]=0x44; // modrm
        buf[2]=(char)SIB;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_SIB_44_test[SIB]);
        //cout << disas_test1(Fuzzy_False, buf, 0x200) << endl;
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        bytefill (buf, sizeof(buf), 0x90);
        buf[0]=0x33;
        buf[1]=0x84; // modrm
        buf[2]=(char)SIB;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_SIB_84_test[SIB]);
        //cout << disas_test1(Fuzzy_False, buf, 0x200) << endl;
    };
};

// disasm_text_x64.cpp
void x86_disas_test_64();

void check_SHR()
{
    Da d;
    bool b;

    b=Da_Da(Fuzzy_False, (BYTE*)"\xD1\xEE", 0, &d); // SHR ESI, 1
    //b=Da_Da(Fuzzy_False, (BYTE*)"\xC1\xEE\x03", 0, &d); // SHR ESI, 3
    oassert(b);
    //Da_DumpString(&cur_fds, &d);
    //printf ("\n");
    oassert (d.op[1].val._v.t==OBJ_BYTE);
    //printf ("%d\n", d.op[1].val._v.t);
};

int main()
{
    printf ("%s() begin\n", __FUNCTION__);

    check_SHR();

    x86_disas_test_32();
    x86_disas_test_64();

#ifdef _DEBUG
    //print_unused_tbl_entries();
#endif
    return 0;
};

/* vim: set expandtab ts=4 sw=4 : */
