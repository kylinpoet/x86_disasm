#include "x86_disas_internals.h"
#include "x86_disas.h"

// FIXME: precompiled index should be here somehow!

Ins_definition ins_tbl[]=
{
    // вынесено в отдельный файл чтобы легче было отыскивать в таблице строку по номеру..
    // а нафига это надо? забыл.
    
    // WARNING: entries should be sorted by first byte
#include "x86_tbl_entries.h"
    
    { 0, 0, 0, OP_ABSENT, OP_ABSENT, OP_ABSENT, NULL, I_INVALID }

};
