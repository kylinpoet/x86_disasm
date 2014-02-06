/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#include <stdbool.h>

#include "oassert.h"
#include "x86_disas.h"
#include "x86_disas_internals.h"
#include "dmalloc.h"
#include "memutils.h"
#include "bitfields.h"
#include "strbuf.h"
#include "stuff.h"
#include "fmt_utils.h"
#include "memutils.h"

extern Ins_definition ins_tbl[]; // in x86_tbl.cpp file

static X86_register _64_registers_by_idx[]=
{ R_RAX, R_RCX, R_RDX, R_RBX, R_RSP, R_RBP, R_RSI, R_RDI, R_R8, R_R9, R_R10, R_R11, R_R12, R_R13, R_R14, R_R15 };

static X86_register XMM_registers_by_idx[]=
{ R_XMM0, R_XMM1, R_XMM2, R_XMM3, R_XMM4, R_XMM5, R_XMM6, R_XMM7, R_XMM8, R_XMM9, R_XMM10, R_XMM11,
  R_XMM12, R_XMM13, R_XMM14, R_XMM15 };

static X86_register _32_registers_by_idx[]=
{ R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI, R_R8D, R_R9D, R_R10D, R_R11D, R_R12D, R_R13D,
  R_R14D, R_R15D, };

static X86_register _16_registers_by_idx[]=
{ R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI, R_R8W, R_R9W, R_R10W, R_R11W, R_R12W, R_R13W, R_R14W, R_R15W };

static X86_register STx_registers_by_idx[]=
{ R_ST0, R_ST1, R_ST2, R_ST3, R_ST4, R_ST5, R_ST6, R_ST7, };

static X86_register _8_registers_by_idx[]=
{ R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH, R_R8L, R_R9L, R_R10L, R_R11L, R_R12L, R_R13L, R_R14L, R_R15L };

static bool dbg_print=false;

#ifdef _DEBUG
void dump_Ins_definition(Ins_definition *d)
{
    printf ("%s, opc=%02X", d->name, d->opc);
    if (IS_SET(d->flags, F_OPC2))
        printf (", opc2=%02X", d->opc2);
    printf ("\n");
};

void print_unused_tbl_entries()
{
    int i;

    printf ("%s()\n", __FUNCTION__);

    for (i=0;;i++)
    {
        Ins_definition *d=&ins_tbl[i];

        if (d->ins_code==I_INVALID)
            break;
        if (IS_SET(d->flags, F_HIT_DURING_EXECUTION)==false)
        {
            printf ("(untested) ");
            dump_Ins_definition(d);
        }
        else
        {
            //printf ("(tested) ");
            //dump_Ins_definition(d);
        };
    };
};
#endif

static unsigned precomputed_ins_pointers[0x100]={0};
static bool precomputed_ins_pointers_present=false;

static void precompute_ins_pointers()
{
    int i, entries_total=0;
    unsigned cur_opc;
    Ins_definition *d;

    //printf ("ins_tbl=0x%p\n", ins_tbl);

    // check ins_tbl[] monotonicity
    for (i=0, cur_opc=0; ins_tbl[i].ins_code!=I_INVALID; i++, entries_total++)
    {
        d=&ins_tbl[i];
        if (d->opc < cur_opc)
        {
            oassert (!"ins_tbl[] isn't monotonic");
        };

        if (d->opc > cur_opc)
            cur_opc=d->opc;
    };

    //printf ("ins_tbl[] entries=%d\n", entries_total);

    // precompute
    for (i=0, cur_opc=0; ins_tbl[i].ins_code!=I_INVALID; i++)
    {
        d=&ins_tbl[i];
        if (d->opc > cur_opc)
        {
            if (precomputed_ins_pointers[d->opc]==0)
            {
                precomputed_ins_pointers[d->opc]=i;
                if (IS_SET(d->flags, F_REG32_IS_LOWEST_PART_OF_1ST_BYTE))
                {
                    precomputed_ins_pointers[d->opc+1]=i;
                    precomputed_ins_pointers[d->opc+2]=i;
                    precomputed_ins_pointers[d->opc+3]=i;
                    precomputed_ins_pointers[d->opc+4]=i;
                    precomputed_ins_pointers[d->opc+5]=i;
                    precomputed_ins_pointers[d->opc+6]=i;
                    precomputed_ins_pointers[d->opc+7]=i;
                };
            }
            cur_opc=d->opc;
        };
    }; 
    precomputed_ins_pointers_present=true;
};

bool Da_stage1_get_next_byte(Da_stage1* p, uint8_t *out)
{
    if (p->use_callbacks==false)
        p->cur_ptr++, p->cur_adr++, *out=*(p->cur_ptr-1);
    else
    {
        p->cur_adr++;
        if (p->read_byte_fn(p->callback_param, p->cur_adr-1, out)==false)
        {
            if (dbg_print)
                printf ("%s() cannot read at " PRI_ADR_HEX "\n", __func__, p->cur_adr-1);
            return false;
        };
    };
    return true;
};

void Da_stage1_unget_byte(Da_stage1 *p)
{
    p->cur_ptr--;
    p->cur_adr--;
};

bool Da_stage1_get_next_word(Da_stage1 *p, uint16_t *out)
{
    if (p->use_callbacks==false)
    {
        p->cur_ptr+=sizeof(uint16_t);
        p->cur_adr+=sizeof(uint16_t); // will you need it?
        *out=*(uint16_t*)(p->cur_ptr-sizeof(uint16_t));
    } 
    else
    {
        p->cur_adr+=sizeof(uint16_t);
        if (p->read_word_fn(p->callback_param, p->cur_adr-sizeof(uint16_t), out)==false)
        {
            if (dbg_print)
                printf ("%s() cannot read at " PRI_ADR_HEX "\n", __func__, p->cur_adr-sizeof(uint16_t));
            return false;
        };
    };
    return true;
};

bool Da_stage1_get_next_dword(Da_stage1 *p, uint32_t *out)
{
    if (p->use_callbacks==false)
    {
        p->cur_ptr+=sizeof(uint32_t);
        p->cur_adr+=sizeof(uint32_t); // will you need it?
        *out=*(uint32_t*)(p->cur_ptr-sizeof(uint32_t));
    }
    else
    {
        p->cur_adr+=sizeof(uint32_t);
        if (p->read_dword_fn(p->callback_param, p->cur_adr-sizeof(uint32_t), out)==false)
        {
            if (dbg_print)
                printf ("%s() cannot read at " PRI_ADR_HEX "\n", __func__, p->cur_adr-sizeof(uint32_t));
            return false;
        };
    };
    return true;
};

bool Da_stage1_get_next_qword (Da_stage1 *p, uint64_t *out)
{
    if (p->use_callbacks==false)
    {
        p->cur_ptr+=sizeof(uint64_t);
        p->cur_adr+=sizeof(uint64_t); // will you need it?
        *out=*(uint64_t*)(p->cur_ptr-sizeof(uint64_t));
    }
    else
    {
        p->cur_adr+=sizeof(uint64_t);
        if (p->read_oword_fn(p->callback_param, p->cur_adr-sizeof(uint64_t), out)==false)
        {
            if (dbg_print)
                printf ("%s() cannot read at " PRI_ADR_HEX "\n", __func__, p->cur_adr-sizeof(uint64_t));
            return false;
        };
    };
    return true;
};

uint64_t promote_32_flags_to_64 (uint64_t f)
{
    uint64_t rt=f;

    if (IS_SET(rt, F_REG32_IS_LOWEST_PART_OF_1ST_BYTE))
    {
        REMOVE_BIT (rt, F_REG32_IS_LOWEST_PART_OF_1ST_BYTE);
        SET_BIT (rt, F_REG64_IS_LOWEST_PART_OF_1ST_BYTE);
    };

    if (IS_SET(rt, F_IMM32))
    {
        REMOVE_BIT (rt, F_IMM32);
        SET_BIT (rt, F_IMM64);
    };

    return rt;
};

bool Da_stage1_load_prefixes_escapes_opcode (Da_stage1 *p, disas_address adr_of_ins, uint8_t *out)
{
    bool got_prefix=true;

    // grp 1/2/3/4 prefixes
    uint8_t next_byte;

    if (Da_stage1_get_next_byte(p, &next_byte)==false)
        return false;

    while (got_prefix)
    {
        got_prefix=true;
        switch (next_byte)
        {
            case 0x36:
                p->PREFIXES|=PREFIX_SS; 
                p->len++; 
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0x64:
                p->PREFIXES|=PREFIX_FS; 
                p->len++;
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0x65:
                p->PREFIXES|=PREFIX_GS; 
                p->len++; 
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0xF0:
                p->PREFIXES|=PREFIX_LOCK; 
                p->len++; 
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0xF2:
                p->ESCAPE_F2=true; 
                p->len++;
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0xF3:
                p->ESCAPE_F3=true; 
                p->len++;
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0x66:
                p->PREFIX_66_is_present=true;
                p->len++;
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            case 0x67:
                p->PREFIX_67=true;
                p->len++;
                if (Da_stage1_get_next_byte(p, &next_byte)==false)
                    return false;
                break;
            default:
                got_prefix=false;
                break;
        };
    };

    if (p->x64)
    {
        // ищем 4x-префиксы
        if ((next_byte&0xF0)==0x40)
        {
            p->REX_W=p->REX_R=p->REX_X=p->REX_B=false;

            if (IS_SET (next_byte, 1<<3)) p->REX_W=true;
            if (IS_SET (next_byte, 1<<2)) p->REX_R=true;
            if (IS_SET (next_byte, 1<<1)) p->REX_X=true;
            if (IS_SET (next_byte, 1<<0)) p->REX_B=true;

            //printf ("got 0x%02x prefix. REX_W=%d REX_R=%d REX_X=%d REX_B=%d\n", next_byte, REX_W, REX_R, REX_X, REX_B);

            p->REX_prefix_seen=true;
            p->len++;
            if (Da_stage1_get_next_byte(p, &next_byte)==false)
                return false;
            if ((next_byte&0xF0)==0x40)
            {
                fprintf (stderr, "%s() second 4x prefix present at 0x" PRI_ADR_HEX "!", __FUNCTION__, adr_of_ins);
                return false;
            };
        };
    };

    if (next_byte==0x0F)
    {
        p->ESCAPE_0F=true;
        p->len++;
        if (Da_stage1_get_next_byte(p, &next_byte)==false)
            return false;
    };

    *out=next_byte; // opcode
    return true;
};

