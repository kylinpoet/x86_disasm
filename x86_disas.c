#include <assert.h>
#include "x86_disas.h"
#include "x86_disas_internals.h"
#include "dmalloc.h"
#include "memutils.h"
#include "bitfields.h"
#include "strbuf.h"
#include "fmt_utils.h"

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
    int tbl_entries_total=0;
    int i;

    printf (__FUNCTION__"()\n");

    for (i=0;;i++)
    {
        Ins_definition *d=&ins_tbl[i];

        if (d->ins_code==I_INVALID)
            break;
        if (IS_SET(d->flags, F_HIT_DURING_EXECUTION)==FALSE)
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

extern Ins_definition ins_tbl[]; // in x86_tbl.cpp file

static unsigned precomputed_ins_pointers[0x100]={0};
static BOOL precomputed_ins_pointers_present=FALSE;

static void precompute_ins_pointers()
{
    int i;
    unsigned cur_opc;
    Ins_definition *d;

    // check ins_tbl[] monotonicity
    for (i=0, cur_opc=0; ins_tbl[i].ins_code!=I_INVALID; i++)
    {
        d=&ins_tbl[i];
        if (d->opc < cur_opc)
        {
            assert (!"ins_tbl[] isn't monotonic");
        };

        if (d->opc > cur_opc)
            cur_opc=d->opc;
    };

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
    precomputed_ins_pointers_present=TRUE;
};

uint8_t Da_stage1_get_next_byte(Da_stage1* p)
{
    uint8_t rt;
    BOOL b;
    if (p->use_callbacks==FALSE)
    {
        p->cur_ptr++;
        p->cur_adr++;
        return *(p->cur_ptr-1);
    }
    else
    {
        p->cur_adr++;
        b=p->read_byte_fn(p->callback_param, p->cur_adr-1, &rt);
        if (b==FALSE)
            die (__FUNCTION__"(): can't read byte at 0x" PRI_SIZE_T_HEX "\n", p->cur_adr-1);
        return rt;
    };   
};

void Da_stage1_unget_byte(Da_stage1 *p)
{
    p->cur_ptr--;
    p->cur_adr--;
};

uint16_t Da_stage1_get_next_word(Da_stage1 *p)
{
    BOOL b;
    uint16_t rt;

    if (p->use_callbacks==FALSE)
    {
        p->cur_ptr+=sizeof(uint16_t);
        p->cur_adr+=sizeof(uint16_t); // will you need it?
        return *(uint16_t*)(p->cur_ptr-sizeof(uint16_t));
    } 
    else
    {
        p->cur_adr+=sizeof(uint16_t);
        b=p->read_word_fn(p->callback_param, p->cur_adr-sizeof(uint16_t), &rt);
        assert (b==TRUE);
        return rt;
    };
};

uint32_t Da_stage1_get_next_dword(Da_stage1 *p)
{
    uint32_t rt;
    BOOL b;

    if (p->use_callbacks==FALSE)
    {
        p->cur_ptr+=sizeof(uint32_t);
        p->cur_adr+=sizeof(uint32_t); // will you need it?
        return *(uint32_t*)(p->cur_ptr-sizeof(uint32_t));
    }
    else
    {
        p->cur_adr+=sizeof(uint32_t);
        b=p->read_dword_fn(p->callback_param, p->cur_adr-sizeof(uint32_t), &rt);
        assert (b==TRUE);
        return rt;
    };
};

uint64_t Da_stage1_get_next_qword (Da_stage1 *p)
{
    uint64_t rt;
    BOOL b;

    if (p->use_callbacks==FALSE)
    {
        p->cur_ptr+=sizeof(uint64_t);
        p->cur_adr+=sizeof(uint64_t); // will you need it?
        return *(uint64_t*)(p->cur_ptr-sizeof(uint64_t));
    }
    else
    {
        p->cur_adr+=sizeof(uint64_t);
        b=p->read_oword_fn(p->callback_param, p->cur_adr-sizeof(uint64_t), &rt);
        assert (b==TRUE);
        return rt;
    };   
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

uint8_t Da_stage1_load_prefixes_escapes_opcode (Da_stage1 *p, disas_address adr_of_ins)
{
    BOOL got_prefix=TRUE;

    // grp 1/2/3/4 prefixes
    uint8_t next_byte=Da_stage1_get_next_byte(p);

    while (got_prefix)
    {
        got_prefix=TRUE;
        switch (next_byte)
        {
        case 0x36:
            p->PREFIXES|=PREFIX_SS; 
            p->len++; 
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0x64:
            p->PREFIXES|=PREFIX_FS; 
            p->len++;
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0x65:
            p->PREFIXES|=PREFIX_GS; 
            p->len++; 
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0xF0:
            p->PREFIXES|=PREFIX_LOCK; 
            p->len++; 
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0xF2:
            p->ESCAPE_F2=TRUE; 
            p->len++;
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0xF3:
            p->ESCAPE_F3=TRUE; 
            p->len++;
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0x66:
            p->PREFIX_66_is_present=TRUE;
            p->len++;
            next_byte=Da_stage1_get_next_byte(p);
            break;
        case 0x67:
            p->PREFIX_67=TRUE;
            p->len++;
            next_byte=Da_stage1_get_next_byte(p);
            break;
        default:
            got_prefix=FALSE;
            break;
        };
    };

    if (p->x64)
    {
        // ищем 4x-префиксы
        if ((next_byte&0xF0)==0x40)
        {
            p->REX_W=p->REX_R=p->REX_X=p->REX_B=FALSE;

            if (IS_SET (next_byte, 1<<3)) p->REX_W=TRUE;
            if (IS_SET (next_byte, 1<<2)) p->REX_R=TRUE;
            if (IS_SET (next_byte, 1<<1)) p->REX_X=TRUE;
            if (IS_SET (next_byte, 1<<0)) p->REX_B=TRUE;

            //printf ("got 0x%02x prefix. REX_W=%d REX_R=%d REX_X=%d REX_B=%d\n", next_byte, REX_W, REX_R, REX_X, REX_B);

            p->REX_prefix_seen=TRUE;
            p->len++;
            next_byte=Da_stage1_get_next_byte(p);
            if ((next_byte&0xF0)==0x40)
            {
                assert (!"second 4x prefix present!");
            };
        };
    };
    
    if (next_byte==0x0F)
    {
        p->ESCAPE_0F=TRUE;
        p->len++;
        next_byte=Da_stage1_get_next_byte(p);
    };

    return next_byte; // opcode
};

void Da_stage1_dump (Da_stage1 *p, disas_address adr, int len)
{ 
    //if (p->from_mem_or_from_MemoryCache==FALSE)
    {
        assert(!"not implemented");
    }
#if 0
    else
    {
        BOOL b=MC_L_print_buf_in_mem (p->mem, adr, len);
        assert (b==TRUE);
    };
#endif
};

// только эта часть дизасма что-то вытягивает из памяти
BOOL Da_stage1_Da_stage1 (Da_stage1 *p, TrueFalseUndefined x64_code, disas_address adr_of_ins)
{
    uint8_t opc, mask;
    BOOL PREFIX66_may_present, PREFIX66_allowed_and_present;

    //printf (__FUNCTION__"()\n");

    p->cur_adr=adr_of_ins;

    if (x64_code==Fuzzy_Undefined)
    {
#ifdef _WIN64
        p->x64=TRUE;
#else
        p->x64=FALSE;
#endif
    }
    else if (x64_code==Fuzzy_True)
        p->x64=TRUE;
    else
        p->x64=FALSE;

    opc=Da_stage1_load_prefixes_escapes_opcode (p, adr_of_ins);

    if (precomputed_ins_pointers_present==FALSE)
        precompute_ins_pointers();

    //p->tbl_p=0;
    p->tbl_p=precomputed_ins_pointers[opc];

    while (ins_tbl[p->tbl_p].ins_code!=I_INVALID)
    {
        p->new_flags=ins_tbl[p->tbl_p].flags;

        if (p->REX_W && IS_SET(p->new_flags, F_REXW_PROMOTE_ALL_32_OPS_TO_64))
        {
            p->new_flags=promote_32_flags_to_64 (p->new_flags);
        };

        if ((IS_SET(p->new_flags, F_REXW_PRESENT) && p->REX_W==FALSE) ||
                (IS_SET(p->new_flags, F_REXW_ABSENT) && p->REX_W==TRUE))
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

        if (p->PREFIX_66_is_present && PREFIX66_may_present==FALSE)
        {
            p->tbl_p++;
            continue;
        };

        PREFIX66_allowed_and_present=p->PREFIX_66_is_present && IS_SET(p->new_flags, F_PREFIX66_ALLOWED);

        if (IS_SET(p->new_flags, F_PREFIX66_IS_PART_OF_OPCODE) && p->PREFIX_66_is_present==FALSE)
        {
            p->tbl_p++;
            continue;
        };

        if ((p->x64 && IS_SET (p->new_flags, F_X32_ONLY)) ||
                ((p->x64==FALSE) && IS_SET (p->new_flags, F_X64_ONLY)))
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
            uint8_t opc2=Da_stage1_get_next_byte(p);
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
            p->MODRM.as_byte=Da_stage1_get_next_byte(p); 
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

            p->MODRM_loaded=TRUE;

            if (p->PREFIX_67==FALSE)
            { // PREFIX_67 absent, 32-bit uint64_ting, second version of modrm table

                // SIB is present?

                if ((p->MODRM.s.MOD==0 && p->MODRM.s.RM==4) || 
                        (p->MODRM.s.MOD==1 && p->MODRM.s.RM==4) || 
                        (p->MODRM.s.MOD==2 && p->MODRM.s.RM==4))
                {
                    p->SIB.as_byte=Da_stage1_get_next_byte(p); p->len++;
                    p->SIB_loaded=TRUE;
                };

                // DISP is present?

                if (p->MODRM.s.MOD==1) // DISP8 present, read it
                {
                    p->DISP8=Da_stage1_get_next_byte(p); 
                    p->len++;
                    p->DISP8_loaded=TRUE;
                };

                if ((p->MODRM.s.MOD==0 && p->MODRM.s.RM==5) || 
                        p->MODRM.s.MOD==2 || 
                        (p->MODRM.s.MOD==0 && p->SIB_loaded==TRUE && p->SIB.s.base==5)) // DISP32 present, read it
                {
                    p->DISP32=Da_stage1_get_next_dword(p);
                    p->DISP32_pos=p->cur_adr-sizeof(uint32_t);
                    p->len+=4;
                    p->DISP32_loaded=TRUE;
                };
            } 
            else
            { // PREFIX_67 present, 16-bit ...ing mode, first version of modrm table

                if (p->MODRM.s.MOD==1) // DISP8 present, read it
                {
                    p->DISP8=Da_stage1_get_next_byte(p); 
                    p->len++;
                    p->DISP8_loaded=TRUE;
                };

                if ((p->MODRM.s.MOD==0 && p->MODRM.s.RM==6) || p->MODRM.s.MOD==2) // DISP16 present, read it
                {
                    p->DISP16=Da_stage1_get_next_word(p); 
                    p->len+=2;
                    p->DISP16_loaded=TRUE;
                };
            };
        };

        // IMM8/16/32 is present? load if so

        // порядок загрузки 16, а потом 8, должен соблюдаться чтобы ENTER верно дизасмился
        if ((p->new_flags & F_IMM16) || (PREFIX66_allowed_and_present && (p->new_flags & F_IMM32)))
        {
            p->IMM16=Da_stage1_get_next_word(p);
            p->len+=2;
            p->IMM16_loaded=TRUE;
        };

        if (p->new_flags & F_IMM8)
        {
            p->IMM8=Da_stage1_get_next_byte(p); 
            p->len++;
            p->IMM8_loaded=TRUE;
        };

        if ((p->new_flags & F_IMM32) && PREFIX66_allowed_and_present==FALSE)
        {
            assert (PREFIX66_allowed_and_present==FALSE);
            p->IMM32=Da_stage1_get_next_dword(p);
            p->IMM32_pos=p->cur_adr-sizeof(uint32_t);
            p->len+=4;
            p->IMM32_loaded=TRUE;
        };

        if ((p->new_flags & F_IMM64) && PREFIX66_allowed_and_present==FALSE)
        {
            assert (PREFIX66_allowed_and_present==FALSE);
            p->IMM64=Da_stage1_get_next_qword(p);
            //L ("IMM64=0x" PRI_REG_HEX "\n", IMM64);
            p->IMM64_pos=p->cur_adr-sizeof(uint64_t);
            p->len+=8;
            p->IMM64_loaded=TRUE;
        };

        if (p->new_flags & F_PTR)
        {
            if (p->x64)
            {
                p->PTR=Da_stage1_get_next_qword(p);
                p->PTR_pos=p->cur_adr-sizeof(uint64_t);
                p->len+=sizeof(uint64_t);
                p->PTR_loaded=TRUE;
            }
            else
            {
                p->PTR=Da_stage1_get_next_dword(p);
                p->PTR_pos=p->cur_adr-sizeof(uint32_t);
                p->len+=4;
                p->PTR_loaded=TRUE;
            };
        };

        if ((p->new_flags & F_REG32_IS_LOWEST_PART_OF_1ST_BYTE) || 
                (p->new_flags & F_REG64_IS_LOWEST_PART_OF_1ST_BYTE))
        {
            p->REG_FROM_LOWEST_PART_OF_1ST_BYTE=opc&7;
            p->REG_FROM_LOWEST_PART_OF_1ST_BYTE_loaded=TRUE;
        };

        return TRUE;
    };

    // opcode not found

    printf ("adr_of_ins=0x" PRI_SIZE_T_HEX " opcode not found, opc=%02X, p->x64=%d, prefixes=", 
            adr_of_ins, opc, p->x64);

    if (p->x64)   printf("X64 ");
    if (p->REX_W) printf("REX_W ");
    if (p->REX_R) printf("REX_R ");
    if (p->REX_X) printf("REX_X ");
    if (p->REX_B) printf("REX_B ");

    if (p->ESCAPE_0F) printf("0F ");
    if (p->ESCAPE_F2) printf("F2 ");
    if (p->ESCAPE_F3) printf("F3 ");
    if (p->PREFIX_66_is_present) printf("66 ");
    if (p->PREFIX_67) printf("67 ");

    printf ("\n");

    return FALSE;
};

static X86_register get_x64_reg (int i)
{
    switch (i)
    {
    case 0: return R_RAX;
    case 1: return R_RCX;
    case 2: return R_RDX;
    case 3: return R_RBX;
    case 4: return R_RSP;
    case 5: return R_RBP;
    case 6: return R_RSI;
    case 7: return R_RDI;
    case 8: return R_R8;
    case 9: return R_R9;
    case 10: return R_R10;
    case 11: return R_R11;
    case 12: return R_R12;
    case 13: return R_R13;
    case 14: return R_R14;
    case 15: return R_R15;
    default: 
        assert(0);
        return R_ABSENT;
    };
};

static X86_register get_XMM_reg (int i)
{
    switch (i)
    {
    case 0: return R_XMM0;
    case 1: return R_XMM1;
    case 2: return R_XMM2;
    case 3: return R_XMM3;
    case 4: return R_XMM4;
    case 5: return R_XMM5;
    case 6: return R_XMM6;
    case 7: return R_XMM7;
    case 8: return R_XMM8;
    case 9: return R_XMM9;
    case 10: return R_XMM10;
    case 11: return R_XMM11;
    case 12: return R_XMM12;
    case 13: return R_XMM13;
    case 14: return R_XMM14;
    case 15: return R_XMM15;
    default: assert(0); return R_ABSENT; // make compiler happy
    };
};

static X86_register get_x32_reg (int i)
{
    switch (i)
    {
    case 0: return R_EAX;
    case 1: return R_ECX;
    case 2: return R_EDX;
    case 3: return R_EBX;
    case 4: return R_ESP;
    case 5: return R_EBP;
    case 6: return R_ESI;
    case 7: return R_EDI;
    case 8: return R_R8D;
    case 9: return R_R9D;
    case 10: return R_R10D;
    case 11: return R_R11D;
    case 12: return R_R12D;
    case 13: return R_R13D;
    case 14: return R_R14D;
    case 15: return R_R15D;
    default: 
        assert(0); 
        return R_ABSENT; // make compiler happy
    };
};

static X86_register get_x16_reg (int i)
{
    switch (i)
    {
    case 0: return R_AX;
    case 1: return R_CX;
    case 2: return R_DX;
    case 3: return R_BX;
    case 4: return R_SP;
    case 5: return R_BP;
    case 6: return R_SI;
    case 7: return R_DI;
    case 8: return R_R8W;
    case 9: return R_R9W;
    case 10: return R_R10W;
    case 11: return R_R11W;
    case 12: return R_R12W;
    case 13: return R_R13W;
    case 14: return R_R14W;
    case 15: return R_R15W;
    default: 
        assert(0); 
        return R_ABSENT; // make compiler happy
    }
};

static X86_register get_STx_reg (int i)
{
    switch (i)
    {
    case 0: return R_ST0;
    case 1: return R_ST1;
    case 2: return R_ST2;
    case 3: return R_ST3;
    case 4: return R_ST4;
    case 5: return R_ST5;
    case 6: return R_ST6;
    case 7: return R_ST7;
    default: 
        assert(0); 
        return R_ABSENT; // make compiler happy
    }
};

static X86_register get_8bit_reg (int i, BOOL replace_xH_to_xPL_and_xIL)
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

    switch (i)
    {
    case 0: return R_AL;
    case 1: return R_CL;
    case 2: return R_DL;
    case 3: return R_BL;
    case 4: return R_AH;
    case 5: return R_CH;
    case 6: return R_DH;
    case 7: return R_BH;
    case 8: return R_R8L;
    case 9: return R_R9L;
    case 10: return R_R10L;
    case 11: return R_R11L;
    case 12: return R_R12L;
    case 13: return R_R13L;
    case 14: return R_R14L;
    case 15: return R_R15L;
    default: 
        assert(0); 
        return R_ABSENT; // make compiler happy
    };
};

