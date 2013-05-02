#include <assert.h>
#include "stuff.h"

#include "x86_disas.h"

#include "XOR_tests.h"

void disas_test1(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be)
{
    strbuf t=STRBUF_INIT;
    Da* d=Da_Da(x64, (BYTE*)code, adr);
    size_t i;

    //printf (__FUNCTION__"(x64=%d) begin\n", x64);
    if (d==NULL)
    {
        printf ("%s(x64=%d, ); must_be=%s\n", __FUNCTION__, x64, must_be);
        printf ("can't disassemble!\n");
        exit(0);
    };
    Da_ToString(d, &t);
    if (_stricmp (t.buf, must_be)!=0)
    {
        printf ("%s(x64=%d, )->[%s]\n", __FUNCTION__, x64, t.buf);
        printf ("must_be=[%s]\n", must_be);
        for (i=0; i<d->len; i++)
            printf ("code[%d]=0x%02X\n", i, code[i]);
        //debugger_breakpoint();
        exit(0);
    };
    strbuf_deinit(&t);
    Da_free (d);
};

void disas_test2_2op(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be, int value2_must_be)
{
    strbuf t=STRBUF_INIT;
    Da* d=Da_Da(x64, (BYTE*)code, adr);
    size_t i;

    //printf (__FUNCTION__"() begin\n");
    assert(d!=NULL);
    Da_ToString(d, &t);
    if (_stricmp (t.buf, must_be)!=0
        || d->_op[0]->value_width_in_bits!=value1_must_be
        || d->_op[1]->value_width_in_bits!=value2_must_be)
    {
        printf ("%s(%s, )->[%s]\n", __FUNCTION__, x64 ? "true" : "false", t.buf);
        printf ("must_be=[%s]\n", must_be);
        printf ("d.op[0]->value_width_in_bits=%d\n", d->_op[0]->value_width_in_bits);
        printf ("d.op[1]->value_width_in_bits=%d\n", d->_op[1]->value_width_in_bits);
        printf ("value1_must_be=%d\n", value1_must_be);
        printf ("value2_must_be=%d\n", value2_must_be);
        for (i=0; i<d->len; i++)
            printf ("code[%d]=0x%02X\n", i, code[i]);
        exit(0);
    };
    strbuf_deinit(&t);
    Da_free(d);
};

void disas_test2_1op(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be)
{
    strbuf t=STRBUF_INIT;
    Da* d=Da_Da(x64, (BYTE*)code, adr); 

    //printf (__FUNCTION__"() begin\n");
    assert (d!=NULL);
    Da_ToString(d, &t);
    if (_stricmp(t.buf, must_be)!=0 || d->_op[0]->value_width_in_bits!=value1_must_be)
    {
        printf ("disas_test1(Fuzzy_False, )->[%s]\n", t.buf);
        printf ("must_be=[%s]\n", must_be);
        printf ("d.op[0]->value_width_in_bits=%d\n", d->_op[0]->value_width_in_bits);
        printf ("value1_must_be=%d\n", value1_must_be);
        assert(0);
    };
    strbuf_deinit(&t);
    Da_free (d);
};

void x86_disas_test_1()
{
    Da* d;

    //printf (__FUNCTION__"() begin\n");

#ifdef _WIN64
    d=Da_Da(Fuzzy_True, (BYTE*)"\x74\x2F", 0x14000114c);
#else
    d=Da_Da(Fuzzy_True, (BYTE*)"\x74\x2F", 0x4000114c);
#endif
    assert (d!=NULL);
    Da_DumpString(&cur_fds, d);
    if (d->_op[0]) printf ("op0 value width=%d\n", d->_op[0]->value_width_in_bits);
    if (d->_op[1]) printf ("op1 value width=%d\n", d->_op[1]->value_width_in_bits);
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
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=(char)modrm;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_modrm_test[modrm]);
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=0x04; // modrm
        buf[2]=(char)SIB;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_SIB_04_test[SIB]);
        //cout << disas_test1(Fuzzy_False, buf, 0x200) << endl;
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=0x44; // modrm
        buf[2]=(char)SIB;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_SIB_44_test[SIB]);
        //cout << disas_test1(Fuzzy_False, buf, 0x200) << endl;
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=0x84; // modrm
        buf[2]=(char)SIB;

        disas_test1(Fuzzy_False, buf, 0x200, XOR_SIB_84_test[SIB]);
        //cout << disas_test1(Fuzzy_False, buf, 0x200) << endl;
    };
};

// disasm_text_x64.cpp
void x86_disas_test_64();

int main()
{
    printf ("%s() begin\n", __FUNCTION__);
    
    x86_disas_test_32();
    x86_disas_test_64();

    print_unused_tbl_entries();
    return 0;
};