void Da_stage1_dump (Da_stage1 *p, disas_address adr, int len)
{ 
    //if (p->from_mem_or_from_MemoryCache==false)
    {
        oassert(!"not implemented");
    }
#if 0
    else
    {
        bool b=MC_L_print_buf_in_mem (p->mem, adr, len);
        oassert (b==true);
    };
#endif
};

// FIXME may be optimized probably
void Da_stage1_clear(Da_stage1 *p)
{
    p->PREFIXES=0;
    p->ESCAPE_0F=p->ESCAPE_F2=p->ESCAPE_F3=0;
    p->PREFIX_66_is_present=p->PREFIX_67=0;
    p->len=0;
    p->REX_prefix_seen=0;
    p->REX_W=p->REX_R=p->REX_X=p->REX_B=0;
};

// только эта часть дизасма что-то вытягивает из памяти
bool Da_stage1_Da_stage1 (Da_stage1 *p, TrueFalseUndefined x64_code, disas_address adr_of_ins)
{
    uint8_t opc, mask;
    bool PREFIX66_may_present, PREFIX66_allowed_and_present;

    //printf (__FUNCTION__"()\n");

    p->cur_adr=adr_of_ins;
    Da_stage1_clear(p);

    if (x64_code==Fuzzy_Undefined)
    {
#ifdef _WIN64
        p->x64=true;
#else
        p->x64=false;
#endif
    }
    else if (x64_code==Fuzzy_True)
        p->x64=true;
    else
        p->x64=false;

    if (Da_stage1_load_prefixes_escapes_opcode (p, adr_of_ins, &opc)==false)
    {
        if (dbg_print)
            printf ("%s(): Da_stage1_load_prefixes_escapes_opcode() failed\n", __func__);
        return false;
    };

    if (precomputed_ins_pointers_present==false)
        precompute_ins_pointers();

    //p->tbl_p=0;
    p->tbl_p=precomputed_ins_pointers[opc];

    while (ins_tbl[p->tbl_p].ins_code!=I_INVALID)
    {
        p->new_flags=ins_tbl[p->tbl_p].flags;

        if (p->REX_W && IS_SET(p->new_flags, F_REXW_PROMOTE_ALL_32_OPS_TO_64))
            p->new_flags=promote_32_flags_to_64 (p->new_flags);

        if ((IS_SET(p->new_flags, F_REXW_PRESENT) && p->REX_W==false) ||
                (IS_SET(p->new_flags, F_REXW_ABSENT) && p->REX_W==true))
        {
            p->tbl_p++;
            continue;
        };

        // need 0F prefix?
        if (IS_SET(p->new_flags, F_0F) ^ p->ESCAPE_0F)
        {
            p->tbl_p++;
            continue;
        };

        // need F3 prefix?
        if (IS_SET(p->new_flags, F_F3) ^ p->ESCAPE_F3)
        {
            p->tbl_p++;
            continue;
        };

        // need F2 prefix, but we haven't one
        if (IS_SET(p->new_flags, F_F2) ^ p->ESCAPE_F2)
        {
            p->tbl_p++;
            continue;
        };

        PREFIX66_may_present=IS_SET(p->new_flags, F_PREFIX66_ALLOWED) ||
            IS_SET(p->new_flags, F_PREFIX66_IS_PART_OF_OPCODE) ||
            IS_SET(p->new_flags, F_PREFIX66_APPLIED_TO_OP1_ONLY);

        if (p->PREFIX_66_is_present && PREFIX66_may_present==false)
        {
            p->tbl_p++;
            continue;
        };

        PREFIX66_allowed_and_present=p->PREFIX_66_is_present && IS_SET(p->new_flags, F_PREFIX66_ALLOWED);

        if (IS_SET(p->new_flags, F_PREFIX66_IS_PART_OF_OPCODE) && p->PREFIX_66_is_present==false)
        {
            p->tbl_p++;
            continue;
        };

        if ((p->x64 && IS_SET (p->new_flags, F_X32_ONLY)) ||
                ((p->x64==false) && IS_SET (p->new_flags, F_X64_ONLY)))
        {
            p->tbl_p++;
            continue;
        };

        mask=0xFF;

        if (IS_SET (p->new_flags, F_REG32_IS_LOWEST_PART_OF_1ST_BYTE))
            mask=0xF8;
        if (IS_SET (p->new_flags, F_REG64_IS_LOWEST_PART_OF_1ST_BYTE))
            mask=0xF8;

        if ((opc & mask) != ins_tbl[p->tbl_p].opc)
        {
            p->tbl_p++;
            continue;
        };

        // load second opcode if needed
        if (IS_SET (p->new_flags, F_OPC2))
        {
            uint8_t opc2;
            if (Da_stage1_get_next_byte(p, &opc2)==false)
            {
                if (dbg_print)
                    printf ("%s() line %d\n", __func__, __LINE__);
                return false;
            };
            if (opc2!=ins_tbl[p->tbl_p].opc2)
            {
                Da_stage1_unget_byte(p);
                p->tbl_p++;
                continue;
            };
            // second opcode is correct here
        };

        // here we found correct tbl_p
        // load everything

        p->len++;

        p->ins_code=ins_tbl[p->tbl_p].ins_code;

#ifdef _DEBUG
        SET_BIT(ins_tbl[p->tbl_p].flags, F_HIT_DURING_EXECUTION);
#endif
        // let's load MODRM
        if (p->new_flags & F_MODRM)
        {
            //uint8_t MODRM_BYTE=Da_stage1_get_next_byte(p); 
            if (Da_stage1_get_next_byte(p, &p->MODRM.as_byte)==false)
            {
                if (dbg_print)
                    printf ("%s() line %d\n", __func__, __LINE__);
                return false;
            };
            p->len++;

            //printf ("MODRM=0x%02X MOD=%d REG=%d RM=%d\n", p->MODRM.as_byte, p->MODRM.s.MOD, p->MODRM.s.REG, p->MODRM.s.RM);
            if (
                    (IS_SET(p->new_flags, F_MODRM_REG_0) && p->MODRM.s.REG!=0) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_1) && p->MODRM.s.REG!=1) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_2) && p->MODRM.s.REG!=2) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_3) && p->MODRM.s.REG!=3) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_4) && p->MODRM.s.REG!=4) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_5) && p->MODRM.s.REG!=5) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_6) && p->MODRM.s.REG!=6) ||
                    (IS_SET(p->new_flags, F_MODRM_REG_7) && p->MODRM.s.REG!=7)
               )
            {
                Da_stage1_unget_byte(p); // push back mod/rm byte
                p->len--;
                p->len--;
                p->tbl_p++;
                //printf ("non-suitable MODRM_REG=%d, new_flags=0x%016X\n", MODRM_REG, new_flags);
                continue;
            };

            if (
                    (IS_SET(p->new_flags, F_MODRM_RM_0) && p->MODRM.s.RM!=0) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_1) && p->MODRM.s.RM!=1) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_2) && p->MODRM.s.RM!=2) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_3) && p->MODRM.s.RM!=3) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_4) && p->MODRM.s.RM!=4) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_5) && p->MODRM.s.RM!=5) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_6) && p->MODRM.s.RM!=6) ||
                    (IS_SET(p->new_flags, F_MODRM_RM_7) && p->MODRM.s.RM!=7)
               )
            {
                Da_stage1_unget_byte(p); // push back mod/rm byte
                p->len--;
                p->len--;
                p->tbl_p++;
                continue;
            };

            if (
                    (IS_SET(p->new_flags, F_MODRM_MOD_IS_3) && p->MODRM.s.MOD!=3) ||
                    (IS_SET(p->new_flags, F_MODRM_MOD_IS_NOT_3) && p->MODRM.s.MOD==3)
               )
            {
                Da_stage1_unget_byte(p); // push back mod/rm byte
                p->len--;
                p->len--;
                p->tbl_p++;
                continue;
            };

            // MODRM is loaded here

            p->MODRM_loaded=true;

            if (p->PREFIX_67==false)
            { // PREFIX_67 absent, 32-bit uint64_ting, second version of modrm table

                // SIB is present?

                if ((p->MODRM.s.MOD==0 && p->MODRM.s.RM==4) || 
                        (p->MODRM.s.MOD==1 && p->MODRM.s.RM==4) || 
                        (p->MODRM.s.MOD==2 && p->MODRM.s.RM==4))
                {
                    if (Da_stage1_get_next_byte(p, &p->SIB.as_byte)==false)
                    {
                        if (dbg_print)
                            printf ("%s() line %d\n", __func__, __LINE__);
                        return false;
                    };
                    p->len++;
                    p->SIB_loaded=true;
                };

                // DISP is present?

                if (p->MODRM.s.MOD==1) // DISP8 present, read it
                {
                    if (Da_stage1_get_next_byte(p, (uint8_t*)&p->DISP8)==false)
                    {
                        if (dbg_print)
                            printf ("%s() line %d\n", __func__, __LINE__);
                        return false;
                    };
                    p->len++;
                    p->DISP8_loaded=true;
                };

                if ((p->MODRM.s.MOD==0 && p->MODRM.s.RM==5) || 
                        p->MODRM.s.MOD==2 || 
                        (p->MODRM.s.MOD==0 && p->SIB_loaded==true && p->SIB.s.base==5)) // DISP32 present, read it
                {
                    if (Da_stage1_get_next_dword(p, (uint32_t*)&p->DISP32)==false)
                    {
                        if (dbg_print)
                            printf ("%s() line %d\n", __func__, __LINE__);
                        return false;
                    };
                    p->DISP32_pos=p->cur_adr-sizeof(uint32_t);
                    p->len+=4;
                    p->DISP32_loaded=true;
                };
            } 
            else
            { // PREFIX_67 present, 16-bit ...ing mode, first version of modrm table

                if (p->MODRM.s.MOD==1) // DISP8 present, read it
                {
                    if (Da_stage1_get_next_byte(p, (uint8_t*)&p->DISP8)==false)
                    {
                        if (dbg_print)
                            printf ("%s() line %d\n", __func__, __LINE__);
                        return false;
                    };
                    p->len++;
                    p->DISP8_loaded=true;
                };

                if ((p->MODRM.s.MOD==0 && p->MODRM.s.RM==6) || p->MODRM.s.MOD==2) // DISP16 present, read it
                {
                    if (Da_stage1_get_next_word(p, (uint16_t*)&p->DISP16)==false)
                    {
                        if (dbg_print)
                            printf ("%s() line %d\n", __func__, __LINE__);
                        return false;
                    };
                    p->len+=2;
                    p->DISP16_loaded=true;
                };
            };
        };

        // IMM8/16/32 is present? load if so

        // порядок загрузки 16, а потом 8, должен соблюдаться чтобы ENTER верно дизасмился
        if ((p->new_flags & F_IMM16) || (PREFIX66_allowed_and_present && (p->new_flags & F_IMM32)))
        {
            if (Da_stage1_get_next_word(p, &p->IMM16)==false)
            {
                if (dbg_print)
                    printf ("%s() line %d\n", __func__, __LINE__);
                return false;
            };
            p->len+=2;
            p->IMM16_loaded=true;
        };

        if (p->new_flags & F_IMM8)
        {
            if (Da_stage1_get_next_byte(p, &p->IMM8)==false)
            {
                if (dbg_print)
                    printf ("%s() line %d\n", __func__, __LINE__);
                return false;
            };
            p->len++;
            p->IMM8_loaded=true;
        };

        if ((p->new_flags & F_IMM32) && PREFIX66_allowed_and_present==false)
        {
            oassert (PREFIX66_allowed_and_present==false);
            if (Da_stage1_get_next_dword(p, &p->IMM32)==false)
            {
                if (dbg_print)
                    printf ("%s() line %d\n", __func__, __LINE__);
                return false;
            };
            p->IMM32_pos=p->cur_adr-sizeof(uint32_t);
            p->len+=4;
            p->IMM32_loaded=true;
        };

        if ((p->new_flags & F_IMM64) && PREFIX66_allowed_and_present==false)
        {
            oassert (PREFIX66_allowed_and_present==false);
            if (Da_stage1_get_next_qword(p, &p->IMM64)==false)
            {
                if (dbg_print)
                    printf ("%s() line %d\n", __func__, __LINE__);
                return false;
            };
            //L ("IMM64=0x" PRI_REG_HEX "\n", IMM64);
            p->IMM64_pos=p->cur_adr-sizeof(uint64_t);
            p->len+=8;
            p->IMM64_loaded=true;
        };

        if (p->new_flags & F_PTR)
        {
            if (p->x64)
            {
                if (Da_stage1_get_next_qword(p, &p->PTR)==false)
                {
                    if (dbg_print)
                        printf ("%s() line %d\n", __func__, __LINE__);
                    return false;
                };
                p->PTR_pos=p->cur_adr-sizeof(uint64_t);
                p->len+=sizeof(uint64_t);
                p->PTR_loaded=true;
            }
            else
            {
                uint32_t tmp;
                if (Da_stage1_get_next_dword(p, &tmp)==false)
                {
                    if (dbg_print)
                        printf ("%s() line %d\n", __func__, __LINE__);
                    return false;
                };
                p->PTR=tmp;
                p->PTR_pos=p->cur_adr-sizeof(uint32_t);
                p->len+=4;
                p->PTR_loaded=true;
            };
        };

        if ((p->new_flags & F_REG32_IS_LOWEST_PART_OF_1ST_BYTE) || 
                (p->new_flags & F_REG64_IS_LOWEST_PART_OF_1ST_BYTE))
        {
            p->REG_FROM_LOWEST_PART_OF_1ST_BYTE=opc&7;
            p->REG_FROM_LOWEST_PART_OF_1ST_BYTE_loaded=true;
        };

        return true;
    };

    // opcode not found

    fprintf (stderr, "adr_of_ins=0x" PRI_SIZE_T_HEX " opcode not found, opc=%02X, p->x64=%d, prefixes=", 
            adr_of_ins, opc, p->x64);

    if (p->x64)   fprintf(stderr, "X64 ");
    if (p->REX_W) fprintf(stderr, "REX_W ");
    if (p->REX_R) fprintf(stderr, "REX_R ");
    if (p->REX_X) fprintf(stderr, "REX_X ");
    if (p->REX_B) fprintf(stderr, "REX_B ");

    if (p->ESCAPE_0F) fprintf(stderr, "0F ");
    if (p->ESCAPE_F2) fprintf(stderr, "F2 ");
    if (p->ESCAPE_F3) fprintf(stderr, "F3 ");
    if (p->PREFIX_66_is_present) fprintf(stderr, "66 ");
    if (p->PREFIX_67) fprintf(stderr, "67 ");

    fprintf (stderr, "\n");

    return false;
};

