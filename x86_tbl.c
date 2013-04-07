#include "x86_disas_internals.h"
#include "x86_disas.h"

Ins_definition ins_tbl[]=
{
    // вынесено в отдельный файл чтобы легче было отыскивать в таблице строку по номеру..
    // а нафига это надо? забыл.
    
    // * WARNING: entries should be sorted by first byte
    // * ins with F_REG32_IS_LOWEST_PART_OF_1ST_BYTE should be first before others, with the same opcode!
#include "x86_tbl_entries.h"
    
    { 0, 0, 0, OP_ABSENT, OP_ABSENT, OP_ABSENT, NULL, I_INVALID }

};