static void decode_SIB (Da_stage1 *stage1,
                        X86_register * adr_base, X86_register * adr_index, unsigned * adr_index_mult, int64_t * adr_disp, uint8_t * adr_disp_width_in_bits, disas_address *adr_disp_pos)
{
    int scale_i=1;

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
    case 0: 
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7: 
        if (stage1->x64)
            *adr_base=get_x64_reg((stage1->REX_B ? 0x8 : 0) | stage1->SIB.s.base); 
        else
            *adr_base=get_x32_reg(stage1->SIB.s.base); 
        break;
    default: assert(0); break;
    };

    switch (stage1->SIB.s.scale)
    {
    case 0: scale_i=1; *adr_index_mult=1; break;
    case 1: scale_i=2; *adr_index_mult=2; break;
    case 2: scale_i=4; *adr_index_mult=4; break;
    case 3: scale_i=8; *adr_index_mult=8; break;
    default: assert(0);
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
                *adr_index=get_x64_reg(tmp);
        }
        else
            *adr_index=R_ABSENT;
        break;
    case 0: 
    case 1: 
    case 2: 
    case 3: 
    case 5: 
    case 6: 
    case 7: 
        if (stage1->x64)
            *adr_index=get_x64_reg((stage1->REX_X ? 0x8 : 0) | stage1->SIB.s.index); 
        else
            *adr_index=get_x32_reg(stage1->SIB.s.index); 
        break;
    default: assert(0); break;
    };

    if (stage1->SIB.s.base==5 && stage1->MODRM.s.MOD==0)
    {
        assert (stage1->DISP32_loaded==TRUE);
        *adr_disp_width_in_bits=32;
        *adr_disp=stage1->DISP32;
        assert (stage1->DISP32_pos!=0);
        *adr_disp_pos=stage1->DISP32_pos;
    };

