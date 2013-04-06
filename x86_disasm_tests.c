#include <assert.h>
#include "x86_disas.h"

#include "XOR_tests.h"

void disas_test1(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be)
{
    strbuf t=STRBUF_INIT;
    Da* d=Da_Da(x64, (BYTE*)code, adr);
    size_t i;

    //printf (__FUNCTION__"(x64=%d) begin\n", x64);
    assert(d!=NULL);
    Da_ToString(d, &t);
    if (_stricmp (t.buf, must_be)!=0)
    {
        printf ("disas_test1(x64=%d, )->[%s]\n", x64, t.buf);
        printf ("must_be=[%s]\n", must_be);
        for (i=0; i<d->len; i++)
            printf ("code[%d]=0x%02X\n", i, code[i]);
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
        printf (__FUNCTION__"(%s, )->[%s]\n", x64 ? "TRUE" : "FALSE", t.buf);
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
    //cout << "len=" << d.len << endl;
    //cout << hex << d.op[1].get()->value << endl;
    //cout << hex << d.op[0].get()->adr_disp << endl;
};

void x86_disas_test_32()
{
    int modrm, SIB;
    uint8_t buf[0x10];

    //printf (__FUNCTION__"() begin\n");

#include "test32_1.h"

    //Da d(FALSE, (BYTE*)"\x0F\xC9", 0x123456);
    //cout << d.ToString() << endl;

    /*
       for (int i=0; i<=0xf; i++)
       {
       BYTE buf[2];
       buf[0]=0x66;
       buf[1]=0x40 | i;
       Da d(FALSE, (BYTE*)buf, 0x123456);
       cout << d.ToString() << endl;
       };
       exit(0);
       */

    //disas_test1(Fuzzy_False, (const unsigned char*)"", 0x123456, "");

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
    /*
       for (int first_byte=0; first_byte<=0xff; first_byte++)
       {
       if (first_byte==0x9a)
       continue;

       if (first_byte==0xc4 || first_byte==0xc5)
       continue;

       if (first_byte==0xc6 || first_byte==0xc7)
       continue;

       if (first_byte==0xea)
       continue;

       if (first_byte==0xfe)
       continue;

       unsigned char buf[0x10];
       memset (buf, 0x90, sizeof(buf));
       buf[0]=(char)first_byte;

    //disas_test1(Fuzzy_False, buf, 0x200, XOR_modrm_test[modrm]);

    cout << hex << first_byte << endl;
    Da d(FALSE, (BYTE*)buf, 0x300);
    string tst=d.ToString();
    cout << tst << endl;
    };
    */
};

#if 0
void hex_string_to_bytestring (string st, BYTE* out)
{
    for (auto s=st.begin(); s!=st.end();)
    {
        BYTE val;

        if (boost::spirit::qi::parse (s+0, s+2, boost::spirit::hex, val)==FALSE)
        {
            assert(0);
        };
        *out=val;
        out++;
        s=s+2;
    };
};
#endif

// disasm_text_x64.cpp
void x86_disas_test_64();

void main()
{
    printf (__FUNCTION__"() begin\n");
    
    disas_test1(Fuzzy_True, (const unsigned char*)"\x48\x05\x90\x90\x90\x90", 0x8855, "ADD RAX, 0FFFFFFFF90909090h"); // checked in IDA
    disas_test1(Fuzzy_True, (const unsigned char*)"\x48\x0F\x90\x90\x90\x90\x90\x90", 0x88ff, "SETO [RAX-6F6F6F70h]");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\xFC\xE0", 0x1234, "PADDB XMM4, XMM0");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\x76\xC0", 0x1234, "PCMPEQD XMM0, XMM0");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x8C\xC0", 0x1234, "MOV AX, ES");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x8C\xE0", 0x1234, "MOV AX, FS");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x8C\xE8", 0x1234, "MOV AX, GS");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\xA1\xD0\x02\xFE\x7F", 0x1234, "MOV AX, [7FFE02D0h]");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x0F\x03\xC1", 0x1234, "LSL EAX, ECX");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x87\x45\x06", 0x1234, "XCHG AX, [EBP+6]");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x0F\x10\x02", 0x1234, "MOVUPS XMM0, [EDX]");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\x38\xDB\xC0", 0x1234, "AESIMC XMM0, XMM0");
    disas_test1(Fuzzy_False, (const unsigned char*)"\xF0\x66\x0F\xB1\x0B", 0x1234, "LOCK CMPXCHG [EBX], CX");
    disas_test1(Fuzzy_False, (const unsigned char*)"\xF3\x0F\x12\xC0", 0x1234, "MOVSLDUP XMM0, XMM0");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\x3A\xDF\xC0\x00", 0x1234, "AESKEYGENASSIST XMM0, XMM0, 0");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\x38\xDC\xC1", 0x1234, "AESENC XMM0, XMM1");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\x38\xDD\xC1", 0x1234, "AESENCLAST XMM0, XMM1");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x66\x0F\x28\xC1", 0x1234, "MOVAPD XMM0, XMM1");
    disas_test1(Fuzzy_False, (const unsigned char*)"\x0F\x01\xD0", 0x1234, "XGETBV");
    disas_test1(Fuzzy_True, (const unsigned char*)"\x0F\x1F\x40\x00", 0x1234, "NOP [RAX]");
    disas_test1(Fuzzy_False, (const unsigned char*)"\xD9\xCA", 0x1234, "FXCH ST0, ST2");
    disas_test1(Fuzzy_False, (const unsigned char*)"\xDE\xF9", 0x1234, "FDIVP ST1, ST0");
    disas_test1(Fuzzy_False, (const unsigned char*)"\xDE\xF1", 0x1234, "FDIVRP ST1, ST0");

    //exit(0);

    //x86_disas_test_1();

    x86_disas_test_32();
    x86_disas_test_64();

    //x86_disas_text_64_from_lst("disp+work.lst.unfull");
    /*
       cout << "untested ins_tbl[] entries:" << endl;
       print_unused_tbl_entries();
       */
};