static X86_register get_8bit_reg (int i, bool replace_xH_to_xPL_and_xIL)
{
    if (replace_xH_to_xPL_and_xIL)
    {
        switch (i)
        {
            case 4: return R_SPL;
            case 5: return R_BPL;
            case 6: return R_SIL;
            case 7: return R_DIL;
        };
    }

    if (i>15)
        fatal_error();
    return _8_registers_by_idx[i];
};

static void decode_SIB (Da_stage1 *stage1,
        X86_register * adr_base, X86_register * adr_index, unsigned * adr_index_mult, int64_t * adr_disp, uint8_t * adr_disp_width_in_bits, disas_address *adr_disp_pos)
{
#if 0
    cout << strfmt ("%s(): MOD=%02X, SIB_scale=%02X, SIB_index=%02X, SIB_base=%02X, DISP32_loaded=%d\n", __FUNCTION__, MOD, SIB_scale, SIB_index, SIB_base, DISP32_loaded) << endl;
#endif

    switch (stage1->SIB.s.base)
    {
        case 5: // [*] case
            if (stage1->MODRM.s.MOD!=0)
            {
                if (stage1->x64)
                {
                    if (stage1->REX_B)
                        *adr_base=R_R13;
                    else
                        *adr_base=R_RBP;
                }
                else
                    *adr_base=R_EBP;
            }
            else
            {
                *adr_base=R_ABSENT;
            };
            break;
        case 0 ... 4: 
        case 6:
        case 7: 
            if (stage1->x64)
                *adr_base=_64_registers_by_idx[(stage1->REX_B ? 0x8 : 0) | stage1->SIB.s.base]; 
            else
                *adr_base=_32_registers_by_idx[stage1->SIB.s.base];
            break;
        default: 
            oassert(0); 
            fatal_error();
    };

    switch (stage1->SIB.s.scale)
    {
        case 0: *adr_index_mult=1; break;
        case 1: *adr_index_mult=2; break;
        case 2: *adr_index_mult=4; break;
        case 3: *adr_index_mult=8; break;
        default: 
                oassert(0);
                fatal_error();
    };

    switch (stage1->SIB.s.index)
    {
        case 4:
            if (stage1->x64)
            {
                int tmp=(stage1->REX_X ? 0x8 : 0) | stage1->SIB.s.index;
                if (tmp==4)
                    *adr_index=R_ABSENT;
                else
                    *adr_index=_64_registers_by_idx[tmp];
            }
            else
                *adr_index=R_ABSENT;
            break;
        case 0 ... 3: 
        case 5 ... 7:
            if (stage1->x64)
                *adr_index=_64_registers_by_idx[(stage1->REX_X ? 0x8 : 0) | stage1->SIB.s.index];
            else
                *adr_index=_32_registers_by_idx[stage1->SIB.s.index];
            break;
        default: 
            oassert(0); 
            fatal_error();
    };

    if (stage1->SIB.s.base==5 && stage1->MODRM.s.MOD==0)
    {
        oassert (stage1->DISP32_loaded==true);
        *adr_disp_width_in_bits=32;
        *adr_disp=stage1->DISP32;
        oassert (stage1->DISP32_pos!=0);
        *adr_disp_pos=stage1->DISP32_pos;
    };

#if 0
    cout << strfmt ("scale_i=%d *adr_index_mult=%d", scale_i, *adr_index_mult) << endl;
    cout << "*adr_base=" << adr_base->ToString() << endl;
    cout << "*adr_index=" << adr_index->ToString() << endl;
#endif
};