#if 0
    cout << strfmt ("scale_i=%d *adr_index_mult=%d", scale_i, *adr_index_mult) << endl;
    cout << "*adr_base=" << adr_base->ToString() << endl;
    cout << "*adr_index=" << adr_index->ToString() << endl;
#endif
};

static Da_op *create_Da_op (enum op_source op, Da_stage1 *stage1, disas_address ins_adr, unsigned ins_len)
{
    Da_op* rt=DCALLOC(Da_op, 1, "Da_op");

    rt->u.adr.adr_index_mult=1; // default value

    assert (op!=OP_ABSENT);

    switch (op)
    {
    case OP_REG64_FROM_LOWEST_PART_OF_1ST_BYTE:
        rt->type=DA_OP_TYPE_REGISTER; 
        rt->value_width_in_bits=64;
        rt->u.reg=get_x64_reg ((stage1->REX_B ? 8 : 0) | stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE);
        break;

    case OP_REG32_FROM_LOWEST_PART_OF_1ST_BYTE:
        rt->type=DA_OP_TYPE_REGISTER; 

        if (stage1->PREFIX_66_is_present)
        {
            rt->value_width_in_bits=16;
            rt->u.reg=get_x16_reg (stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE);
        }
        else
        {
            rt->value_width_in_bits=32;
            if (stage1->x64)
                rt->u.reg=get_x32_reg ((stage1->REX_B ? 8 : 0) | stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE);
            else
                rt->u.reg=get_x32_reg (stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE);
        };
        break;

    case OP_REG8_FROM_LOWEST_PART_OF_1ST_BYTE:
        rt->type=DA_OP_TYPE_REGISTER; 
        rt->value_width_in_bits=8;
        if (stage1->x64)
            rt->u.reg=get_8bit_reg ((stage1->REX_B ? 8 : 0) | stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE, stage1->REX_prefix_seen);
        else
            rt->u.reg=get_8bit_reg (stage1->REG_FROM_LOWEST_PART_OF_1ST_BYTE, FALSE);
        break;

    case OP_1:
        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=32; // FIXME: тут не всегда 32 бита
        create_Value(V_DWORD, 1, &rt->u.val.v);
        break;

    case OP_AH: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_AH; break;
    case OP_AL: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_AL; break;
    case OP_BH: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_BH; break;
    case OP_BL: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_BL; break;
    case OP_CH: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_CH; break;
    case OP_CL: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_CL; break;
    case OP_DH: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_DH; break;
    case OP_DL: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8; rt->u.reg=R_DL; break;

    case OP_AX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_AX; break;
    case OP_BX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_BX; break;
    case OP_CX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_CX; break;
    case OP_DX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_DX; break;

    case OP_SP: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_SP; break;
    case OP_BP: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_BP; break;
    case OP_SI: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_SI; break;
    case OP_DI: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_DI; break;

    case OP_EAX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_EAX; break;
    case OP_EBP: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_EBP; break;
    case OP_EBX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_EBX; break;
    case OP_ECX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_ECX; break;
    case OP_EDI: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_EDI; break;
    case OP_EDX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_EDX; break;
    case OP_ESI: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_ESI; break;
    case OP_ESP: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=32; rt->u.reg=R_ESP; break;

    case OP_RAX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RAX; break;
    case OP_RBP: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RBP; break;
    case OP_RBX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RBX; break;
    case OP_RCX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RCX; break;
    case OP_RDI: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RDI; break;
    case OP_RDX: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RDX; break;
    case OP_RSI: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RSI; break;
    case OP_RSP: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64; rt->u.reg=R_RSP; break;

    case OP_ST0: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST0; break;
    case OP_ST1: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST1; break;
    case OP_ST2: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST2; break;
    case OP_ST3: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST3; break;
    case OP_ST4: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST4; break;
    case OP_ST5: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST5; break;
    case OP_ST6: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST6; break;
    case OP_ST7: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80; rt->u.reg=R_ST7; break;

    case OP_ES: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_ES; break;
    case OP_CS: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_CS; break;
    case OP_SS: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_SS; break;
    case OP_DS: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_DS; break;
    case OP_FS: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_FS; break;
    case OP_GS: rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; rt->u.reg=R_GS; break;

    case OP_IMM8:
        assert (stage1->IMM8_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=8;
        create_Value (V_BYTE, stage1->IMM8, &rt->u.val.v);

        break;

    case OP_IMM8_SIGN_EXTENDED_TO_IMM32:
        assert (stage1->IMM8_loaded==TRUE); // dirty hack

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=32;
        
        create_Value (V_DWORD, (int32_t)(int8_t)stage1->IMM8, &rt->u.val.v);
        break;

    case OP_IMM8_SIGN_EXTENDED_TO_IMM64:
        assert (stage1->IMM8_loaded==TRUE); // dirty hack

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=64;
        create_Value (V_QWORD, (int64_t)(int8_t)stage1->IMM8, &rt->u.val.v);
        break;

    case OP_IMM16_SIGN_EXTENDED_TO_IMM32:
        assert (stage1->IMM16_loaded==TRUE); // dirty hack

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=32;
        create_Value (V_DWORD, (int32_t)(int16_t)stage1->IMM16, &rt->u.val.v);
        break;

    case OP_IMM16_SIGN_EXTENDED_TO_IMM64:
        assert (stage1->IMM16_loaded==TRUE); // dirty hack

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=64;
        create_Value(V_QWORD, (int64_t)(int16_t)stage1->IMM16, &rt->u.val.v);
        break;

    case OP_IMM32_SIGN_EXTENDED_TO_IMM64:
        assert (stage1->IMM32_loaded==TRUE); // dirty hack

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=64;

        //L ("stage1.IMM32 = %08X, %d\n", stage1.IMM32, (int32_t)stage1.IMM32);

        if ((int32_t)stage1->IMM32>=0)
        {
            //L ("p1\n");
            create_Value (V_QWORD, (uint64_t)stage1->IMM32, &rt->u.val.v);
        }
        else
        {
            //L ("p2\n");
            create_Value (V_QWORD, (uint64_t)(stage1->IMM32 | 0xFFFFFFFF00000000), &rt->u.val.v);
        }
        break;

    case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_BYTE:
    case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_DWORD:

        assert (stage1->IMM64_loaded==TRUE);
        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->u.adr.adr_disp_width_in_bits=64;
        rt->u.adr.adr_disp=stage1->IMM64;
        //L ("stage1.IMM64=0x" PRI_REG_HEX "\n", stage1.IMM64);
        rt->u.adr.adr_disp_is_absolute=TRUE;
        assert (stage1->IMM64_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->IMM64_pos;
        switch (op)
        {
        case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_BYTE: rt->value_width_in_bits=8; break;
        case OP_IMM64_AS_ABSOLUTE_ADDRESS_PTR_TO_DWORD: rt->value_width_in_bits=32; break;
        default: assert(0);
        };
        break;

    case OP_MOFFS32:
    case OP_MOFFS16:
    case OP_MOFFS8:

        assert (stage1->PTR_loaded);
        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->u.adr.adr_disp=stage1->PTR;
        rt->u.adr.adr_disp_is_absolute=TRUE;
        assert (stage1->PTR_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->PTR_pos;

        if (stage1->x64)
            rt->u.adr.adr_disp_width_in_bits=64;
        else
            rt->u.adr.adr_disp_width_in_bits=32;

        switch (op)
        {
        case OP_MOFFS32: rt->value_width_in_bits=32; break;
        case OP_MOFFS16: rt->value_width_in_bits=16; break;
        case OP_MOFFS8:  rt->value_width_in_bits=8; break;
        default: assert(0);
        };
        break;

    case OP_IMM8_SIGN_EXTENDED_TO_IMM16:
        assert (stage1->IMM8_loaded==TRUE); // dirty hack

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=16;
        create_Value (V_WORD, (int16_t)(int8_t)stage1->IMM8, &rt->u.val.v);
        break;


    case OP_IMM8_AS_REL32:
        assert (stage1->IMM8_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=32;
        create_Value (V_DWORD, (int32_t)(ins_adr + ins_len)+(int8_t)stage1->IMM8, &rt->u.val.v);
        break;

    case OP_IMM8_AS_REL64:
        assert (stage1->IMM8_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=64;
        create_Value (V_QWORD, (int64_t)(ins_adr + ins_len)+(int8_t)stage1->IMM8, &rt->u.val.v);
        break;

    case OP_IMM16:
        assert (stage1->IMM16_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=16;
        create_Value(V_WORD, stage1->IMM16, &rt->u.val.v);
        break;

    case OP_IMM32:
        assert (stage1->IMM32_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=32;
        create_Value (V_DWORD, stage1->IMM32, &rt->u.val.v);
        assert (stage1->IMM32_pos!=0);
        rt->u.val.value32_pos=stage1->IMM32_pos;

        break;

    case OP_IMM64:
        assert (stage1->IMM64_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=64;
        create_Value (V_QWORD, stage1->IMM64, &rt->u.val.v);
        assert (stage1->IMM64_pos!=0);
        rt->u.val.value64_pos=stage1->IMM64_pos;

        break;

    case OP_IMM32_AS_OFS32:
        assert (stage1->IMM32_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->value_width_in_bits=32;
        rt->u.adr.adr_disp_width_in_bits=32;
        rt->u.adr.adr_disp=stage1->IMM32;
        assert (stage1->IMM32_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->IMM32_pos;

        break;

    case OP_IMM64_AS_OFS32:
        assert (stage1->IMM64_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->value_width_in_bits=32;
        rt->u.adr.adr_disp_width_in_bits=64;
        rt->u.adr.adr_disp=stage1->IMM64;
        assert (stage1->IMM64_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->IMM64_pos;

        break;

    case OP_IMM32_AS_OFS16:
        if (stage1->IMM32_loaded==FALSE)
        {
            L ("stage1->IMM32_loaded==FALSE\n");
            L ("instruction:\n");
            Da_stage1_dump(stage1, ins_adr, ins_len);
            L ("ins_adr=0x" PRI_SIZE_T_HEX "\n", ins_adr);
            //if (p!=NULL) 
            //    cout << "sym=" << p->symbols->get_sym (ins_adr) << endl;
            L("exiting\n");
            exit(0);
        };

        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->value_width_in_bits=16;
        rt->u.adr.adr_disp_width_in_bits=32;
        rt->u.adr.adr_disp=stage1->IMM32;
        assert (stage1->IMM32_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->IMM32_pos;

        break;

    case OP_IMM32_AS_OFS8:
        assert (stage1->IMM32_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->value_width_in_bits=8;
        rt->u.adr.adr_disp=stage1->IMM32;
        rt->u.adr.adr_disp_width_in_bits=32;
        assert (stage1->IMM32_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->IMM32_pos;

        break;
 
    case OP_IMM64_AS_OFS8:
        assert (stage1->IMM64_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
        rt->value_width_in_bits=8;
        rt->u.adr.adr_disp=stage1->IMM64;
        rt->u.adr.adr_disp_width_in_bits=64;
        assert (stage1->IMM64_pos!=0);
        rt->u.adr.adr_disp_pos=stage1->IMM64_pos;

        break;

    case OP_IMM32_AS_REL32:
        assert (stage1->IMM32_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=32;

        create_Value (V_DWORD, (int32_t)(ins_adr+ins_len)+(int32_t)stage1->IMM32, &rt->u.val.v);
        assert (stage1->IMM32_pos!=0);
        rt->u.val.value32_pos=stage1->IMM32_pos;

        break;

    case OP_IMM32_SIGN_EXTENDED_TO_REL64:
        assert (stage1->IMM32_loaded==TRUE);

        rt->type=DA_OP_TYPE_VALUE;
        rt->value_width_in_bits=64;

        create_Value (V_QWORD, (int64_t)(ins_adr+ins_len)+(int64_t)((int32_t)stage1->IMM32), &rt->u.val.v);
        assert (stage1->IMM32_pos!=0);
        rt->u.val.value32_pos=stage1->IMM32_pos;

        break;

    case OP_MODRM_R32:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; 
        rt->value_width_in_bits=32;
        rt->u.reg=get_x32_reg ((stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG);
        break;

    case OP_MODRM_R64:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; 
        rt->value_width_in_bits=64;
        rt->u.reg=get_x64_reg ((stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG);
        break;

    case OP_MODRM_R16:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; 
        if (stage1->REX_prefix_seen)
            rt->u.reg=get_x16_reg ((stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG);
        else
            rt->u.reg=get_x16_reg (stage1->MODRM.s.REG);
        break;

    case OP_MODRM_SREG:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16; 
        switch (stage1->MODRM.s.REG)
        {
        case 0: rt->u.reg=R_ES; break;
        case 1: rt->u.reg=R_CS; break;
        case 2: rt->u.reg=R_SS; break;
        case 3: rt->u.reg=R_DS; break;
        case 4: rt->u.reg=R_FS; break;
        case 5: rt->u.reg=R_GS; break;
        case 6: assert(0); break; // reserved
        case 7: assert(0); break; // reserved
        }
        break;

    case OP_MODRM_R8:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8;
        if (stage1->x64)
            rt->u.reg=get_8bit_reg ((stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG, stage1->REX_prefix_seen);
        else
            rt->u.reg=get_8bit_reg (stage1->MODRM.s.REG, FALSE);
        break;

    case OP_MODRM_R_XMM:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; 
        rt->value_width_in_bits=128;
        if (stage1->REX_prefix_seen)
            rt->u.reg=get_XMM_reg ((stage1->REX_R ? 8 : 0) | stage1->MODRM.s.REG);
        else
            rt->u.reg=get_XMM_reg (stage1->MODRM.s.REG);
        break;

    case OP_MODRM_R_MM:
        assert (stage1->MODRM_loaded==TRUE);
        rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64;
        switch (stage1->MODRM.s.REG)
        {
        case 0: rt->u.reg=R_MM0; break;
        case 1: rt->u.reg=R_MM1; break;
        case 2: rt->u.reg=R_MM2; break;
        case 3: rt->u.reg=R_MM3; break;
        case 4: rt->u.reg=R_MM4; break;
        case 5: rt->u.reg=R_MM5; break;
        case 6: rt->u.reg=R_MM6; break;
        case 7: rt->u.reg=R_MM7; break;
        };
        break;

    case OP_MODRM_RM64:
    case OP_MODRM_RM32:
    case OP_MODRM_RM16:
    case OP_MODRM_RM8:
    case OP_MODRM_RM_XMM:
    case OP_MODRM_RM_MM:
    case OP_MODRM_RM_M64FP:
        assert (stage1->MODRM_loaded==TRUE);
        switch (stage1->MODRM.s.MOD)
        {
        case 0: // mod=0

            rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;

            switch (op)
            {
            case OP_MODRM_RM64:   rt->value_width_in_bits=64; break;
            case OP_MODRM_RM32:   rt->value_width_in_bits=32; break;
            case OP_MODRM_RM16:   rt->value_width_in_bits=16; break;
            case OP_MODRM_RM8:    rt->value_width_in_bits=8; break;
            case OP_MODRM_RM_MM:
            case OP_MODRM_RM_M64FP:
                rt->value_width_in_bits=64; break; // 64 bit
            case OP_MODRM_RM_XMM: rt->value_width_in_bits=128; break; // 128 bit

            default: assert(0);
            };

            if (stage1->PREFIX_67==FALSE)
            { // PREFIX_67==FALSE, take 32-bit part of modrm table

                //cout << "stage1->MODRM_RM=" << (int)stage1->MODRM_RM << endl;

                switch (stage1->MODRM.s.RM)
                {
                case 4: // SIB often without disp32, but sometimes with disp32
                    assert (stage1->SIB_loaded==TRUE);
                    {
                        decode_SIB (stage1,
                            &rt->u.adr.adr_base,
                            &rt->u.adr.adr_index,
                            &rt->u.adr.adr_index_mult,
                            &rt->u.adr.adr_disp,
                            &rt->u.adr.adr_disp_width_in_bits,
                            &rt->u.adr.adr_disp_pos);
                        rt->u.adr.adr_disp_is_not_negative=TRUE;
                    };

                    break;
                case 5:  
                    assert (stage1->DISP32_loaded==TRUE);
                    //cout << hex << "stage1->DISP32=" << stage1->DISP32 << endl;
                    if (stage1->x64)
                    {
                        rt->u.adr.adr_disp_width_in_bits=64;
                        rt->u.adr.adr_disp=ins_adr + stage1->DISP32 + stage1->len;
                        if (rt->u.adr.adr_disp&0x80000000)
                            rt->u.adr.adr_disp|=0xFFFFFFFF00000000;
                    }
                    else
                    {
                        rt->u.adr.adr_disp_width_in_bits=32;
                        rt->u.adr.adr_disp=stage1->DISP32;
                    };
                    assert (stage1->DISP32_pos!=0);
                    rt->u.adr.adr_disp_pos=stage1->DISP32_pos;
                    rt->u.adr.adr_disp_is_not_negative=TRUE;
                    break; // EA is just disp32
                
                case 0:
                case 1:
                case 2:
                case 3:
                case 6: 
                case 7: 
                    //if (op==OP_MODRM_RM64)
                    if (stage1->x64)
                        rt->u.adr.adr_base=get_x64_reg((stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM);
                    else
                        rt->u.adr.adr_base=get_x32_reg(stage1->MODRM.s.RM);
                    break;
                default: assert(0); break;
                };
            } else
            { // PREFIX_67==TRUE, take 16-bit part of modrm table
                switch (stage1->MODRM.s.RM)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:  
                case 7:  
                    assert (!"PREFIX_67=TRUE, we don't process 16-bit part of modrm table (yet)");
                case 6: // take disp16
                    assert (stage1->DISP16_loaded==TRUE);
                    // на практике это только в случае FS:[..] вроде...

                    rt->type=DA_OP_TYPE_VALUE_IN_MEMORY; rt->value_width_in_bits=32;
                    break;
                default: assert(0);
                };
            };
            break;

        case 1: // mod=1. [reg+disp8]

            rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;

            switch (op)
            {
            case OP_MODRM_RM64:   rt->value_width_in_bits=64; break;
            case OP_MODRM_RM32:   rt->value_width_in_bits=32; break;
            case OP_MODRM_RM16:   rt->value_width_in_bits=16; break;
            case OP_MODRM_RM8:    rt->value_width_in_bits=8; break;
            case OP_MODRM_RM_MM:
            case OP_MODRM_RM_M64FP:
                rt->value_width_in_bits=64; break; // 64 bit
            case OP_MODRM_RM_XMM: rt->value_width_in_bits=128; break; // 128 bit
            default: assert(0);
            };

            assert (stage1->PREFIX_67==FALSE); // yet...
            switch (stage1->MODRM.s.RM)
            {
            case 4:  // SIB byte present, SIB+disp8; 
                assert (stage1->SIB_loaded==TRUE);
                assert (stage1->DISP8_loaded==TRUE);
                {
                    decode_SIB (stage1,
                        &rt->u.adr.adr_base,
                        &rt->u.adr.adr_index,
                        &rt->u.adr.adr_index_mult,
                        &rt->u.adr.adr_disp,
                        &rt->u.adr.adr_disp_width_in_bits,
                        &rt->u.adr.adr_disp_pos);

                    rt->u.adr.adr_disp_width_in_bits=32;
                    rt->u.adr.adr_disp=(uint32_t)(int32_t)(int8_t)stage1->DISP8;

                    rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    switch (op)
                    {
                    case OP_MODRM_RM8:    rt->value_width_in_bits=8; break;
                    case OP_MODRM_RM16:   rt->value_width_in_bits=16; break;
                    case OP_MODRM_RM32:   rt->value_width_in_bits=32; break;
                    case OP_MODRM_RM64:   rt->value_width_in_bits=64; break;
                    case OP_MODRM_RM_MM:
                    case OP_MODRM_RM_M64FP:
                        rt->value_width_in_bits=64; break;
                    case OP_MODRM_RM_XMM: rt->value_width_in_bits=128; break;
                    default: 
                        assert(0);
                        break;
                    };
                };
                break;

            case 0:  
            case 1:  
            case 2:  
            case 3:  
            case 5:  
            case 6:  
            case 7:  
                assert (stage1->DISP8_loaded==TRUE);
                //if (op==OP_MODRM_RM64)
                if (stage1->x64)
                    rt->u.adr.adr_base=get_x64_reg((stage1->REX_B ? 8 : 0) | stage1->MODRM.s.RM);
                else
                    rt->u.adr.adr_base=get_x32_reg(stage1->MODRM.s.RM);

                rt->u.adr.adr_disp_width_in_bits=32;
                rt->u.adr.adr_disp=(uint32_t)(int32_t)(int8_t)stage1->DISP8;

                break;

            default: assert(0);
            };
            break;

        case 2:  // mod=2. [reg+disp32]

            rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;

            switch (op)
            {
            case OP_MODRM_RM64:   rt->value_width_in_bits=64; break;
            case OP_MODRM_RM32:   rt->value_width_in_bits=32; break;
            case OP_MODRM_RM16:   rt->value_width_in_bits=16; break;
            case OP_MODRM_RM8:    rt->value_width_in_bits=8; break;
            case OP_MODRM_RM_MM:
            case OP_MODRM_RM_M64FP:
                rt->value_width_in_bits=64; break; // 64 bit
            case OP_MODRM_RM_XMM: rt->value_width_in_bits=128; break; // 128 bit
            default: assert(0);
            };

            assert (stage1->PREFIX_67==FALSE); // yet...
            switch (stage1->MODRM.s.RM)
            {
            case 4:  // SIB byte present, SIB+disp32; 
                assert (stage1->SIB_loaded==TRUE);
                assert (stage1->DISP32_loaded==TRUE);
                {
                    decode_SIB (stage1,
                        &rt->u.adr.adr_base,
                        &rt->u.adr.adr_index,
                        &rt->u.adr.adr_index_mult,
                        &rt->u.adr.adr_disp,
                        &rt->u.adr.adr_disp_width_in_bits,
                        &rt->u.adr.adr_disp_pos);

                    rt->u.adr.adr_disp_width_in_bits=32;
                    rt->u.adr.adr_disp=stage1->DISP32; // bug was there
                    rt->u.adr.adr_disp_pos=stage1->DISP32_pos; // bug was there

                    rt->type=DA_OP_TYPE_VALUE_IN_MEMORY;
                    switch (op)
                    {
                    case OP_MODRM_RM8:    rt->value_width_in_bits=8; break;
                    case OP_MODRM_RM16:   rt->value_width_in_bits=16; break;
                    case OP_MODRM_RM32:   rt->value_width_in_bits=32; break;
                    case OP_MODRM_RM64:   rt->value_width_in_bits=64; break;
                    case OP_MODRM_RM_MM:  
                    case OP_MODRM_RM_M64FP:
                        rt->value_width_in_bits=64; break;
                    case OP_MODRM_RM_XMM: rt->value_width_in_bits=128; break;
                    default: assert(0);
                    };
                };

                break;

            case 0:  
            case 1:  
            case 2:  
            case 3:  
            case 5:  
            case 6:  
            case 7:  
                assert (stage1->DISP32_loaded==TRUE);
                if (stage1->x64)
                    rt->u.adr.adr_base=get_x64_reg((stage1->REX_B ? 8 : 0) | stage1->MODRM.s.RM);
                else
                    rt->u.adr.adr_base=get_x32_reg(stage1->MODRM.s.RM);
                rt->u.adr.adr_disp_width_in_bits=32;
                rt->u.adr.adr_disp=stage1->DISP32;
                assert (stage1->DISP32_pos!=0);
                rt->u.adr.adr_disp_pos=stage1->DISP32_pos;
                break;

            default: assert(0);
            };
            break;

        case 3:  // mod == 3, treat RM as register

            switch (op)
            {
            case OP_MODRM_RM64:
                rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64;
                rt->u.reg=get_x64_reg((stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM);
                break;

            case OP_MODRM_RM32:

                rt->type=DA_OP_TYPE_REGISTER;

                if (IS_SET(stage1->new_flags, F_WHEN_MOD3_TREAT_RM_AS_STx))
                {
                    rt->value_width_in_bits=80;
                    rt->u.reg=get_STx_reg (stage1->MODRM.s.RM);
                }
                else
                {
                    rt->value_width_in_bits=32;
                    //if (stage1->REX_prefix_seen)
                    if (stage1->x64)
                        rt->u.reg=get_x32_reg((stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM);
                    //reg=get_x64_reg((stage1->REX_B ? 0x8 : 0) | stage1->MODRM_RM);
                    else
                        rt->u.reg=get_x32_reg(stage1->MODRM.s.RM);
                };
                break;

            case OP_MODRM_RM16:

                rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=16;

                if (stage1->x64)
                    rt->u.reg=get_x16_reg ((stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM);
                else
                    rt->u.reg=get_x16_reg (stage1->MODRM.s.RM);
                
                break;

            case OP_MODRM_RM8:

                rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=8;

                if (stage1->x64)
                    rt->u.reg=get_8bit_reg ((stage1->REX_B ? 0x8 : 0) | stage1->MODRM.s.RM, stage1->REX_prefix_seen);
                else
                    rt->u.reg=get_8bit_reg (stage1->MODRM.s.RM, FALSE);
                break;
            case OP_MODRM_RM_XMM:

                rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=128;

                if (stage1->REX_prefix_seen)
                    rt->u.reg=get_XMM_reg ((stage1->REX_B ? 8 : 0) | stage1->MODRM.s.RM);
                else
                    rt->u.reg=get_XMM_reg (stage1->MODRM.s.RM);
                break;

            case OP_MODRM_RM_MM:

                rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=64;

                switch (stage1->MODRM.s.RM)
                {
                case 0: rt->u.reg=R_MM0; break;
                case 1: rt->u.reg=R_MM1; break;
                case 2: rt->u.reg=R_MM2; break;
                case 3: rt->u.reg=R_MM3; break;
                case 4: rt->u.reg=R_MM4; break;
                case 5: rt->u.reg=R_MM5; break;
                case 6: rt->u.reg=R_MM6; break;
                case 7: rt->u.reg=R_MM7; break;
                default: assert(0);
                };
                break;

            case OP_MODRM_RM_M64FP:

                rt->type=DA_OP_TYPE_REGISTER; rt->value_width_in_bits=80;

                rt->u.reg=get_STx_reg (stage1->MODRM.s.RM);
                break;

            default: assert(0);
            };
        };

        break;

    default:
        L ("unknown op=%d\n", op);
        assert(0);
    };
    return rt;
};

static enum op_source shrink_op_32_to_16 (enum op_source op)
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

static enum op_source sign_extend_op_32_to_64 (enum op_source op)
{
    switch (op)
    {
    case OP_IMM8_SIGN_EXTENDED_TO_IMM32: return OP_IMM8_SIGN_EXTENDED_TO_IMM64;
    case OP_IMM32_AS_REL32: return OP_IMM32_SIGN_EXTENDED_TO_REL64;
    case OP_IMM32: return OP_IMM32_SIGN_EXTENDED_TO_IMM64;
    default: return op;
    };
};

static enum op_source promote_op_32_to_64 (enum op_source op)
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

Da* Da_stage1_into_result (Da_stage1 *stage1, disas_address adr_of_ins)
{
    Da* rt;
    Ins_definition *i;
    uint64_t fl;
    op_source new_op1, new_op2, new_op3;

    rt=DCALLOC(Da, 1, "Da");
    rt->ins_code=stage1->ins_code;
    rt->len=stage1->len;
    rt->prefix_codes=stage1->PREFIXES;

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

    if (i->op1!=OP_ABSENT) rt->_op[0]=create_Da_op (new_op1, stage1, adr_of_ins, rt->len);
    if (i->op2!=OP_ABSENT) rt->_op[1]=create_Da_op (new_op2, stage1, adr_of_ins, rt->len);
    if (i->op3!=OP_ABSENT) rt->_op[2]=create_Da_op (new_op3, stage1, adr_of_ins, rt->len);

    if (rt->ins_code==I_XCHG && (Da_op_equals (rt->_op[0], rt->_op[1])))
    {
        rt->ins_code=I_NOP;
        DFREE(rt->_op[0]);
        DFREE(rt->_op[1]);
        rt->_op[0]=NULL;
        rt->_op[1]=NULL;
    };

#if 0
    // self-test...
    //if ((ins_code==I_MOV || ins_code==I_LEA || ins_code==I_ADD) && i->op1!=OP_ABSENT && i->op2!=OP_ABSENT)
   //if ((i->op1!=OP_ABSENT) && (i->op2!=OP_ABSENT))
   if (ins_code!=I_ENTER &&
       ins_code!=I_RCL && ins_code!=I_RCR && ins_code!=I_ROL && ins_code!=I_ROR &&
       ins_code!=I_FCOM && ins_code!=I_FICOM && ins_code!=I_FCOMP && ins_code!=I_FIADD && 
       ins_code!=I_FDIVR && ins_code!=I_FDIV && 
       ins_code!=I_FSUBR && ins_code!=I_FSUB && 
       ins_code!=I_FMUL &&
       ins_code!=I_IN && ins_code!=I_OUT &&
       ins_code!=I_MOVZX && ins_code!=I_MOVSX &&
       ins_code!=I_MOVSXD &&
       ins_code!=I_SHL && ins_code!=I_SHR && ins_code!=I_SAR &&
       ins_code!=I_BT && ins_code!=I_BTC && ins_code!=I_BTS && ins_code!=I_BTR &&
       ins_code!=I_MOVQ &&
       op[0].use_count()>0 && op[1].use_count()>0)
    {
        if (op[0].get()->value_width_in_bits != op[1].get()->value_width_in_bits)
        {
            cout << "adr_of_ins=" << hex << adr_of_ins << endl;
            cout << ToString() << endl;
            cout << "op[0].get()->value_width_in_bits=" << dec << op[0].get()->value_width_in_bits << endl;
            cout << "op[1].get()->value_width_in_bits=" << dec << op[1].get()->value_width_in_bits << endl;
            //assert(0);
        };
    };
#endif

    return rt;
};

Da* Da_Da (TrueFalseUndefined x64_code, uint8_t* ptr_to_ins, disas_address adr_of_ins)
{
    Da_stage1 stage1;
    
    bzero (&stage1, sizeof(Da_stage1));

    stage1.use_callbacks=FALSE;
    stage1.cur_ptr=ptr_to_ins;
    
    if (Da_stage1_Da_stage1(&stage1, x64_code, adr_of_ins)==FALSE)
        return NULL;
    return Da_stage1_into_result (&stage1, adr_of_ins);
};

Da* Da_Da_callbacks (TrueFalseUndefined x64_code, disas_address adr_of_ins, 
        callback_read_byte rb, callback_read_word rw, callback_read_dword rd, callback_read_oword ro, 
        void *param)
{
    Da_stage1 stage1;
    
    bzero (&stage1, sizeof(Da_stage1));

    stage1.use_callbacks=TRUE;
    stage1.read_byte_fn=rb;
    stage1.read_word_fn=rw;
    stage1.read_dword_fn=rd;
    stage1.read_oword_fn=ro;
    stage1.callback_param=param;

    if (Da_stage1_Da_stage1(&stage1, x64_code, adr_of_ins)==FALSE)
        return NULL;
    return Da_stage1_into_result (&stage1, adr_of_ins);
};

BOOL Da_op_is_reg(Da_op *op, X86_register reg)
{
    return op->type==DA_OP_TYPE_REGISTER && op->u.reg==reg;
};

BOOL Da_is_MOV_EBP_ESP(Da *d)
{
    return (d->ins_code==I_MOV) && Da_op_is_reg(d->_op[0], R_EBP) && Da_op_is_reg(d->_op[1], R_ESP);
};

BOOL Da_is_PUSH_EBP(Da *d)
{
    return (d->ins_code==I_PUSH) && Da_op_is_reg(d->_op[0], R_EBP);
};

//#include "x86_disas.h"

//#include "utils.hpp"

//using namespace std;

// FIXME: переписать
BOOL Da_op_is_adr_disp_negative(Da_op *op)
{
    if (op->u.adr.adr_disp_width_in_bits==32)
        return (op->u.adr.adr_disp & 0x80000000)!=0;
    else if (op->u.adr.adr_disp_width_in_bits==64)
        return (op->u.adr.adr_disp & 0x8000000000000000)!=0;
    else
    {
        //assert(0);
        return FALSE; // FIXME! чо за хрень
    };
};

void Da_op_ToString (Da_op* op, strbuf* out)
{
    BOOL something_added=FALSE;

    assert (op->type!=DA_OP_TYPE_ABSENT);

    switch (op->type)
    {
        case DA_OP_TYPE_REGISTER:
            strbuf_addstr(out, X86_register_ToString (op->u.reg));
            break;

        case DA_OP_TYPE_VALUE:

            Value_to_hex_str (&op->u.val.v, out, TRUE); // asm notation here
            break;

        case DA_OP_TYPE_VALUE_IN_MEMORY:
            strbuf_addc(out, '[');

            if (op->u.adr.adr_base != R_ABSENT)
            {
                strbuf_addstr (out, X86_register_ToString (op->u.adr.adr_base));
                something_added=TRUE;
            };

            if (op->u.adr.adr_index!=R_ABSENT)
            {
                if (op->u.adr.adr_index_mult==1)
                {
                    if (something_added)
                        strbuf_addc(out, '+');
                    strbuf_addstr(out, X86_register_ToString (op->u.adr.adr_index));
                    something_added=TRUE;
                }
                else
                {
                    if (something_added)
                        strbuf_addc(out, '+');
                    strbuf_addstr (out, X86_register_ToString (op->u.adr.adr_index));
                    strbuf_addf (out, "*%d", op->u.adr.adr_index_mult);
                    something_added=TRUE;
                };
            };

            // FIXME: забыл, что тут надо было фиксить
            if (op->u.adr.adr_disp_is_not_negative==FALSE && Da_op_is_adr_disp_negative(op) && op->u.adr.adr_disp_is_absolute==FALSE)
            {
                if (something_added)
                    strbuf_addc (out, '-');

                if (op->u.adr.adr_disp_width_in_bits==32)
                    strbuf_asmhex(out, (~((uint32_t)op->u.adr.adr_disp))+1);
                else
                    strbuf_asmhex(out, (~op->u.adr.adr_disp)+1);
                //if (op.adr_disp_pos!=0)
                //	r+=strfmt ("(adr_disp_pos=0x%x)",op.adr_disp_pos);

            }
            else
            {
                if (op->u.adr.adr_disp!=0 || something_added==FALSE)
                {
                    if (something_added)
                        strbuf_addc (out, '+');
                
                    if (op->u.adr.adr_disp_width_in_bits==32)
                        strbuf_asmhex(out, (uint32_t)op->u.adr.adr_disp);
                    else
                        strbuf_asmhex(out, op->u.adr.adr_disp);
                    //if (op.adr_disp_pos!=0)
                    //	r+=strfmt ("(adr_disp_pos=0x%x)",op.adr_disp_pos);
                };
            };
            strbuf_addc(out, ']');
            break;

        default:
            L ("%s, type=%d\n", __FUNCTION__, op->type);
            assert(0);
    };
};

void Da_op_DumpString (fds *s, Da_op* op)
{
    strbuf t;
    strbuf_init (&t, 10);
    Da_op_ToString(op, &t);
    L_fds (s, t.buf);
    strbuf_deinit(&t);
};

BOOL Da_ins_is_Jcc (Da* d)
{
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
        return TRUE;
    default:
        return FALSE;
    };
};

const char* disas1_ins_code_to_string (Ins_codes ins_code)
{
    int i;

    if (ins_code==I_NOP)
        return "NOP";

    for (i=0;;i++)
    {
        assert (ins_tbl[i].ins_code!=I_INVALID); // ins_code not found in table
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

    for (i=0; i<3; i++)
        if (d->_op[i])
        {
            if (i!=0)
                strbuf_addc(out, ',');
            strbuf_addc(out, ' ');
            Da_op_ToString(d->_op[i], out);
        };
};

void Da_DumpString(fds* s, Da *d)
{
    strbuf t;
    strbuf_init(&t, 10);
    Da_ToString(d, &t);
    puts (t.buf);
    strbuf_deinit(&t);
};

BOOL Da_ins_is_FPU (Da *d)
{
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
        return TRUE;

    default:
        return FALSE;
    };
};

BOOL Da_op_equals(Da_op *op1, Da_op *op2)
{
    if (op1->type != op2->type) return FALSE;
    if (op1->value_width_in_bits != op2->value_width_in_bits) return FALSE;

    switch (op1->type)
    {
    case DA_OP_TYPE_ABSENT: 
        return TRUE;

    case DA_OP_TYPE_REGISTER: 
        return op1->u.reg == op2->u.reg;

    case DA_OP_TYPE_VALUE:
        return compare_Values (&op1->u.val.v, &op2->u.val.v) && (op1->u.val.value32_pos == op2->u.val.value32_pos);

    case DA_OP_TYPE_VALUE_IN_MEMORY:
        if (op1->u.adr.adr_base != op2->u.adr.adr_base) return FALSE;
        if (op1->u.adr.adr_index != op2->u.adr.adr_index) return FALSE;
        if (op1->u.adr.adr_index_mult != op2->u.adr.adr_index_mult) return FALSE;
        if (op1->u.adr.adr_disp_width_in_bits != op2->u.adr.adr_disp_width_in_bits) return FALSE;
        if (op1->u.adr.adr_disp != op2->u.adr.adr_disp) return FALSE;
        if (op1->u.adr.adr_disp_pos != op2->u.adr.adr_disp_pos) return FALSE;
        return TRUE; // а в остальном - здоров как бык!

    default:
        assert(0);
        return FALSE; // make compiler happy
    };
};

const char* Da_ins_code_ToString(Da *d)
{ 
    return disas1_ins_code_to_string (d->ins_code); 
};

int Da_operands_total(Da* d)
{
    if (d->_op[0]==NULL)
        return 0;
    if (d->_op[1]==NULL)
        return 1;
    if (d->_op[2]==NULL)
        return 2;
    return 3;
};

#if 0
BOOL Da_op::is_EBP_plus_minus_X (address_offset & x) const // x can be negative/positive here
{
    if (type==DA_OP_TYPE_VALUE_IN_MEMORY &&
        adr_base==R_EBP &&
        adr_index!=R_ABSENT &&
        adr_index_mult==1)
    {
        x=(address_offset)adr_disp;
        return TRUE;
    }
    else
        return FALSE;
}
#endif

// FIXME: REG should be here? or Value?
BOOL Da_is_ADD_ESP_X (Da* d, uint32_t * out_X)
{
    if (d->ins_code!=I_ADD) return FALSE;
    assert (d->_op[0]->type==DA_OP_TYPE_REGISTER);
    if (d->_op[0]->u.reg != R_ESP) return FALSE;
    if (d->_op[1]->type != DA_OP_TYPE_VALUE) return FALSE;
    *out_X = get_as_32(&d->_op[1]->u.val.v);
    return TRUE;
};

// FIXME: REG should be here? or Value?
BOOL Da_is_SUB_ESP_X (Da* d, uint32_t * out_X)
{
    if (d->ins_code!=I_SUB) return FALSE;
    assert (d->_op[0]->type==DA_OP_TYPE_REGISTER);
    if (d->_op[0]->u.reg != R_ESP) return FALSE;
    if (d->_op[1]->type != DA_OP_TYPE_VALUE) return FALSE;
    *out_X = get_as_32(&d->_op[1]->u.val.v);
    return TRUE;
};

BOOL Da_is_RET (Da* d, uint16_t * out_X)
{
    if (d->ins_code!=I_RETN) return FALSE;
    if (d->_op[0] && d->_op[0]->type==DA_OP_TYPE_VALUE)
        *out_X = get_as_16(&d->_op[0]->u.val.v);
    else
        *out_X = 0;
    return TRUE;
};

void Da_op_free(Da_op* op)
{
   if (op->type==DA_OP_TYPE_VALUE)
   {
       Value_free(&op->u.val.v);
   };
   DFREE(op);
};

void Da_free (Da* d)
{
    if (d->_op[0]) Da_op_free (d->_op[0]);
    if (d->_op[1]) Da_op_free (d->_op[1]);
    if (d->_op[2]) Da_op_free (d->_op[2]);

    DFREE (d);
};