// FIXME: should be optimized...
void init_adr_in_Da_op (Da_op *out)
{
    out->adr.adr_base=0;
    out->adr.adr_index=0;
    out->adr.adr_index_mult=1; // default value
    out->adr.adr_disp=0;
    out->adr.adr_disp_is_absolute=0;
    out->adr.adr_disp_is_not_negative=0;
    out->adr.adr_disp_pos=0;
};

static bool create_Da_op (op_source op, Da_stage1 *stage1, disas_address ins_adr, unsigned ins_len, Da_op *out)
{
    oassert (op!=OP_ABSENT);

    switch (op)
    {
        case OP_REG64_FROM_LOWEST_PART_OF_1ST_BYTE:
            out->type=DA_OP_TYPE_REGISTER; 
            out->value_width_in_bits=64;
            out->reg=_64_registers_by_idx[(stage1->REX_B ? 8 : 0) | stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE];
            break;

        case OP_REG32_FROM_LOWEST_PART_OF_1ST_BYTE:
            out->type=DA_OP_TYPE_REGISTER; 

            if (stage1->PREFIX_66_is_present)
            {
                out->value_width_in_bits=16;
                out->reg=_16_registers_by_idx[stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE];
            }
            else
            {
                out->value_width_in_bits=32;
                if (stage1->x64)
                    out->reg=_32_registers_by_idx[(stage1->REX_B ? 8 : 0) | stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE];
                else
                    out->reg=_32_registers_by_idx[stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE];
            };
            break;

        case OP_REG8_FROM_LOWEST_PART_OF_1ST_BYTE:
            out->type=DA_OP_TYPE_REGISTER; 
            out->value_width_in_bits=8;
            if (stage1->x64)
                out->reg=get_8bit_reg ((stage1->REX_B ? 8 : 0) | stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE, stage1->REX_prefix_seen);
            else
                out->reg=get_8bit_reg (stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE, false);
            break;

        case OP_1:
            if (value_in(stage1->ins_code, I_ROL, I_ROR, I_RCL, I_RCR, I_SHL, I_SHR, I_SAR))
            {
                out->type=DA_OP_TYPE_VALUE;
                out->value_width_in_bits=8;
                obj_byte2 (1, &out->val._v);
            }
            else
            {
                out->type=DA_OP_TYPE_VALUE;
                out->value_width_in_bits=32; // FIXME: тут не всегда 32 бита
                obj_tetrabyte2 (1, &out->val._v);
            };
            break;

        case OP_AH: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_AH; break;
        case OP_AL: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_AL; break;
        case OP_BH: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_BH; break;
        case OP_BL: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_BL; break;
        case OP_CH: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_CH; break;
        case OP_CL: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_CL; break;
        case OP_DH: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_DH; break;
        case OP_DL: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8; out->reg=R_DL; break;

        case OP_AX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_AX; break;
        case OP_BX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_BX; break;
        case OP_CX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_CX; break;
        case OP_DX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_DX; break;

        case OP_SP: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_SP; break;
        case OP_BP: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_BP; break;
        case OP_SI: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_SI; break;
        case OP_DI: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_DI; break;

        case OP_EAX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_EAX; break;
        case OP_EBP: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_EBP; break;
        case OP_EBX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_EBX; break;
        case OP_ECX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_ECX; break;
        case OP_EDI: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_EDI; break;
        case OP_EDX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_EDX; break;
        case OP_ESI: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_ESI; break;
        case OP_ESP: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=32; out->reg=R_ESP; break;

        case OP_RAX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RAX; break;
        case OP_RBP: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RBP; break;
        case OP_RBX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RBX; break;
        case OP_RCX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RCX; break;
        case OP_RDI: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RDI; break;
        case OP_RDX: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RDX; break;
        case OP_RSI: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RSI; break;
        case OP_RSP: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64; out->reg=R_RSP; break;

        case OP_ST0: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST0; break;
        case OP_ST1: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST1; break;
        case OP_ST2: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST2; break;
        case OP_ST3: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST3; break;
        case OP_ST4: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST4; break;
        case OP_ST5: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST5; break;
        case OP_ST6: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST6; break;
        case OP_ST7: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80; out->reg=R_ST7; break;

        case OP_ES: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_ES; break;
        case OP_CS: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_CS; break;
        case OP_SS: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_SS; break;
        case OP_DS: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_DS; break;
        case OP_FS: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_FS; break;
        case OP_GS: out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; out->reg=R_GS; break;

        case OP_IMM8:
                    oassert (stage1->IMM8_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=8;
                    obj_byte2 (stage1->IMM8, &out->val._v);
                    break;

        case OP_IMM8_SIGN_EXTENDED_TO_IMM32:
                    oassert (stage1->IMM8_loaded==true); // dirty hack

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=32;
                    obj_tetrabyte2 ((int32_t)(int8_t)stage1->IMM8, &out->val._v);
                    break;

        case OP_IMM8_SIGN_EXTENDED_TO_IMM64:
                    oassert (stage1->IMM8_loaded==true); // dirty hack

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=64;
                    obj_octabyte2 ((int64_t)(int8_t)stage1->IMM8, &out->val._v);
                    break;

        case OP_IMM16_SIGN_EXTENDED_TO_IMM32:
                    oassert (stage1->IMM16_loaded==true); // dirty hack

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=32;
                    obj_tetrabyte2 ((int32_t)(int16_t)stage1->IMM16, &out->val._v);
                    break;

        case OP_IMM16_SIGN_EXTENDED_TO_IMM64:
                    oassert (stage1->IMM16_loaded==true); // dirty hack

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=64;
                    obj_octabyte2 ((int64_t)(int16_t)stage1->IMM16, &out->val._v);
                    break;

        case OP_IMM32_SIGN_EXTENDED_TO_IMM64:
                    oassert (stage1->IMM32_loaded==true); // dirty hack

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=64;

                    //L ("stage1.IMM32 = %08X, %d\n", stage1.IMM32, (int32_t)stage1.IMM32);

                    if ((int32_t)stage1->IMM32>=0)
                    {
                        //L ("p1\n");
                        obj_octabyte2 ((uint64_t)stage1->IMM32, &out->val._v);
                    }
                    else
                    {
                        //L ("p2\n");
                        obj_octabyte2 ((uint64_t)(stage1->IMM32 | 0xFFFFFFFF00000000), &out->val._v);
                    }
                    break;

        case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_BYTE:
        case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_DWORD:

                    if (stage1->IMM64_loaded==false)
                        return false; // yet. it's a hack!
                    oassert (stage1->IMM64_loaded==true);
                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->adr.adr_disp_width_in_bits=64;
                    out->adr.adr_disp=stage1->IMM64;
                    //L ("stage1.IMM64=0x" PRI_REG_HEX "\n", stage1.IMM64);
                    out->adr.adr_disp_is_absolute=true;
                    oassert (stage1->IMM64_pos!=0);
                    out->adr.adr_disp_pos=stage1->IMM64_pos;
                    switch (op)
                    {
                        case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_BYTE: out->value_width_in_bits=8; break;
                        case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_DWORD: out->value_width_in_bits=32; break;
                        default: oassert(0); fatal_error();
                    };
                    break;

        case OP_MOFFS32:
        case OP_MOFFS16:
        case OP_MOFFS8:

                    oassert (stage1->PTR_loaded);
                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->adr.adr_disp=stage1->PTR;
                    out->adr.adr_disp_is_absolute=true;
                    oassert (stage1->PTR_pos!=0);
                    out->adr.adr_disp_pos=stage1->PTR_pos;

                    if (stage1->x64)
                        out->adr.adr_disp_width_in_bits=64;
                    else
                        out->adr.adr_disp_width_in_bits=32;

                    switch (op)
                    {
                        case OP_MOFFS32: out->value_width_in_bits=32; break;
                        case OP_MOFFS16: out->value_width_in_bits=16; break;
                        case OP_MOFFS8:  out->value_width_in_bits=8; break;
                        default: oassert(0); fatal_error();
                    };
                    break;

        case OP_IMM8_SIGN_EXTENDED_TO_IMM16:
                    oassert (stage1->IMM8_loaded==true); // dirty hack

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=16;
                    obj_wyde2 ((int16_t)(int8_t)stage1->IMM8, &out->val._v);
                    break;


        case OP_IMM8_AS_REL32:
                    oassert (stage1->IMM8_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=32;
                    obj_tetrabyte2 ((int32_t)(ins_adr + ins_len)+(int8_t)stage1->IMM8, &out->val._v);
                    break;

        case OP_IMM8_AS_REL64:
                    oassert (stage1->IMM8_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=64;
                    obj_octabyte2 ((int64_t)(ins_adr + ins_len)+(int8_t)stage1->IMM8, &out->val._v);
                    break;

        case OP_IMM16:
                    oassert (stage1->IMM16_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=16;
                    obj_wyde2 (stage1->IMM16, &out->val._v);
                    break;

        case OP_IMM32:
                    oassert (stage1->IMM32_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=32;
                    obj_tetrabyte2 (stage1->IMM32, &out->val._v);
                    oassert (stage1->IMM32_pos!=0);
                    out->val.value32_pos=stage1->IMM32_pos;

                    break;

        case OP_IMM64:
                    oassert (stage1->IMM64_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=64;
                    obj_octabyte2 (stage1->IMM64, &out->val._v);
                    oassert (stage1->IMM64_pos!=0);
                    out->val.value64_pos=stage1->IMM64_pos;

                    break;

        case OP_IMM32_AS_OFS32:
                    oassert (stage1->IMM32_loaded==true);

                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->value_width_in_bits=32;
                    out->adr.adr_disp_width_in_bits=32;
                    out->adr.adr_disp=stage1->IMM32;
                    oassert (stage1->IMM32_pos!=0);
                    out->adr.adr_disp_pos=stage1->IMM32_pos;

                    break;

        case OP_IMM64_AS_OFS32:
                    oassert (stage1->IMM64_loaded==true);

                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->value_width_in_bits=32;
                    out->adr.adr_disp_width_in_bits=64;
                    out->adr.adr_disp=stage1->IMM64;
                    oassert (stage1->IMM64_pos!=0);
                    out->adr.adr_disp_pos=stage1->IMM64_pos;

                    break;

        case OP_IMM32_AS_OFS16:
                    if (stage1->IMM32_loaded==false)
                    {
                        L ("stage1->IMM32_loaded==false\n");
                        L ("instruction:\n");
                        Da_stage1_dump(stage1, ins_adr, ins_len);
                        L ("ins_adr=0x" PRI_SIZE_T_HEX "\n", ins_adr);
                        //if (p!=NULL) 
                        //    cout << "sym=" << p->symbols->get_sym (ins_adr) << endl;
                        L("exiting\n");
                        exit(0);
                    };

                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->value_width_in_bits=16;
                    out->adr.adr_disp_width_in_bits=32;
                    out->adr.adr_disp=stage1->IMM32;
                    oassert (stage1->IMM32_pos!=0);
                    out->adr.adr_disp_pos=stage1->IMM32_pos;

                    break;

        case OP_IMM32_AS_OFS8:
                    oassert (stage1->IMM32_loaded==true);

                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->value_width_in_bits=8;
                    out->adr.adr_disp=stage1->IMM32;
                    out->adr.adr_disp_width_in_bits=32;
                    oassert (stage1->IMM32_pos!=0);
                    out->adr.adr_disp_pos=stage1->IMM32_pos;

                    break;

        case OP_IMM64_AS_OFS8:
                    oassert (stage1->IMM64_loaded==true);

                    out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    init_adr_in_Da_op(out);
                    out->value_width_in_bits=8;
                    out->adr.adr_disp=stage1->IMM64;
                    out->adr.adr_disp_width_in_bits=64;
                    oassert (stage1->IMM64_pos!=0);
                    out->adr.adr_disp_pos=stage1->IMM64_pos;

                    break;

        case OP_IMM32_AS_REL32:
                    oassert (stage1->IMM32_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=32;

                    obj_tetrabyte2 ((int32_t)(ins_adr+ins_len)+(int32_t)stage1->IMM32, &out->val._v);
                    oassert (stage1->IMM32_pos!=0);
                    out->val.value32_pos=stage1->IMM32_pos;

                    break;

        case OP_IMM32_SIGN_EXTENDED_TO_REL64:
                    oassert (stage1->IMM32_loaded==true);

                    out->type=DA_OP_TYPE_VALUE;
                    out->value_width_in_bits=64;

                    obj_octabyte2 ((int64_t)(ins_adr+ins_len)+(int64_t)((int32_t)stage1->IMM32), &out->val._v);
                    oassert (stage1->IMM32_pos!=0);
                    out->val.value32_pos=stage1->IMM32_pos;

                    break;

        case OP_MODRM_R32:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; 
                    out->value_width_in_bits=32;
                    out->reg=_32_registers_by_idx[(stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG];
                    break;

        case OP_MODRM_R64:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; 
                    out->value_width_in_bits=64;
                    out->reg=_64_registers_by_idx[(stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG];
                    break;

        case OP_MODRM_R16:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; 
                    if (stage1->REX_prefix_seen)
                        out->reg=_16_registers_by_idx[(stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG];
                    else
                        out->reg=_16_registers_by_idx[stage1->MODRM.s.REG];
                    break;

        case OP_MODRM_SREG:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16; 
                    switch (stage1->MODRM.s.REG)
                    {
                        case 0: out->reg=R_ES; break;
                        case 1: out->reg=R_CS; break;
                        case 2: out->reg=R_SS; break;
                        case 3: out->reg=R_DS; break;
                        case 4: out->reg=R_FS; break;
                        case 5: out->reg=R_GS; break;
                        case 6: return false; // oassert(0); fatal_error(); break; // reserved
                        case 7: return false; // oassert(0); fatal_error(); break; // reserved
                    }
                    break;

        case OP_MODRM_R8:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8;
                    if (stage1->x64)
                        out->reg=get_8bit_reg ((stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG, stage1->REX_prefix_seen);
                    else
                        out->reg=get_8bit_reg (stage1->MODRM.s.REG, false);
                    break;

        case OP_MODRM_R_XMM:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; 
                    out->value_width_in_bits=128;
                    if (stage1->REX_prefix_seen)
                        out->reg=XMM_registers_by_idx[(stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG];
                    else
                        out->reg=XMM_registers_by_idx[stage1->MODRM.s.REG];
                    break;

        case OP_MODRM_R_MM:
                    oassert (stage1->MODRM_loaded==true);
                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64;
                    switch (stage1->MODRM.s.REG)
                    {
                        case 0: out->reg=R_MM0; break;
                        case 1: out->reg=R_MM1; break;
                        case 2: out->reg=R_MM2; break;
                        case 3: out->reg=R_MM3; break;
                        case 4: out->reg=R_MM4; break;
                        case 5: out->reg=R_MM5; break;
                        case 6: out->reg=R_MM6; break;
                        case 7: out->reg=R_MM7; break;
                    };
                    break;

        case OP_MODRM_RM64:
        case OP_MODRM_RM32:
        case OP_MODRM_RM16:
        case OP_MODRM_RM8:
        case OP_MODRM_RM_XMM:
        case OP_MODRM_RM_MM:
        case OP_MODRM_RM_M64FP:
                    oassert (stage1->MODRM_loaded==true);
                    switch (stage1->MODRM.s.MOD)
                    {
                        case 0: // mod=0

                            out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                            init_adr_in_Da_op(out);

                            switch (op)
                            {
                                case OP_MODRM_RM64:   out->value_width_in_bits=64; break;
                                case OP_MODRM_RM32:   out->value_width_in_bits=32; break;
                                case OP_MODRM_RM16:   out->value_width_in_bits=16; break;
                                case OP_MODRM_RM8:    out->value_width_in_bits=8; break;
                                case OP_MODRM_RM_MM:
                                case OP_MODRM_RM_M64FP:
                                                      out->value_width_in_bits=64; break; // 64 bit
                                case OP_MODRM_RM_XMM: out->value_width_in_bits=128; break; // 128 bit

                                default: oassert(0); fatal_error();
                            };

                            if (stage1->PREFIX_67==false)
                            { // PREFIX_67==false, take 32-bit part of modrm table

                                //cout << "stage1->MODRM_RM=" << (int)stage1->MODRM_RM << endl;

                                switch (stage1->MODRM.s.RM)
                                {
                                    case 4: // SIB often without disp32, but sometimes with disp32
                                        oassert (stage1->SIB_loaded==true);
                                        {
                                            decode_SIB (stage1,
                                                    &out->adr.adr_base,
                                                    &out->adr.adr_index,
                                                    &out->adr.adr_index_mult,
                                                    &out->adr.adr_disp,
                                                    &out->adr.adr_disp_width_in_bits,
                                                    &out->adr.adr_disp_pos);
                                            out->adr.adr_disp_is_not_negative=true;
                                        };

                                        break;
                                    case 5:  
                                        oassert (stage1->DISP32_loaded==true);
                                        //cout << hex << "stage1->DISP32=" << stage1->DISP32 << endl;
                                        if (stage1->x64)
                                        {
                                            out->adr.adr_disp_width_in_bits=64;
                                            out->adr.adr_disp=ins_adr + stage1->DISP32 + stage1->len;
                                            if (out->adr.adr_disp&0x80000000)
                                                out->adr.adr_disp|=0xFFFFFFFF00000000;
                                        }
                                        else
                                        {
                                            out->adr.adr_disp_width_in_bits=32;
                                            out->adr.adr_disp=stage1->DISP32;
                                        };
                                        oassert (stage1->DISP32_pos!=0);
                                        out->adr.adr_disp_pos=stage1->DISP32_pos;
                                        out->adr.adr_disp_is_not_negative=true;
                                        break; // EA is just disp32

                                    case 0 ... 3:
                                    case 6 ... 7: 
                                        //if (op==OP_MODRM_RM64)
                                        if (stage1->x64)
                                            out->adr.adr_base=_64_registers_by_idx[(stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM];
                                        else
                                            out->adr.adr_base=_32_registers_by_idx[stage1->MODRM.s.RM];
                                        break;
                                    default: oassert(0); fatal_error(); break;
                                };
                            } else
                            { // PREFIX_67==true, take 16-bit part of modrm table
                                switch (stage1->MODRM.s.RM)
                                {
                                    case 0 ... 5:
                                    case 7:  
                                        return false; // yet
                                        oassert (!"PREFIX_67=true, we don't process 16-bit part of modrm table (yet)");
                                    case 6: // take disp16
                                        oassert (stage1->DISP16_loaded==true);
                                        // на практике это только в случае FS:[..] вроде...

                                        out->type=DA_OP_TYPE_VALUE_IN_MEMORY; 
                                        //init_adr_in_Da_op(out);
                                        out->value_width_in_bits=32;
                                        break;
                                    default: oassert(0); fatal_error();
                                };
                            };
                            break;

                        case 1: // mod=1. [reg+disp8]

                            out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                            init_adr_in_Da_op(out);

                            switch (op)
                            {
                                case OP_MODRM_RM64:   out->value_width_in_bits=64; break;
                                case OP_MODRM_RM32:   out->value_width_in_bits=32; break;
                                case OP_MODRM_RM16:   out->value_width_in_bits=16; break;
                                case OP_MODRM_RM8:    out->value_width_in_bits=8; break;
                                case OP_MODRM_RM_MM:
                                case OP_MODRM_RM_M64FP:
                                                      out->value_width_in_bits=64; break; // 64 bit
                                case OP_MODRM_RM_XMM: out->value_width_in_bits=128; break; // 128 bit
                                default: oassert(0); fatal_error();
                            };

                            if (stage1->PREFIX_67==true) // not handling it yet
                                return false;
                            oassert (stage1->PREFIX_67==false); // yet...
                            switch (stage1->MODRM.s.RM)
                            {
                                case 4:  // SIB byte present, SIB+disp8; 
                                    oassert (stage1->SIB_loaded==true);
                                    oassert (stage1->DISP8_loaded==true);
                                    {
                                        decode_SIB (stage1,
                                                &out->adr.adr_base,
                                                &out->adr.adr_index,
                                                &out->adr.adr_index_mult,
                                                &out->adr.adr_disp,
                                                &out->adr.adr_disp_width_in_bits,
                                                &out->adr.adr_disp_pos);

                                        out->adr.adr_disp_width_in_bits=32;
                                        out->adr.adr_disp=(uint32_t)(int32_t)(int8_t)stage1->DISP8;

                                        out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                                        //init_adr_in_Da_op(out);
                                        switch (op)
                                        {
                                            case OP_MODRM_RM8:    out->value_width_in_bits=8; break;
                                            case OP_MODRM_RM16:   out->value_width_in_bits=16; break;
                                            case OP_MODRM_RM32:   out->value_width_in_bits=32; break;
                                            case OP_MODRM_RM64:   out->value_width_in_bits=64; break;
                                            case OP_MODRM_RM_MM:
                                            case OP_MODRM_RM_M64FP:
                                                                  out->value_width_in_bits=64; break;
                                            case OP_MODRM_RM_XMM: out->value_width_in_bits=128; break;
                                            default: 
                                                                  oassert(0); fatal_error();
                                                                  break;
                                        };
                                    };
                                    break;

                                case 0 ... 3:
                                case 5 ... 7:
                                    oassert (stage1->DISP8_loaded==true);
                                    //if (op==OP_MODRM_RM64)
                                    if (stage1->x64)
                                        out->adr.adr_base=_64_registers_by_idx[(stage1->REX_B ? 8 : 0) | stage1->MODRM.s.RM];
                                    else
                                        out->adr.adr_base=_32_registers_by_idx[stage1->MODRM.s.RM];

                                    out->adr.adr_disp_width_in_bits=32;
                                    out->adr.adr_disp=(uint32_t)(int32_t)(int8_t)stage1->DISP8;

                                    break;

                                default: oassert(0); fatal_error();
                            };
                            break;

                        case 2:  // mod=2. [reg+disp32]

                            out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                            init_adr_in_Da_op(out);

                            switch (op)
                            {
                                case OP_MODRM_RM64:   out->value_width_in_bits=64; break;
                                case OP_MODRM_RM32:   out->value_width_in_bits=32; break;
                                case OP_MODRM_RM16:   out->value_width_in_bits=16; break;
                                case OP_MODRM_RM8:    out->value_width_in_bits=8; break;
                                case OP_MODRM_RM_MM:
                                case OP_MODRM_RM_M64FP:
                                                      out->value_width_in_bits=64; break; // 64 bit
                                case OP_MODRM_RM_XMM: out->value_width_in_bits=128; break; // 128 bit
                                default: oassert(0); fatal_error();
                            };

                            if (stage1->PREFIX_67==true)
                                return false; // we don't handle it yet.
                            oassert (stage1->PREFIX_67==false); // yet...
                            switch (stage1->MODRM.s.RM)
                            {
                                case 4:  // SIB byte present, SIB+disp32; 
                                    oassert (stage1->SIB_loaded==true);
                                    oassert (stage1->DISP32_loaded==true);
                                    {
                                        decode_SIB (stage1,
                                                &out->adr.adr_base,
                                                &out->adr.adr_index,
                                                &out->adr.adr_index_mult,
                                                &out->adr.adr_disp,
                                                &out->adr.adr_disp_width_in_bits,
                                                &out->adr.adr_disp_pos);

                                        out->adr.adr_disp_width_in_bits=32;
                                        out->adr.adr_disp=stage1->DISP32; // bug was there
                                        out->adr.adr_disp_pos=stage1->DISP32_pos; // bug was there

                                        out->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                                        //init_adr_in_Da_op(out);
                                        switch (op)
                                        {
                                            case OP_MODRM_RM8:    out->value_width_in_bits=8; break;
                                            case OP_MODRM_RM16:   out->value_width_in_bits=16; break;
                                            case OP_MODRM_RM32:   out->value_width_in_bits=32; break;
                                            case OP_MODRM_RM64:   out->value_width_in_bits=64; break;
                                            case OP_MODRM_RM_MM:  
                                            case OP_MODRM_RM_M64FP:
                                                                  out->value_width_in_bits=64; break;
                                            case OP_MODRM_RM_XMM: out->value_width_in_bits=128; break;
                                            default: oassert(0); fatal_error();
                                        };
                                    };

                                    break;

                                case 0 ... 3:
                                case 5 ... 7:
                                    oassert (stage1->DISP32_loaded==true);
                                    if (stage1->x64)
                                        out->adr.adr_base=_64_registers_by_idx[(stage1->REX_B ? 8 : 0) | stage1->MODRM.s.RM];
                                    else
                                        out->adr.adr_base=_32_registers_by_idx[stage1->MODRM.s.RM];
                                    out->adr.adr_disp_width_in_bits=32;
                                    out->adr.adr_disp=stage1->DISP32;
                                    oassert (stage1->DISP32_pos!=0);
                                    out->adr.adr_disp_pos=stage1->DISP32_pos;
                                    break;

                                default: oassert(0); fatal_error();
                            };
                            break;

                        case 3:  // mod == 3, treat RM as register

                            switch (op)
                            {
                                case OP_MODRM_RM64:
                                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64;
                                    out->reg=_64_registers_by_idx[(stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM];
                                    break;

                                case OP_MODRM_RM32:

                                    out->type=DA_OP_TYPE_REGISTER;

                                    if (IS_SET(stage1->new_flags, F_WHEN_MOD3_TREAT_RM_AS_STx))
                                    {
                                        out->value_width_in_bits=80;
                                        out->reg=STx_registers_by_idx [stage1->MODRM.s.RM];
                                    }
                                    else
                                    {
                                        out->value_width_in_bits=32;
                                        //if (stage1->REX_prefix_seen)
                                        if (stage1->x64)
                                            out->reg=_32_registers_by_idx[(stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM];
                                        //reg=_64_registers_by_idx{(stage1->REX_B ? 0x8 : 0) | stage1->MODRM_RM];
                                        else
                                            out->reg=_32_registers_by_idx[stage1->MODRM.s.RM];
                                    };
                                    break;

                                case OP_MODRM_RM16:

                                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=16;

                                    if (stage1->x64)
                                        out->reg=_16_registers_by_idx[(stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM];
                                    else
                                        out->reg=_16_registers_by_idx[stage1->MODRM.s.RM];

                                    break;

                                case OP_MODRM_RM8:

                                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=8;

                                    if (stage1->x64)
                                        out->reg=get_8bit_reg ((stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM, stage1->REX_prefix_seen);
                                    else
                                        out->reg=get_8bit_reg (stage1->MODRM.s.RM, false);
                                    break;
                                case OP_MODRM_RM_XMM:

                                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=128;

                                    if (stage1->REX_prefix_seen)
                                        out->reg=XMM_registers_by_idx[(stage1->REX_B ? 8 : 0) | stage1->MODRM.s.RM];
                                    else
                                        out->reg=XMM_registers_by_idx[stage1->MODRM.s.RM];
                                    break;

                                case OP_MODRM_RM_MM:

                                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=64;

                                    switch (stage1->MODRM.s.RM)
                                    {
                                        case 0: out->reg=R_MM0; break;
                                        case 1: out->reg=R_MM1; break;
                                        case 2: out->reg=R_MM2; break;
                                        case 3: out->reg=R_MM3; break;
                                        case 4: out->reg=R_MM4; break;
                                        case 5: out->reg=R_MM5; break;
                                        case 6: out->reg=R_MM6; break;
                                        case 7: out->reg=R_MM7; break;
                                        default: oassert(0); fatal_error();
                                    };
                                    break;

                                case OP_MODRM_RM_M64FP:

                                    out->type=DA_OP_TYPE_REGISTER; out->value_width_in_bits=80;

                                    out->reg=STx_registers_by_idx[stage1->MODRM.s.RM];
                                    break;

                                default: oassert(0); fatal_error();
                            };
                    };

                    break;

        default:
                    L ("unknown op=%d\n", op);
                    oassert(0); fatal_error();
    };

    return true;
};

static op_source shrink_op_32_to_16 (op_source op)
{
    switch (op)
    {
        case OP_EAX: return OP_AX;
        case OP_EBX: return OP_BX;
        case OP_ECX: return OP_CX;
        case OP_EDX: return OP_DX;
        case OP_ESI: return OP_SI;
        case OP_EDI: return OP_DI;
        case OP_EBP: return OP_BP;
        case OP_ESP: return OP_SP;

        case OP_MODRM_R32: return OP_MODRM_R16;
        case OP_MODRM_RM32: return OP_MODRM_RM16;

        case OP_IMM8_SIGN_EXTENDED_TO_IMM32: return OP_IMM8_SIGN_EXTENDED_TO_IMM16;
        case OP_IMM32: return OP_IMM16;

        case OP_IMM32_AS_OFS32: return OP_IMM32_AS_OFS16;

        case OP_MOFFS32: return OP_MOFFS16;

        default: return op;
    };
};

static op_source sign_extend_op_32_to_64 (op_source op)
{
    switch (op)
    {
        case OP_IMM8_SIGN_EXTENDED_TO_IMM32: return OP_IMM8_SIGN_EXTENDED_TO_IMM64;
        case OP_IMM32_AS_REL32: return OP_IMM32_SIGN_EXTENDED_TO_REL64;
        case OP_IMM32: return OP_IMM32_SIGN_EXTENDED_TO_IMM64;
        default: return op;
    };
};

static op_source promote_op_32_to_64 (op_source op)
{
    switch (op)
    {
        case OP_EAX: return OP_RAX;
        case OP_EBX: return OP_RBX;
        case OP_ECX: return OP_RCX;
        case OP_EDX: return OP_RDX;
        case OP_ESI: return OP_RSI;
        case OP_EDI: return OP_RDI;
        case OP_EBP: return OP_RBP;
        case OP_ESP: return OP_RSP;

        case OP_MODRM_R32: return OP_MODRM_R64;
        case OP_MODRM_RM32: return OP_MODRM_RM64;

        case OP_IMM8_SIGN_EXTENDED_TO_IMM32: return OP_IMM8_SIGN_EXTENDED_TO_IMM64;
        case OP_IMM32: return OP_IMM64;

        case OP_REG32_FROM_LOWEST_PART_OF_1ST_BYTE: return OP_REG64_FROM_LOWEST_PART_OF_1ST_BYTE;
                                                    //case OP_REG32_FROM_LOWEST_PART_OF_2ND_BYTE: return OP_REG64_FROM_LOWEST_PART_OF_2ND_BYTE;

        case OP_IMM32_AS_REL32: return OP_IMM32_SIGN_EXTENDED_TO_REL64;

        case OP_IMM8_AS_REL32: return OP_IMM8_AS_REL64;
                               //case OP_IMM32_AS_OFS32: return OP_IMM32_AS_OFS16;

        default: return op;
    };
};

bool Da_stage1_into_result (Da_stage1 *stage1, disas_address adr_of_ins, Da* out)
{
    Ins_definition *i;
    uint64_t fl;
    op_source new_op1, new_op2, new_op3;

    out->ins_code=stage1->ins_code;
    out->ins_len=stage1->len;
    out->prefix_codes=stage1->PREFIXES;

    i=&ins_tbl[stage1->tbl_p];

    fl=stage1->new_flags; // including "promoted" flags

    new_op1=i->op1; new_op2=i->op2; new_op3=i->op3;

    if (stage1->PREFIX_66_is_present)
    {
        if (IS_SET(fl, F_PREFIX66_APPLIED_TO_OP1_ONLY) || (IS_SET(fl, F_PREFIX66_ALLOWED)))
            new_op1=shrink_op_32_to_16 (new_op1);

        if (IS_SET(fl, F_PREFIX66_ALLOWED))
        {
            new_op2=shrink_op_32_to_16 (new_op2);
            new_op3=shrink_op_32_to_16 (new_op3);
        };
    };

    if (IS_SET(fl, F_REXW_PROMOTE_ALL_32_OPS_TO_64) && stage1->REX_W)
    {
        new_op1=promote_op_32_to_64 (new_op1);
        new_op2=promote_op_32_to_64 (new_op2);
        new_op3=promote_op_32_to_64 (new_op3);
    };

    if ((IS_SET (fl, F_X64_PROMOTE_OP1_32_TO_64) && stage1->x64) ||
            (IS_SET (fl, F_REXW_PROMOTE_OP1_32_TO_64) && stage1->REX_W)) 
    {
        new_op1=promote_op_32_to_64 (new_op1);
    }

    if (IS_SET (fl, F_REXW_SIGN_EXTEND_OP2_32_TO_64) && stage1->REX_W)
    {
        new_op2=sign_extend_op_32_to_64 (new_op2);
    };

    out->ops_total=0;
    if (i->op1!=OP_ABSENT) 
    { 
        if (create_Da_op (new_op1, stage1, adr_of_ins, out->ins_len, &out->op[0])==false)
            return false;
        out->ops_total=1;
    }
    if (i->op2!=OP_ABSENT) 
    {
        if (create_Da_op (new_op2, stage1, adr_of_ins, out->ins_len, &out->op[1])==false)
            return false;
        out->ops_total=2;
    };
    if (i->op3!=OP_ABSENT)
    {
        if (create_Da_op (new_op3, stage1, adr_of_ins, out->ins_len, &out->op[2])==false)
            return false;
        out->ops_total=3;
    };

    if (out->ins_code==I_NOP && out->ops_total>0) // 0x1F case..
        out->ops_total=0;

    // NOP (0x90) is absent in tables...
    if (out->ins_code==I_XCHG && (Da_op_equals (&out->op[0], &out->op[1])))
    {
        out->ins_code=I_NOP;
        out->ops_total=0;
    };
    out->struct_size=sizeof(Da)-(3-out->ops_total)*sizeof(Da_op);
    return true;
};

bool Da_Da (TrueFalseUndefined x64_code, uint8_t* ptr_to_ins, disas_address adr_of_ins, Da* out)
{
    Da_stage1 stage1;

    stage1.use_callbacks=false;
    stage1.cur_ptr=ptr_to_ins;

    out->ins_code=I_INVALID;
    if (Da_stage1_Da_stage1(&stage1, x64_code, adr_of_ins)==false)
    {
#ifdef _DEBUG
        fprintf (stderr, "Da_stage1_Da_stage1() failed\n");
#endif
        return false;
    };
    
    return Da_stage1_into_result (&stage1, adr_of_ins, out);
};

bool Da_Da_callbacks (TrueFalseUndefined x64_code, disas_address adr_of_ins, 
        callback_read_byte rb, callback_read_word rw, callback_read_dword rd, callback_read_oword ro, 
        void *param, Da* out)
{
    Da_stage1 stage1;

    stage1.use_callbacks=true;
    stage1.read_byte_fn=rb;
    stage1.read_word_fn=rw;
    stage1.read_dword_fn=rd;
    stage1.read_oword_fn=ro;
    stage1.callback_param=param;

    out->ins_code=I_INVALID;
    if (Da_stage1_Da_stage1(&stage1, x64_code, adr_of_ins)==false)
        return false;

    return Da_stage1_into_result (&stage1, adr_of_ins, out);
};

bool Da_op_is_reg(Da_op *op, X86_register reg)
{
    return op->type==DA_OP_TYPE_REGISTER && op->reg==reg;
};

bool Da_is_MOV_EBP_ESP(Da *d)
{
    return (d->ins_code==I_MOV) && Da_op_is_reg(&d->op[0], R_EBP) && Da_op_is_reg(&d->op[1], R_ESP);
};

bool Da_is_PUSH_EBP(Da *d)
{
    return (d->ins_code==I_PUSH) && Da_op_is_reg(&d->op[0], R_EBP);
};

//#include "x86_disas.h"

//#include "utils.hpp"

//using namespace std;

// FIXME: переписать
bool Da_op_is_adr_disp_negative(Da_op *op)
{
    if (op->adr.adr_disp_width_in_bits==32)
        return (op->adr.adr_disp & 0x80000000)!=0;
    else if (op->adr.adr_disp_width_in_bits==64)
        return (op->adr.adr_disp & 0x8000000000000000)!=0;
    else
        return false;
};

void Da_op_ToString (Da_op* op, strbuf* out)
{
    bool something_added=false;

    oassert (op->type!=DA_OP_TYPE_ABSENT);

    switch (op->type)
    {
        case DA_OP_TYPE_REGISTER:
            strbuf_addstr(out, X86_register_ToString (op->reg));
            break;

        case DA_OP_TYPE_VALUE:
            // asm notation here
            strbuf_asmhex (out, zero_extend_to_octabyte (&op->val._v));
            break;

        case DA_OP_TYPE_VALUE_IN_MEMORY:
            strbuf_addc(out, '[');

            if (op->adr.adr_base != R_ABSENT)
            {
                strbuf_addstr (out, X86_register_ToString (op->adr.adr_base));
                something_added=true;
            };

            if (op->adr.adr_index!=R_ABSENT)
            {
                if (op->adr.adr_index_mult==1)
                {
                    if (something_added)
                        strbuf_addc(out, '+');
                    strbuf_addstr(out, X86_register_ToString (op->adr.adr_index));
                    something_added=true;
                }
                else
                {
                    if (something_added)
                        strbuf_addc(out, '+');
                    strbuf_addstr (out, X86_register_ToString (op->adr.adr_index));
                    strbuf_addf (out, "*%d", op->adr.adr_index_mult);
                    something_added=true;
                };
            };

            // FIXME: забыл, что тут надо было фиксить
            if (op->adr.adr_disp_is_not_negative==false && Da_op_is_adr_disp_negative(op) && op->adr.adr_disp_is_absolute==false)
            {
                if (something_added)
                    strbuf_addc (out, '-');

                if (op->adr.adr_disp_width_in_bits==32)
                    strbuf_asmhex(out, (~((uint32_t)op->adr.adr_disp))+1);
                else
                    strbuf_asmhex(out, (~op->adr.adr_disp)+1);
                //if (op.adr_disp_pos!=0)
                //	r+=strfmt ("(adr_disp_pos=0x%x)",op.adr_disp_pos);

            }
            else
            {
                if (op->adr.adr_disp!=0 || something_added==false)
                {
                    if (something_added)
                        strbuf_addc (out, '+');

                    if (op->adr.adr_disp_width_in_bits==32)
                        strbuf_asmhex(out, (uint32_t)op->adr.adr_disp);
                    else
                        strbuf_asmhex(out, op->adr.adr_disp);
                    //if (op.adr_disp_pos!=0)
                    //	r+=strfmt ("(adr_disp_pos=0x%x)",op.adr_disp_pos);
                };
            };
            strbuf_addc(out, ']');
            break;

        default:
            oassert(!"unknown op");
    };
};

void Da_op_DumpString (fds *s, Da_op* op)
{
    strbuf t=STRBUF_INIT;
    Da_op_ToString(op, &t);
    L_fds (s, t.buf);
    strbuf_deinit(&t);
};

bool Da_ins_is_Jcc (Da* d)
{
    // FIXME: there might be a flag in tbl...
    switch (d->ins_code)
    {
        case I_JA:
        case I_JB:
        case I_JBE:
        case I_JECXZ:
        case I_JG:
        case I_JGE:
        case I_JL:
        case I_JLE:
        case I_JNB:
        case I_JNO:
        case I_JNP:
        case I_JNS:
        case I_JNZ:
        case I_JO:
        case I_JP:
        case I_JS:
        case I_JZ:
            return true;
        default:
            return false;
    };
};

const char* disas1_ins_code_to_string (Ins_codes ins_code)
{
    int i;

    if (ins_code==I_NOP) // there are no NOP in tbl...
        return "NOP";

    for (i=0;;i++)
    {
        oassert (ins_tbl[i].ins_code!=I_INVALID); // ins_code not found in table
        if (ins_tbl[i].ins_code == ins_code)
            //return windows_ansi_to_tstring (ins_tbl[i].name);
            return ins_tbl[i].name;
    };
};

void Da_ToString (Da *d, strbuf *out)
{
    int i;

    if (IS_SET (d->prefix_codes, PREFIX_LOCK)) strbuf_addstr(out, "LOCK ");
    if (IS_SET (d->prefix_codes, PREFIX_FS)) strbuf_addstr(out, "FS: ");
    if (IS_SET (d->prefix_codes, PREFIX_SS)) strbuf_addstr(out, "SS: ");
    if (IS_SET (d->prefix_codes, PREFIX_GS)) strbuf_addstr(out, "GS: ");

    strbuf_addstr (out, disas1_ins_code_to_string (d->ins_code));

    for (i=0; i<d->ops_total; i++)
    {
        if (i!=0)
            strbuf_addc(out, ',');
        strbuf_addc(out, ' ');
        Da_op_ToString(&d->op[i], out);
    };
};

void Da_DumpString(fds* s, Da *d)
{
    strbuf t;
    strbuf_init(&t, 10);
    Da_ToString(d, &t);
    L_fds (s, "%s", t.buf);
    strbuf_deinit(&t);
};

bool Da_ins_is_FPU (Da *d)
{
    // FIXME: there might be a flag in tbl...
    switch (d->ins_code)
    {
        case I_FLD1:
        case I_FNSTENV:
        case I_FABS:
        case I_FADD:
        case I_FADDP:
        case I_FCHS:
        case I_FCOM:
        case I_FCOMP:
        case I_FCOMPP:
        case I_FDIV:
        case I_FDIVP:
        case I_FDIVR:
        case I_FDIVRP:
        case I_FILD:
        case I_FIST:
        case I_FISTP:
        case I_FLD:
        case I_FLDCW:
        case I_FMUL:
        case I_FMULP:
        case I_FNCLEX:
        case I_FNINIT:
        case I_FNSTCW:
        case I_FNSTSW:
        case I_FST:
        case I_FSTP:
        case I_FSTR:
        case I_FSTSW:
        case I_FSUB:
        case I_FSUBP:
        case I_FSUBR:
        case I_FSUBRP:
        case I_FXAM:
        case I_FXCH:
        case I_FXSAVE:
        case I_FUCOMPP:
        case I_FUCOMP:
            return true;

        default:
            return false;
    };
};

bool Da_op_equals(Da_op *op1, Da_op *op2)
{
    if (op1->type != op2->type) return false;
    if (op1->value_width_in_bits != op2->value_width_in_bits) return false;

    switch (op1->type)
    {
        case DA_OP_TYPE_ABSENT: 
            return true;

        case DA_OP_TYPE_REGISTER: 
            return op1->reg == op2->reg;

        case DA_OP_TYPE_VALUE:
            return EQL (&op1->val._v, &op2->val._v) && (op1->val.value32_pos == op2->val.value32_pos);

        case DA_OP_TYPE_VALUE_IN_MEMORY:
            if (op1->adr.adr_base != op2->adr.adr_base) return false;
            if (op1->adr.adr_index != op2->adr.adr_index) return false;
            if (op1->adr.adr_index_mult != op2->adr.adr_index_mult) return false;
            if (op1->adr.adr_disp_width_in_bits != op2->adr.adr_disp_width_in_bits) return false;
            if (op1->adr.adr_disp != op2->adr.adr_disp) return false;
            if (op1->adr.adr_disp_pos != op2->adr.adr_disp_pos) return false;
            return true; // а в остальном - здоров как бык!

        default:
            oassert(0);
            fatal_error();
    };
};

const char* Da_ins_code_ToString(Da *d)
{ 
    return disas1_ins_code_to_string (d->ins_code); 
};

#if 0
bool Da_op::is_EBP_plus_minus_X (address_offset & x) const // x can be negative/positive here
{
    if (type==DA_OP_TYPE_VALUE_IN_MEMORY &&
            adr_base==R_EBP &&
            adr_index!=R_ABSENT &&
            adr_index_mult==1)
    {
        x=(address_offset)adr_disp;
        return true;
    }
    else
        return false;
}
#endif

// FIXME: REG should be here? or obj?
bool Da_is_ADD_ESP_X (Da* d, uint32_t * out_X)
{
    if (d->ins_code!=I_ADD) return false;
    oassert (d->op[0].type==DA_OP_TYPE_REGISTER);
    if (d->op[0].reg != R_ESP) return false;
    if (d->op[1].type != DA_OP_TYPE_VALUE) return false;
    *out_X = obj_get_as_tetrabyte(&d->op[1].val._v);
    return true;
};

// FIXME: REG should be here? or obj?
bool Da_is_SUB_ESP_X (Da* d, uint32_t * out_X)
{
    if (d->ins_code!=I_SUB) return false;
    oassert (d->op[0].type==DA_OP_TYPE_REGISTER);
    if (d->op[0].reg != R_ESP) return false;
    if (d->op[1].type != DA_OP_TYPE_VALUE) return false;
    *out_X = obj_get_as_tetrabyte(&d->op[1].val._v);
    return true;
};

bool Da_is_RET (Da* d, uint16_t * out_X)
{
    if (d->ins_code!=I_RETN) return false;
    if (d->ops_total==1 && d->op[0].type==DA_OP_TYPE_VALUE)
        *out_X = obj_get_as_wyde(&d->op[0].val._v);
    else
        *out_X = 0;
    return true;
};

bool Da_2nd_op_is_disp_only (Da* d, uint64_t disp)
{
    if (d->ops_total<2)
        return false;
    if (d->op[1].adr.adr_base!=R_ABSENT)
        return false;
    if (d->op[1].adr.adr_index!=R_ABSENT)
        return false;
    if (d->op[1].adr.adr_disp!=disp)
        return false;

    return true;
};

/* vim: set expandtab ts=4 sw=4 : */
