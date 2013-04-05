#include <assert.h>
#include "x86_disas.h"

const char* XOR_modrm_test[]=
{
    "XOR EAX, [EAX]",
    "XOR EAX, [ECX]",
    "XOR EAX, [EDX]",
    "XOR EAX, [EBX]",
    "XOR EAX, [EAX+EDX*4]",
    "XOR EAX, [90909090h]",
    "XOR EAX, [ESI]",
    "XOR EAX, [EDI]",
    "XOR ECX, [EAX]",
    "XOR ECX, [ECX]",
    "XOR ECX, [EDX]",
    "XOR ECX, [EBX]",
    "XOR ECX, [EAX+EDX*4]",
    "XOR ECX, [90909090h]",
    "XOR ECX, [ESI]",
    "XOR ECX, [EDI]",
    "XOR EDX, [EAX]",
    "XOR EDX, [ECX]",
    "XOR EDX, [EDX]",
    "XOR EDX, [EBX]",
    "XOR EDX, [EAX+EDX*4]",
    "XOR EDX, [90909090h]",
    "XOR EDX, [ESI]",
    "XOR EDX, [EDI]",
    "XOR EBX, [EAX]",
    "XOR EBX, [ECX]",
    "XOR EBX, [EDX]",
    "XOR EBX, [EBX]",
    "XOR EBX, [EAX+EDX*4]",
    "XOR EBX, [90909090h]",
    "XOR EBX, [ESI]",
    "XOR EBX, [EDI]",
    "XOR ESP, [EAX]",
    "XOR ESP, [ECX]",
    "XOR ESP, [EDX]",
    "XOR ESP, [EBX]",
    "XOR ESP, [EAX+EDX*4]",
    "XOR ESP, [90909090h]",
    "XOR ESP, [ESI]",
    "XOR ESP, [EDI]",
    "XOR EBP, [EAX]",
    "XOR EBP, [ECX]",
    "XOR EBP, [EDX]",
    "XOR EBP, [EBX]",
    "XOR EBP, [EAX+EDX*4]",
    "XOR EBP, [90909090h]",
    "XOR EBP, [ESI]",
    "XOR EBP, [EDI]",
    "XOR ESI, [EAX]",
    "XOR ESI, [ECX]",
    "XOR ESI, [EDX]",
    "XOR ESI, [EBX]",
    "XOR ESI, [EAX+EDX*4]",
    "XOR ESI, [90909090h]",
    "XOR ESI, [ESI]",
    "XOR ESI, [EDI]",
    "XOR EDI, [EAX]",
    "XOR EDI, [ECX]",
    "XOR EDI, [EDX]",
    "XOR EDI, [EBX]",
    "XOR EDI, [EAX+EDX*4]",
    "XOR EDI, [90909090h]",
    "XOR EDI, [ESI]",
    "XOR EDI, [EDI]",
    "XOR EAX, [EAX-70h]",
    "XOR EAX, [ECX-70h]",
    "XOR EAX, [EDX-70h]",
    "XOR EAX, [EBX-70h]",
    "XOR EAX, [EAX+EDX*4-70h]",
    "XOR EAX, [EBP-70h]",
    "XOR EAX, [ESI-70h]",
    "XOR EAX, [EDI-70h]",
    "XOR ECX, [EAX-70h]",
    "XOR ECX, [ECX-70h]",
    "XOR ECX, [EDX-70h]",
    "XOR ECX, [EBX-70h]",
    "XOR ECX, [EAX+EDX*4-70h]",
    "XOR ECX, [EBP-70h]",
    "XOR ECX, [ESI-70h]",
    "XOR ECX, [EDI-70h]",
    "XOR EDX, [EAX-70h]",
    "XOR EDX, [ECX-70h]",
    "XOR EDX, [EDX-70h]",
    "XOR EDX, [EBX-70h]",
    "XOR EDX, [EAX+EDX*4-70h]",
    "XOR EDX, [EBP-70h]",
    "XOR EDX, [ESI-70h]",
    "XOR EDX, [EDI-70h]",
    "XOR EBX, [EAX-70h]",
    "XOR EBX, [ECX-70h]",
    "XOR EBX, [EDX-70h]",
    "XOR EBX, [EBX-70h]",
    "XOR EBX, [EAX+EDX*4-70h]",
    "XOR EBX, [EBP-70h]",
    "XOR EBX, [ESI-70h]",
    "XOR EBX, [EDI-70h]",
    "XOR ESP, [EAX-70h]",
    "XOR ESP, [ECX-70h]",
    "XOR ESP, [EDX-70h]",
    "XOR ESP, [EBX-70h]",
    "XOR ESP, [EAX+EDX*4-70h]",
    "XOR ESP, [EBP-70h]",
    "XOR ESP, [ESI-70h]",
    "XOR ESP, [EDI-70h]",
    "XOR EBP, [EAX-70h]",
    "XOR EBP, [ECX-70h]",
    "XOR EBP, [EDX-70h]",
    "XOR EBP, [EBX-70h]",
    "XOR EBP, [EAX+EDX*4-70h]",
    "XOR EBP, [EBP-70h]",
    "XOR EBP, [ESI-70h]",
    "XOR EBP, [EDI-70h]",
    "XOR ESI, [EAX-70h]",
    "XOR ESI, [ECX-70h]",
    "XOR ESI, [EDX-70h]",
    "XOR ESI, [EBX-70h]",
    "XOR ESI, [EAX+EDX*4-70h]",
    "XOR ESI, [EBP-70h]",
    "XOR ESI, [ESI-70h]",
    "XOR ESI, [EDI-70h]",
    "XOR EDI, [EAX-70h]",
    "XOR EDI, [ECX-70h]",
    "XOR EDI, [EDX-70h]",
    "XOR EDI, [EBX-70h]",
    "XOR EDI, [EAX+EDX*4-70h]",
    "XOR EDI, [EBP-70h]",
    "XOR EDI, [ESI-70h]",
    "XOR EDI, [EDI-70h]",
    "XOR EAX, [EAX-6f6f6f70h]",
    "XOR EAX, [ECX-6f6f6f70h]",
    "XOR EAX, [EDX-6f6f6f70h]",
    "XOR EAX, [EBX-6f6f6f70h]",
    "XOR EAX, [EAX+EDX*4-6f6f6f70h]",
    "XOR EAX, [EBP-6f6f6f70h]",
    "XOR EAX, [ESI-6f6f6f70h]",
    "XOR EAX, [EDI-6f6f6f70h]",
    "XOR ECX, [EAX-6f6f6f70h]",
    "XOR ECX, [ECX-6f6f6f70h]",
    "XOR ECX, [EDX-6f6f6f70h]",
    "XOR ECX, [EBX-6f6f6f70h]",
    "XOR ECX, [EAX+EDX*4-6f6f6f70h]",
    "XOR ECX, [EBP-6f6f6f70h]",
    "XOR ECX, [ESI-6f6f6f70h]",
    "XOR ECX, [EDI-6f6f6f70h]",
    "XOR EDX, [EAX-6f6f6f70h]",
    "XOR EDX, [ECX-6f6f6f70h]",
    "XOR EDX, [EDX-6f6f6f70h]",
    "XOR EDX, [EBX-6f6f6f70h]",
    "XOR EDX, [EAX+EDX*4-6f6f6f70h]",
    "XOR EDX, [EBP-6f6f6f70h]",
    "XOR EDX, [ESI-6f6f6f70h]",
    "XOR EDX, [EDI-6f6f6f70h]",
    "XOR EBX, [EAX-6f6f6f70h]",
    "XOR EBX, [ECX-6f6f6f70h]",
    "XOR EBX, [EDX-6f6f6f70h]",
    "XOR EBX, [EBX-6f6f6f70h]",
    "XOR EBX, [EAX+EDX*4-6f6f6f70h]",
    "XOR EBX, [EBP-6f6f6f70h]",
    "XOR EBX, [ESI-6f6f6f70h]",
    "XOR EBX, [EDI-6f6f6f70h]",
    "XOR ESP, [EAX-6f6f6f70h]",
    "XOR ESP, [ECX-6f6f6f70h]",
    "XOR ESP, [EDX-6f6f6f70h]",
    "XOR ESP, [EBX-6f6f6f70h]",
    "XOR ESP, [EAX+EDX*4-6f6f6f70h]",
    "XOR ESP, [EBP-6f6f6f70h]",
    "XOR ESP, [ESI-6f6f6f70h]",
    "XOR ESP, [EDI-6f6f6f70h]",
    "XOR EBP, [EAX-6f6f6f70h]",
    "XOR EBP, [ECX-6f6f6f70h]",
    "XOR EBP, [EDX-6f6f6f70h]",
    "XOR EBP, [EBX-6f6f6f70h]",
    "XOR EBP, [EAX+EDX*4-6f6f6f70h]",
    "XOR EBP, [EBP-6f6f6f70h]",
    "XOR EBP, [ESI-6f6f6f70h]",
    "XOR EBP, [EDI-6f6f6f70h]",
    "XOR ESI, [EAX-6f6f6f70h]",
    "XOR ESI, [ECX-6f6f6f70h]",
    "XOR ESI, [EDX-6f6f6f70h]",
    "XOR ESI, [EBX-6f6f6f70h]",
    "XOR ESI, [EAX+EDX*4-6f6f6f70h]",
    "XOR ESI, [EBP-6f6f6f70h]",
    "XOR ESI, [ESI-6f6f6f70h]",
    "XOR ESI, [EDI-6f6f6f70h]",
    "XOR EDI, [EAX-6f6f6f70h]",
    "XOR EDI, [ECX-6f6f6f70h]",
    "XOR EDI, [EDX-6f6f6f70h]",
    "XOR EDI, [EBX-6f6f6f70h]",
    "XOR EDI, [EAX+EDX*4-6f6f6f70h]",
    "XOR EDI, [EBP-6f6f6f70h]",
    "XOR EDI, [ESI-6f6f6f70h]",
    "XOR EDI, [EDI-6f6f6f70h]",
    "XOR EAX, EAX",
    "XOR EAX, ECX",
    "XOR EAX, EDX",
    "XOR EAX, EBX",
    "XOR EAX, ESP",
    "XOR EAX, EBP",
    "XOR EAX, ESI",
    "XOR EAX, EDI",
    "XOR ECX, EAX",
    "XOR ECX, ECX",
    "XOR ECX, EDX",
    "XOR ECX, EBX",
    "XOR ECX, ESP",
    "XOR ECX, EBP",
    "XOR ECX, ESI",
    "XOR ECX, EDI",
    "XOR EDX, EAX",
    "XOR EDX, ECX",
    "XOR EDX, EDX",
    "XOR EDX, EBX",
    "XOR EDX, ESP",
    "XOR EDX, EBP",
    "XOR EDX, ESI",
    "XOR EDX, EDI",
    "XOR EBX, EAX",
    "XOR EBX, ECX",
    "XOR EBX, EDX",
    "XOR EBX, EBX",
    "XOR EBX, ESP",
    "XOR EBX, EBP",
    "XOR EBX, ESI",
    "XOR EBX, EDI",
    "XOR ESP, EAX",
    "XOR ESP, ECX",
    "XOR ESP, EDX",
    "XOR ESP, EBX",
    "XOR ESP, ESP",
    "XOR ESP, EBP",
    "XOR ESP, ESI",
    "XOR ESP, EDI",
    "XOR EBP, EAX",
    "XOR EBP, ECX",
    "XOR EBP, EDX",
    "XOR EBP, EBX",
    "XOR EBP, ESP",
    "XOR EBP, EBP",
    "XOR EBP, ESI",
    "XOR EBP, EDI",
    "XOR ESI, EAX",
    "XOR ESI, ECX",
    "XOR ESI, EDX",
    "XOR ESI, EBX",
    "XOR ESI, ESP",
    "XOR ESI, EBP",
    "XOR ESI, ESI",
    "XOR ESI, EDI",
    "XOR EDI, EAX",
    "XOR EDI, ECX",
    "XOR EDI, EDX",
    "XOR EDI, EBX",
    "XOR EDI, ESP",
    "XOR EDI, EBP",
    "XOR EDI, ESI",
    "XOR EDI, EDI"
};
const char* XOR_SIB_04_test[]=
{
    "XOR EAX, [EAX+EAX]",
    "XOR EAX, [ECX+EAX]",
    "XOR EAX, [EDX+EAX]",
    "XOR EAX, [EBX+EAX]",
    "XOR EAX, [ESP+EAX]",
    "XOR EAX, [EAX+90909090h]",
    "XOR EAX, [ESI+EAX]",
    "XOR EAX, [EDI+EAX]",
    "XOR EAX, [EAX+ECX]",
    "XOR EAX, [ECX+ECX]",
    "XOR EAX, [EDX+ECX]",
    "XOR EAX, [EBX+ECX]",
    "XOR EAX, [ESP+ECX]",
    "XOR EAX, [ECX+90909090h]",
    "XOR EAX, [ESI+ECX]",
    "XOR EAX, [EDI+ECX]",
    "XOR EAX, [EAX+EDX]",
    "XOR EAX, [ECX+EDX]",
    "XOR EAX, [EDX+EDX]",
    "XOR EAX, [EBX+EDX]",
    "XOR EAX, [ESP+EDX]",
    "XOR EAX, [EDX+90909090h]",
    "XOR EAX, [ESI+EDX]",
    "XOR EAX, [EDI+EDX]",
    "XOR EAX, [EAX+EBX]",
    "XOR EAX, [ECX+EBX]",
    "XOR EAX, [EDX+EBX]",
    "XOR EAX, [EBX+EBX]",
    "XOR EAX, [ESP+EBX]",
    "XOR EAX, [EBX+90909090h]",
    "XOR EAX, [ESI+EBX]",
    "XOR EAX, [EDI+EBX]",
    "XOR EAX, [EAX]",
    "XOR EAX, [ECX]",
    "XOR EAX, [EDX]",
    "XOR EAX, [EBX]",
    "XOR EAX, [ESP]",
    "XOR EAX, [90909090h]",
    "XOR EAX, [ESI]",
    "XOR EAX, [EDI]",
    "XOR EAX, [EAX+EBP]",
    "XOR EAX, [ECX+EBP]",
    "XOR EAX, [EDX+EBP]",
    "XOR EAX, [EBX+EBP]",
    "XOR EAX, [ESP+EBP]",
    "XOR EAX, [EBP+90909090h]",
    "XOR EAX, [ESI+EBP]",
    "XOR EAX, [EDI+EBP]",
    "XOR EAX, [EAX+ESI]",
    "XOR EAX, [ECX+ESI]",
    "XOR EAX, [EDX+ESI]",
    "XOR EAX, [EBX+ESI]",
    "XOR EAX, [ESP+ESI]",
    "XOR EAX, [ESI+90909090h]",
    "XOR EAX, [ESI+ESI]",
    "XOR EAX, [EDI+ESI]",
    "XOR EAX, [EAX+EDI]",
    "XOR EAX, [ECX+EDI]",
    "XOR EAX, [EDX+EDI]",
    "XOR EAX, [EBX+EDI]",
    "XOR EAX, [ESP+EDI]",
    "XOR EAX, [EDI+90909090h]",
    "XOR EAX, [ESI+EDI]",
    "XOR EAX, [EDI+EDI]",
    "XOR EAX, [EAX+EAX*2]",
    "XOR EAX, [ECX+EAX*2]",
    "XOR EAX, [EDX+EAX*2]",
    "XOR EAX, [EBX+EAX*2]",
    "XOR EAX, [ESP+EAX*2]",
    "XOR EAX, [EAX*2+90909090h]",
    "XOR EAX, [ESI+EAX*2]",
    "XOR EAX, [EDI+EAX*2]",
    "XOR EAX, [EAX+ECX*2]",
    "XOR EAX, [ECX+ECX*2]",
    "XOR EAX, [EDX+ECX*2]",
    "XOR EAX, [EBX+ECX*2]",
    "XOR EAX, [ESP+ECX*2]",
    "XOR EAX, [ECX*2+90909090h]",
    "XOR EAX, [ESI+ECX*2]",
    "XOR EAX, [EDI+ECX*2]",
    "XOR EAX, [EAX+EDX*2]",
    "XOR EAX, [ECX+EDX*2]",
    "XOR EAX, [EDX+EDX*2]",
    "XOR EAX, [EBX+EDX*2]",
    "XOR EAX, [ESP+EDX*2]",
    "XOR EAX, [EDX*2+90909090h]",
    "XOR EAX, [ESI+EDX*2]",
    "XOR EAX, [EDI+EDX*2]",
    "XOR EAX, [EAX+EBX*2]",
    "XOR EAX, [ECX+EBX*2]",
    "XOR EAX, [EDX+EBX*2]",
    "XOR EAX, [EBX+EBX*2]",
    "XOR EAX, [ESP+EBX*2]",
    "XOR EAX, [EBX*2+90909090h]",
    "XOR EAX, [ESI+EBX*2]",
    "XOR EAX, [EDI+EBX*2]",
    "XOR EAX, [EAX]",
    "XOR EAX, [ECX]",
    "XOR EAX, [EDX]",
    "XOR EAX, [EBX]",
    "XOR EAX, [ESP]",
    "XOR EAX, [90909090h]",
    "XOR EAX, [ESI]",
    "XOR EAX, [EDI]",
    "XOR EAX, [EAX+EBP*2]",
    "XOR EAX, [ECX+EBP*2]",
    "XOR EAX, [EDX+EBP*2]",
    "XOR EAX, [EBX+EBP*2]",
    "XOR EAX, [ESP+EBP*2]",
    "XOR EAX, [EBP*2+90909090h]",
    "XOR EAX, [ESI+EBP*2]",
    "XOR EAX, [EDI+EBP*2]",
    "XOR EAX, [EAX+ESI*2]",
    "XOR EAX, [ECX+ESI*2]",
    "XOR EAX, [EDX+ESI*2]",
    "XOR EAX, [EBX+ESI*2]",
    "XOR EAX, [ESP+ESI*2]",
    "XOR EAX, [ESI*2+90909090h]",
    "XOR EAX, [ESI+ESI*2]",
    "XOR EAX, [EDI+ESI*2]",
    "XOR EAX, [EAX+EDI*2]",
    "XOR EAX, [ECX+EDI*2]",
    "XOR EAX, [EDX+EDI*2]",
    "XOR EAX, [EBX+EDI*2]",
    "XOR EAX, [ESP+EDI*2]",
    "XOR EAX, [EDI*2+90909090h]",
    "XOR EAX, [ESI+EDI*2]",
    "XOR EAX, [EDI+EDI*2]",
    "XOR EAX, [EAX+EAX*4]",
    "XOR EAX, [ECX+EAX*4]",
    "XOR EAX, [EDX+EAX*4]",
    "XOR EAX, [EBX+EAX*4]",
    "XOR EAX, [ESP+EAX*4]",
    "XOR EAX, [EAX*4+90909090h]",
    "XOR EAX, [ESI+EAX*4]",
    "XOR EAX, [EDI+EAX*4]",
    "XOR EAX, [EAX+ECX*4]",
    "XOR EAX, [ECX+ECX*4]",
    "XOR EAX, [EDX+ECX*4]",
    "XOR EAX, [EBX+ECX*4]",
    "XOR EAX, [ESP+ECX*4]",
    "XOR EAX, [ECX*4+90909090h]",
    "XOR EAX, [ESI+ECX*4]",
    "XOR EAX, [EDI+ECX*4]",
    "XOR EAX, [EAX+EDX*4]",
    "XOR EAX, [ECX+EDX*4]",
    "XOR EAX, [EDX+EDX*4]",
    "XOR EAX, [EBX+EDX*4]",
    "XOR EAX, [ESP+EDX*4]",
    "XOR EAX, [EDX*4+90909090h]",
    "XOR EAX, [ESI+EDX*4]",
    "XOR EAX, [EDI+EDX*4]",
    "XOR EAX, [EAX+EBX*4]",
    "XOR EAX, [ECX+EBX*4]",
    "XOR EAX, [EDX+EBX*4]",
    "XOR EAX, [EBX+EBX*4]",
    "XOR EAX, [ESP+EBX*4]",
    "XOR EAX, [EBX*4+90909090h]",
    "XOR EAX, [ESI+EBX*4]",
    "XOR EAX, [EDI+EBX*4]",
    "XOR EAX, [EAX]",
    "XOR EAX, [ECX]",
    "XOR EAX, [EDX]",
    "XOR EAX, [EBX]",
    "XOR EAX, [ESP]",
    "XOR EAX, [90909090h]",
    "XOR EAX, [ESI]",
    "XOR EAX, [EDI]",
    "XOR EAX, [EAX+EBP*4]",
    "XOR EAX, [ECX+EBP*4]",
    "XOR EAX, [EDX+EBP*4]",
    "XOR EAX, [EBX+EBP*4]",
    "XOR EAX, [ESP+EBP*4]",
    "XOR EAX, [EBP*4+90909090h]",
    "XOR EAX, [ESI+EBP*4]",
    "XOR EAX, [EDI+EBP*4]",
    "XOR EAX, [EAX+ESI*4]",
    "XOR EAX, [ECX+ESI*4]",
    "XOR EAX, [EDX+ESI*4]",
    "XOR EAX, [EBX+ESI*4]",
    "XOR EAX, [ESP+ESI*4]",
    "XOR EAX, [ESI*4+90909090h]",
    "XOR EAX, [ESI+ESI*4]",
    "XOR EAX, [EDI+ESI*4]",
    "XOR EAX, [EAX+EDI*4]",
    "XOR EAX, [ECX+EDI*4]",
    "XOR EAX, [EDX+EDI*4]",
    "XOR EAX, [EBX+EDI*4]",
    "XOR EAX, [ESP+EDI*4]",
    "XOR EAX, [EDI*4+90909090h]",
    "XOR EAX, [ESI+EDI*4]",
    "XOR EAX, [EDI+EDI*4]",
    "XOR EAX, [EAX+EAX*8]",
    "XOR EAX, [ECX+EAX*8]",
    "XOR EAX, [EDX+EAX*8]",
    "XOR EAX, [EBX+EAX*8]",
    "XOR EAX, [ESP+EAX*8]",
    "XOR EAX, [EAX*8+90909090h]",
    "XOR EAX, [ESI+EAX*8]",
    "XOR EAX, [EDI+EAX*8]",
    "XOR EAX, [EAX+ECX*8]",
    "XOR EAX, [ECX+ECX*8]",
    "XOR EAX, [EDX+ECX*8]",
    "XOR EAX, [EBX+ECX*8]",
    "XOR EAX, [ESP+ECX*8]",
    "XOR EAX, [ECX*8+90909090h]",
    "XOR EAX, [ESI+ECX*8]",
    "XOR EAX, [EDI+ECX*8]",
    "XOR EAX, [EAX+EDX*8]",
    "XOR EAX, [ECX+EDX*8]",
    "XOR EAX, [EDX+EDX*8]",
    "XOR EAX, [EBX+EDX*8]",
    "XOR EAX, [ESP+EDX*8]",
    "XOR EAX, [EDX*8+90909090h]",
    "XOR EAX, [ESI+EDX*8]",
    "XOR EAX, [EDI+EDX*8]",
    "XOR EAX, [EAX+EBX*8]",
    "XOR EAX, [ECX+EBX*8]",
    "XOR EAX, [EDX+EBX*8]",
    "XOR EAX, [EBX+EBX*8]",
    "XOR EAX, [ESP+EBX*8]",
    "XOR EAX, [EBX*8+90909090h]",
    "XOR EAX, [ESI+EBX*8]",
    "XOR EAX, [EDI+EBX*8]",
    "XOR EAX, [EAX]",
    "XOR EAX, [ECX]",
    "XOR EAX, [EDX]",
    "XOR EAX, [EBX]",
    "XOR EAX, [ESP]",
    "XOR EAX, [90909090h]",
    "XOR EAX, [ESI]",
    "XOR EAX, [EDI]",
    "XOR EAX, [EAX+EBP*8]",
    "XOR EAX, [ECX+EBP*8]",
    "XOR EAX, [EDX+EBP*8]",
    "XOR EAX, [EBX+EBP*8]",
    "XOR EAX, [ESP+EBP*8]",
    "XOR EAX, [EBP*8+90909090h]",
    "XOR EAX, [ESI+EBP*8]",
    "XOR EAX, [EDI+EBP*8]",
    "XOR EAX, [EAX+ESI*8]",
    "XOR EAX, [ECX+ESI*8]",
    "XOR EAX, [EDX+ESI*8]",
    "XOR EAX, [EBX+ESI*8]",
    "XOR EAX, [ESP+ESI*8]",
    "XOR EAX, [ESI*8+90909090h]",
    "XOR EAX, [ESI+ESI*8]",
    "XOR EAX, [EDI+ESI*8]",
    "XOR EAX, [EAX+EDI*8]",
    "XOR EAX, [ECX+EDI*8]",
    "XOR EAX, [EDX+EDI*8]",
    "XOR EAX, [EBX+EDI*8]",
    "XOR EAX, [ESP+EDI*8]",
    "XOR EAX, [EDI*8+90909090h]",
    "XOR EAX, [ESI+EDI*8]",
    "XOR EAX, [EDI+EDI*8]"
};
const char* XOR_SIB_44_test[]=
{
    "XOR EAX, [EAX+EAX-70h]",
    "XOR EAX, [ECX+EAX-70h]",
    "XOR EAX, [EDX+EAX-70h]",
    "XOR EAX, [EBX+EAX-70h]",
    "XOR EAX, [ESP+EAX-70h]",
    "XOR EAX, [EBP+EAX-70h]",
    "XOR EAX, [ESI+EAX-70h]",
    "XOR EAX, [EDI+EAX-70h]",
    "XOR EAX, [EAX+ECX-70h]",
    "XOR EAX, [ECX+ECX-70h]",
    "XOR EAX, [EDX+ECX-70h]",
    "XOR EAX, [EBX+ECX-70h]",
    "XOR EAX, [ESP+ECX-70h]",
    "XOR EAX, [EBP+ECX-70h]",
    "XOR EAX, [ESI+ECX-70h]",
    "XOR EAX, [EDI+ECX-70h]",
    "XOR EAX, [EAX+EDX-70h]",
    "XOR EAX, [ECX+EDX-70h]",
    "XOR EAX, [EDX+EDX-70h]",
    "XOR EAX, [EBX+EDX-70h]",
    "XOR EAX, [ESP+EDX-70h]",
    "XOR EAX, [EBP+EDX-70h]",
    "XOR EAX, [ESI+EDX-70h]",
    "XOR EAX, [EDI+EDX-70h]",
    "XOR EAX, [EAX+EBX-70h]",
    "XOR EAX, [ECX+EBX-70h]",
    "XOR EAX, [EDX+EBX-70h]",
    "XOR EAX, [EBX+EBX-70h]",
    "XOR EAX, [ESP+EBX-70h]",
    "XOR EAX, [EBP+EBX-70h]",
    "XOR EAX, [ESI+EBX-70h]",
    "XOR EAX, [EDI+EBX-70h]",
    "XOR EAX, [EAX-70h]",
    "XOR EAX, [ECX-70h]",
    "XOR EAX, [EDX-70h]",
    "XOR EAX, [EBX-70h]",
    "XOR EAX, [ESP-70h]",
    "XOR EAX, [EBP-70h]",
    "XOR EAX, [ESI-70h]",
    "XOR EAX, [EDI-70h]",
    "XOR EAX, [EAX+EBP-70h]",
    "XOR EAX, [ECX+EBP-70h]",
    "XOR EAX, [EDX+EBP-70h]",
    "XOR EAX, [EBX+EBP-70h]",
    "XOR EAX, [ESP+EBP-70h]",
    "XOR EAX, [EBP+EBP-70h]",
    "XOR EAX, [ESI+EBP-70h]",
    "XOR EAX, [EDI+EBP-70h]",
    "XOR EAX, [EAX+ESI-70h]",
    "XOR EAX, [ECX+ESI-70h]",
    "XOR EAX, [EDX+ESI-70h]",
    "XOR EAX, [EBX+ESI-70h]",
    "XOR EAX, [ESP+ESI-70h]",
    "XOR EAX, [EBP+ESI-70h]",
    "XOR EAX, [ESI+ESI-70h]",
    "XOR EAX, [EDI+ESI-70h]",
    "XOR EAX, [EAX+EDI-70h]",
    "XOR EAX, [ECX+EDI-70h]",
    "XOR EAX, [EDX+EDI-70h]",
    "XOR EAX, [EBX+EDI-70h]",
    "XOR EAX, [ESP+EDI-70h]",
    "XOR EAX, [EBP+EDI-70h]",
    "XOR EAX, [ESI+EDI-70h]",
    "XOR EAX, [EDI+EDI-70h]",
    "XOR EAX, [EAX+EAX*2-70h]",
    "XOR EAX, [ECX+EAX*2-70h]",
    "XOR EAX, [EDX+EAX*2-70h]",
    "XOR EAX, [EBX+EAX*2-70h]",
    "XOR EAX, [ESP+EAX*2-70h]",
    "XOR EAX, [EBP+EAX*2-70h]",
    "XOR EAX, [ESI+EAX*2-70h]",
    "XOR EAX, [EDI+EAX*2-70h]",
    "XOR EAX, [EAX+ECX*2-70h]",
    "XOR EAX, [ECX+ECX*2-70h]",
    "XOR EAX, [EDX+ECX*2-70h]",
    "XOR EAX, [EBX+ECX*2-70h]",
    "XOR EAX, [ESP+ECX*2-70h]",
    "XOR EAX, [EBP+ECX*2-70h]",
    "XOR EAX, [ESI+ECX*2-70h]",
    "XOR EAX, [EDI+ECX*2-70h]",
    "XOR EAX, [EAX+EDX*2-70h]",
    "XOR EAX, [ECX+EDX*2-70h]",
    "XOR EAX, [EDX+EDX*2-70h]",
    "XOR EAX, [EBX+EDX*2-70h]",
    "XOR EAX, [ESP+EDX*2-70h]",
    "XOR EAX, [EBP+EDX*2-70h]",
    "XOR EAX, [ESI+EDX*2-70h]",
    "XOR EAX, [EDI+EDX*2-70h]",
    "XOR EAX, [EAX+EBX*2-70h]",
    "XOR EAX, [ECX+EBX*2-70h]",
    "XOR EAX, [EDX+EBX*2-70h]",
    "XOR EAX, [EBX+EBX*2-70h]",
    "XOR EAX, [ESP+EBX*2-70h]",
    "XOR EAX, [EBP+EBX*2-70h]",
    "XOR EAX, [ESI+EBX*2-70h]",
    "XOR EAX, [EDI+EBX*2-70h]",
    "XOR EAX, [EAX-70h]",
    "XOR EAX, [ECX-70h]",
    "XOR EAX, [EDX-70h]",
    "XOR EAX, [EBX-70h]",
    "XOR EAX, [ESP-70h]",
    "XOR EAX, [EBP-70h]",
    "XOR EAX, [ESI-70h]",
    "XOR EAX, [EDI-70h]",
    "XOR EAX, [EAX+EBP*2-70h]",
    "XOR EAX, [ECX+EBP*2-70h]",
    "XOR EAX, [EDX+EBP*2-70h]",
    "XOR EAX, [EBX+EBP*2-70h]",
    "XOR EAX, [ESP+EBP*2-70h]",
    "XOR EAX, [EBP+EBP*2-70h]",
    "XOR EAX, [ESI+EBP*2-70h]",
    "XOR EAX, [EDI+EBP*2-70h]",
    "XOR EAX, [EAX+ESI*2-70h]",
    "XOR EAX, [ECX+ESI*2-70h]",
    "XOR EAX, [EDX+ESI*2-70h]",
    "XOR EAX, [EBX+ESI*2-70h]",
    "XOR EAX, [ESP+ESI*2-70h]",
    "XOR EAX, [EBP+ESI*2-70h]",
    "XOR EAX, [ESI+ESI*2-70h]",
    "XOR EAX, [EDI+ESI*2-70h]",
    "XOR EAX, [EAX+EDI*2-70h]",
    "XOR EAX, [ECX+EDI*2-70h]",
    "XOR EAX, [EDX+EDI*2-70h]",
    "XOR EAX, [EBX+EDI*2-70h]",
    "XOR EAX, [ESP+EDI*2-70h]",
    "XOR EAX, [EBP+EDI*2-70h]",
    "XOR EAX, [ESI+EDI*2-70h]",
    "XOR EAX, [EDI+EDI*2-70h]",
    "XOR EAX, [EAX+EAX*4-70h]",
    "XOR EAX, [ECX+EAX*4-70h]",
    "XOR EAX, [EDX+EAX*4-70h]",
    "XOR EAX, [EBX+EAX*4-70h]",
    "XOR EAX, [ESP+EAX*4-70h]",
    "XOR EAX, [EBP+EAX*4-70h]",
    "XOR EAX, [ESI+EAX*4-70h]",
    "XOR EAX, [EDI+EAX*4-70h]",
    "XOR EAX, [EAX+ECX*4-70h]",
    "XOR EAX, [ECX+ECX*4-70h]",
    "XOR EAX, [EDX+ECX*4-70h]",
    "XOR EAX, [EBX+ECX*4-70h]",
    "XOR EAX, [ESP+ECX*4-70h]",
    "XOR EAX, [EBP+ECX*4-70h]",
    "XOR EAX, [ESI+ECX*4-70h]",
    "XOR EAX, [EDI+ECX*4-70h]",
    "XOR EAX, [EAX+EDX*4-70h]",
    "XOR EAX, [ECX+EDX*4-70h]",
    "XOR EAX, [EDX+EDX*4-70h]",
    "XOR EAX, [EBX+EDX*4-70h]",
    "XOR EAX, [ESP+EDX*4-70h]",
    "XOR EAX, [EBP+EDX*4-70h]",
    "XOR EAX, [ESI+EDX*4-70h]",
    "XOR EAX, [EDI+EDX*4-70h]",
    "XOR EAX, [EAX+EBX*4-70h]",
    "XOR EAX, [ECX+EBX*4-70h]",
    "XOR EAX, [EDX+EBX*4-70h]",
    "XOR EAX, [EBX+EBX*4-70h]",
    "XOR EAX, [ESP+EBX*4-70h]",
    "XOR EAX, [EBP+EBX*4-70h]",
    "XOR EAX, [ESI+EBX*4-70h]",
    "XOR EAX, [EDI+EBX*4-70h]",
    "XOR EAX, [EAX-70h]",
    "XOR EAX, [ECX-70h]",
    "XOR EAX, [EDX-70h]",
    "XOR EAX, [EBX-70h]",
    "XOR EAX, [ESP-70h]",
    "XOR EAX, [EBP-70h]",
    "XOR EAX, [ESI-70h]",
    "XOR EAX, [EDI-70h]",
    "XOR EAX, [EAX+EBP*4-70h]",
    "XOR EAX, [ECX+EBP*4-70h]",
    "XOR EAX, [EDX+EBP*4-70h]",
    "XOR EAX, [EBX+EBP*4-70h]",
    "XOR EAX, [ESP+EBP*4-70h]",
    "XOR EAX, [EBP+EBP*4-70h]",
    "XOR EAX, [ESI+EBP*4-70h]",
    "XOR EAX, [EDI+EBP*4-70h]",
    "XOR EAX, [EAX+ESI*4-70h]",
    "XOR EAX, [ECX+ESI*4-70h]",
    "XOR EAX, [EDX+ESI*4-70h]",
    "XOR EAX, [EBX+ESI*4-70h]",
    "XOR EAX, [ESP+ESI*4-70h]",
    "XOR EAX, [EBP+ESI*4-70h]",
    "XOR EAX, [ESI+ESI*4-70h]",
    "XOR EAX, [EDI+ESI*4-70h]",
    "XOR EAX, [EAX+EDI*4-70h]",
    "XOR EAX, [ECX+EDI*4-70h]",
    "XOR EAX, [EDX+EDI*4-70h]",
    "XOR EAX, [EBX+EDI*4-70h]",
    "XOR EAX, [ESP+EDI*4-70h]",
    "XOR EAX, [EBP+EDI*4-70h]",
    "XOR EAX, [ESI+EDI*4-70h]",
    "XOR EAX, [EDI+EDI*4-70h]",
    "XOR EAX, [EAX+EAX*8-70h]",
    "XOR EAX, [ECX+EAX*8-70h]",
    "XOR EAX, [EDX+EAX*8-70h]",
    "XOR EAX, [EBX+EAX*8-70h]",
    "XOR EAX, [ESP+EAX*8-70h]",
    "XOR EAX, [EBP+EAX*8-70h]",
    "XOR EAX, [ESI+EAX*8-70h]",
    "XOR EAX, [EDI+EAX*8-70h]",
    "XOR EAX, [EAX+ECX*8-70h]",
    "XOR EAX, [ECX+ECX*8-70h]",
    "XOR EAX, [EDX+ECX*8-70h]",
    "XOR EAX, [EBX+ECX*8-70h]",
    "XOR EAX, [ESP+ECX*8-70h]",
    "XOR EAX, [EBP+ECX*8-70h]",
    "XOR EAX, [ESI+ECX*8-70h]",
    "XOR EAX, [EDI+ECX*8-70h]",
    "XOR EAX, [EAX+EDX*8-70h]",
    "XOR EAX, [ECX+EDX*8-70h]",
    "XOR EAX, [EDX+EDX*8-70h]",
    "XOR EAX, [EBX+EDX*8-70h]",
    "XOR EAX, [ESP+EDX*8-70h]",
    "XOR EAX, [EBP+EDX*8-70h]",
    "XOR EAX, [ESI+EDX*8-70h]",
    "XOR EAX, [EDI+EDX*8-70h]",
    "XOR EAX, [EAX+EBX*8-70h]",
    "XOR EAX, [ECX+EBX*8-70h]",
    "XOR EAX, [EDX+EBX*8-70h]",
    "XOR EAX, [EBX+EBX*8-70h]",
    "XOR EAX, [ESP+EBX*8-70h]",
    "XOR EAX, [EBP+EBX*8-70h]",
    "XOR EAX, [ESI+EBX*8-70h]",
    "XOR EAX, [EDI+EBX*8-70h]",
    "XOR EAX, [EAX-70h]",
    "XOR EAX, [ECX-70h]",
    "XOR EAX, [EDX-70h]",
    "XOR EAX, [EBX-70h]",
    "XOR EAX, [ESP-70h]",
    "XOR EAX, [EBP-70h]",
    "XOR EAX, [ESI-70h]",
    "XOR EAX, [EDI-70h]",
    "XOR EAX, [EAX+EBP*8-70h]",
    "XOR EAX, [ECX+EBP*8-70h]",
    "XOR EAX, [EDX+EBP*8-70h]",
    "XOR EAX, [EBX+EBP*8-70h]",
    "XOR EAX, [ESP+EBP*8-70h]",
    "XOR EAX, [EBP+EBP*8-70h]",
    "XOR EAX, [ESI+EBP*8-70h]",
    "XOR EAX, [EDI+EBP*8-70h]",
    "XOR EAX, [EAX+ESI*8-70h]",
    "XOR EAX, [ECX+ESI*8-70h]",
    "XOR EAX, [EDX+ESI*8-70h]",
    "XOR EAX, [EBX+ESI*8-70h]",
    "XOR EAX, [ESP+ESI*8-70h]",
    "XOR EAX, [EBP+ESI*8-70h]",
    "XOR EAX, [ESI+ESI*8-70h]",
    "XOR EAX, [EDI+ESI*8-70h]",
    "XOR EAX, [EAX+EDI*8-70h]",
    "XOR EAX, [ECX+EDI*8-70h]",
    "XOR EAX, [EDX+EDI*8-70h]",
    "XOR EAX, [EBX+EDI*8-70h]",
    "XOR EAX, [ESP+EDI*8-70h]",
    "XOR EAX, [EBP+EDI*8-70h]",
    "XOR EAX, [ESI+EDI*8-70h]",
    "XOR EAX, [EDI+EDI*8-70h]"
};
const char* XOR_SIB_84_test[]=
{
    "XOR EAX, [EAX+EAX-6f6f6f70h]",
    "XOR EAX, [ECX+EAX-6f6f6f70h]",
    "XOR EAX, [EDX+EAX-6f6f6f70h]",
    "XOR EAX, [EBX+EAX-6f6f6f70h]",
    "XOR EAX, [ESP+EAX-6f6f6f70h]",
    "XOR EAX, [EBP+EAX-6f6f6f70h]",
    "XOR EAX, [ESI+EAX-6f6f6f70h]",
    "XOR EAX, [EDI+EAX-6f6f6f70h]",
    "XOR EAX, [EAX+ECX-6f6f6f70h]",
    "XOR EAX, [ECX+ECX-6f6f6f70h]",
    "XOR EAX, [EDX+ECX-6f6f6f70h]",
    "XOR EAX, [EBX+ECX-6f6f6f70h]",
    "XOR EAX, [ESP+ECX-6f6f6f70h]",
    "XOR EAX, [EBP+ECX-6f6f6f70h]",
    "XOR EAX, [ESI+ECX-6f6f6f70h]",
    "XOR EAX, [EDI+ECX-6f6f6f70h]",
    "XOR EAX, [EAX+EDX-6f6f6f70h]",
    "XOR EAX, [ECX+EDX-6f6f6f70h]",
    "XOR EAX, [EDX+EDX-6f6f6f70h]",
    "XOR EAX, [EBX+EDX-6f6f6f70h]",
    "XOR EAX, [ESP+EDX-6f6f6f70h]",
    "XOR EAX, [EBP+EDX-6f6f6f70h]",
    "XOR EAX, [ESI+EDX-6f6f6f70h]",
    "XOR EAX, [EDI+EDX-6f6f6f70h]",
    "XOR EAX, [EAX+EBX-6f6f6f70h]",
    "XOR EAX, [ECX+EBX-6f6f6f70h]",
    "XOR EAX, [EDX+EBX-6f6f6f70h]",
    "XOR EAX, [EBX+EBX-6f6f6f70h]",
    "XOR EAX, [ESP+EBX-6f6f6f70h]",
    "XOR EAX, [EBP+EBX-6f6f6f70h]",
    "XOR EAX, [ESI+EBX-6f6f6f70h]",
    "XOR EAX, [EDI+EBX-6f6f6f70h]",
    "XOR EAX, [EAX-6f6f6f70h]",
    "XOR EAX, [ECX-6f6f6f70h]",
    "XOR EAX, [EDX-6f6f6f70h]",
    "XOR EAX, [EBX-6f6f6f70h]",
    "XOR EAX, [ESP-6f6f6f70h]",
    "XOR EAX, [EBP-6f6f6f70h]",
    "XOR EAX, [ESI-6f6f6f70h]",
    "XOR EAX, [EDI-6f6f6f70h]",
    "XOR EAX, [EAX+EBP-6f6f6f70h]",
    "XOR EAX, [ECX+EBP-6f6f6f70h]",
    "XOR EAX, [EDX+EBP-6f6f6f70h]",
    "XOR EAX, [EBX+EBP-6f6f6f70h]",
    "XOR EAX, [ESP+EBP-6f6f6f70h]",
    "XOR EAX, [EBP+EBP-6f6f6f70h]",
    "XOR EAX, [ESI+EBP-6f6f6f70h]",
    "XOR EAX, [EDI+EBP-6f6f6f70h]",
    "XOR EAX, [EAX+ESI-6f6f6f70h]",
    "XOR EAX, [ECX+ESI-6f6f6f70h]",
    "XOR EAX, [EDX+ESI-6f6f6f70h]",
    "XOR EAX, [EBX+ESI-6f6f6f70h]",
    "XOR EAX, [ESP+ESI-6f6f6f70h]",
    "XOR EAX, [EBP+ESI-6f6f6f70h]",
    "XOR EAX, [ESI+ESI-6f6f6f70h]",
    "XOR EAX, [EDI+ESI-6f6f6f70h]",
    "XOR EAX, [EAX+EDI-6f6f6f70h]",
    "XOR EAX, [ECX+EDI-6f6f6f70h]",
    "XOR EAX, [EDX+EDI-6f6f6f70h]",
    "XOR EAX, [EBX+EDI-6f6f6f70h]",
    "XOR EAX, [ESP+EDI-6f6f6f70h]",
    "XOR EAX, [EBP+EDI-6f6f6f70h]",
    "XOR EAX, [ESI+EDI-6f6f6f70h]",
    "XOR EAX, [EDI+EDI-6f6f6f70h]",
    "XOR EAX, [EAX+EAX*2-6f6f6f70h]",
    "XOR EAX, [ECX+EAX*2-6f6f6f70h]",
    "XOR EAX, [EDX+EAX*2-6f6f6f70h]",
    "XOR EAX, [EBX+EAX*2-6f6f6f70h]",
    "XOR EAX, [ESP+EAX*2-6f6f6f70h]",
    "XOR EAX, [EBP+EAX*2-6f6f6f70h]",
    "XOR EAX, [ESI+EAX*2-6f6f6f70h]",
    "XOR EAX, [EDI+EAX*2-6f6f6f70h]",
    "XOR EAX, [EAX+ECX*2-6f6f6f70h]",
    "XOR EAX, [ECX+ECX*2-6f6f6f70h]",
    "XOR EAX, [EDX+ECX*2-6f6f6f70h]",
    "XOR EAX, [EBX+ECX*2-6f6f6f70h]",
    "XOR EAX, [ESP+ECX*2-6f6f6f70h]",
    "XOR EAX, [EBP+ECX*2-6f6f6f70h]",
    "XOR EAX, [ESI+ECX*2-6f6f6f70h]",
    "XOR EAX, [EDI+ECX*2-6f6f6f70h]",
    "XOR EAX, [EAX+EDX*2-6f6f6f70h]",
    "XOR EAX, [ECX+EDX*2-6f6f6f70h]",
    "XOR EAX, [EDX+EDX*2-6f6f6f70h]",
    "XOR EAX, [EBX+EDX*2-6f6f6f70h]",
    "XOR EAX, [ESP+EDX*2-6f6f6f70h]",
    "XOR EAX, [EBP+EDX*2-6f6f6f70h]",
    "XOR EAX, [ESI+EDX*2-6f6f6f70h]",
    "XOR EAX, [EDI+EDX*2-6f6f6f70h]",
    "XOR EAX, [EAX+EBX*2-6f6f6f70h]",
    "XOR EAX, [ECX+EBX*2-6f6f6f70h]",
    "XOR EAX, [EDX+EBX*2-6f6f6f70h]",
    "XOR EAX, [EBX+EBX*2-6f6f6f70h]",
    "XOR EAX, [ESP+EBX*2-6f6f6f70h]",
    "XOR EAX, [EBP+EBX*2-6f6f6f70h]",
    "XOR EAX, [ESI+EBX*2-6f6f6f70h]",
    "XOR EAX, [EDI+EBX*2-6f6f6f70h]",
    "XOR EAX, [EAX-6f6f6f70h]",
    "XOR EAX, [ECX-6f6f6f70h]",
    "XOR EAX, [EDX-6f6f6f70h]",
    "XOR EAX, [EBX-6f6f6f70h]",
    "XOR EAX, [ESP-6f6f6f70h]",
    "XOR EAX, [EBP-6f6f6f70h]",
    "XOR EAX, [ESI-6f6f6f70h]",
    "XOR EAX, [EDI-6f6f6f70h]",
    "XOR EAX, [EAX+EBP*2-6f6f6f70h]",
    "XOR EAX, [ECX+EBP*2-6f6f6f70h]",
    "XOR EAX, [EDX+EBP*2-6f6f6f70h]",
    "XOR EAX, [EBX+EBP*2-6f6f6f70h]",
    "XOR EAX, [ESP+EBP*2-6f6f6f70h]",
    "XOR EAX, [EBP+EBP*2-6f6f6f70h]",
    "XOR EAX, [ESI+EBP*2-6f6f6f70h]",
    "XOR EAX, [EDI+EBP*2-6f6f6f70h]",
    "XOR EAX, [EAX+ESI*2-6f6f6f70h]",
    "XOR EAX, [ECX+ESI*2-6f6f6f70h]",
    "XOR EAX, [EDX+ESI*2-6f6f6f70h]",
    "XOR EAX, [EBX+ESI*2-6f6f6f70h]",
    "XOR EAX, [ESP+ESI*2-6f6f6f70h]",
    "XOR EAX, [EBP+ESI*2-6f6f6f70h]",
    "XOR EAX, [ESI+ESI*2-6f6f6f70h]",
    "XOR EAX, [EDI+ESI*2-6f6f6f70h]",
    "XOR EAX, [EAX+EDI*2-6f6f6f70h]",
    "XOR EAX, [ECX+EDI*2-6f6f6f70h]",
    "XOR EAX, [EDX+EDI*2-6f6f6f70h]",
    "XOR EAX, [EBX+EDI*2-6f6f6f70h]",
    "XOR EAX, [ESP+EDI*2-6f6f6f70h]",
    "XOR EAX, [EBP+EDI*2-6f6f6f70h]",
    "XOR EAX, [ESI+EDI*2-6f6f6f70h]",
    "XOR EAX, [EDI+EDI*2-6f6f6f70h]",
    "XOR EAX, [EAX+EAX*4-6f6f6f70h]",
    "XOR EAX, [ECX+EAX*4-6f6f6f70h]",
    "XOR EAX, [EDX+EAX*4-6f6f6f70h]",
    "XOR EAX, [EBX+EAX*4-6f6f6f70h]",
    "XOR EAX, [ESP+EAX*4-6f6f6f70h]",
    "XOR EAX, [EBP+EAX*4-6f6f6f70h]",
    "XOR EAX, [ESI+EAX*4-6f6f6f70h]",
    "XOR EAX, [EDI+EAX*4-6f6f6f70h]",
    "XOR EAX, [EAX+ECX*4-6f6f6f70h]",
    "XOR EAX, [ECX+ECX*4-6f6f6f70h]",
    "XOR EAX, [EDX+ECX*4-6f6f6f70h]",
    "XOR EAX, [EBX+ECX*4-6f6f6f70h]",
    "XOR EAX, [ESP+ECX*4-6f6f6f70h]",
    "XOR EAX, [EBP+ECX*4-6f6f6f70h]",
    "XOR EAX, [ESI+ECX*4-6f6f6f70h]",
    "XOR EAX, [EDI+ECX*4-6f6f6f70h]",
    "XOR EAX, [EAX+EDX*4-6f6f6f70h]",
    "XOR EAX, [ECX+EDX*4-6f6f6f70h]",
    "XOR EAX, [EDX+EDX*4-6f6f6f70h]",
    "XOR EAX, [EBX+EDX*4-6f6f6f70h]",
    "XOR EAX, [ESP+EDX*4-6f6f6f70h]",
    "XOR EAX, [EBP+EDX*4-6f6f6f70h]",
    "XOR EAX, [ESI+EDX*4-6f6f6f70h]",
    "XOR EAX, [EDI+EDX*4-6f6f6f70h]",
    "XOR EAX, [EAX+EBX*4-6f6f6f70h]",
    "XOR EAX, [ECX+EBX*4-6f6f6f70h]",
    "XOR EAX, [EDX+EBX*4-6f6f6f70h]",
    "XOR EAX, [EBX+EBX*4-6f6f6f70h]",
    "XOR EAX, [ESP+EBX*4-6f6f6f70h]",
    "XOR EAX, [EBP+EBX*4-6f6f6f70h]",
    "XOR EAX, [ESI+EBX*4-6f6f6f70h]",
    "XOR EAX, [EDI+EBX*4-6f6f6f70h]",
    "XOR EAX, [EAX-6f6f6f70h]",
    "XOR EAX, [ECX-6f6f6f70h]",
    "XOR EAX, [EDX-6f6f6f70h]",
    "XOR EAX, [EBX-6f6f6f70h]",
    "XOR EAX, [ESP-6f6f6f70h]",
    "XOR EAX, [EBP-6f6f6f70h]",
    "XOR EAX, [ESI-6f6f6f70h]",
    "XOR EAX, [EDI-6f6f6f70h]",
    "XOR EAX, [EAX+EBP*4-6f6f6f70h]",
    "XOR EAX, [ECX+EBP*4-6f6f6f70h]",
    "XOR EAX, [EDX+EBP*4-6f6f6f70h]",
    "XOR EAX, [EBX+EBP*4-6f6f6f70h]",
    "XOR EAX, [ESP+EBP*4-6f6f6f70h]",
    "XOR EAX, [EBP+EBP*4-6f6f6f70h]",
    "XOR EAX, [ESI+EBP*4-6f6f6f70h]",
    "XOR EAX, [EDI+EBP*4-6f6f6f70h]",
    "XOR EAX, [EAX+ESI*4-6f6f6f70h]",
    "XOR EAX, [ECX+ESI*4-6f6f6f70h]",
    "XOR EAX, [EDX+ESI*4-6f6f6f70h]",
    "XOR EAX, [EBX+ESI*4-6f6f6f70h]",
    "XOR EAX, [ESP+ESI*4-6f6f6f70h]",
    "XOR EAX, [EBP+ESI*4-6f6f6f70h]",
    "XOR EAX, [ESI+ESI*4-6f6f6f70h]",
    "XOR EAX, [EDI+ESI*4-6f6f6f70h]",
    "XOR EAX, [EAX+EDI*4-6f6f6f70h]",
    "XOR EAX, [ECX+EDI*4-6f6f6f70h]",
    "XOR EAX, [EDX+EDI*4-6f6f6f70h]",
    "XOR EAX, [EBX+EDI*4-6f6f6f70h]",
    "XOR EAX, [ESP+EDI*4-6f6f6f70h]",
    "XOR EAX, [EBP+EDI*4-6f6f6f70h]",
    "XOR EAX, [ESI+EDI*4-6f6f6f70h]",
    "XOR EAX, [EDI+EDI*4-6f6f6f70h]",
    "XOR EAX, [EAX+EAX*8-6f6f6f70h]",
    "XOR EAX, [ECX+EAX*8-6f6f6f70h]",
    "XOR EAX, [EDX+EAX*8-6f6f6f70h]",
    "XOR EAX, [EBX+EAX*8-6f6f6f70h]",
    "XOR EAX, [ESP+EAX*8-6f6f6f70h]",
    "XOR EAX, [EBP+EAX*8-6f6f6f70h]",
    "XOR EAX, [ESI+EAX*8-6f6f6f70h]",
    "XOR EAX, [EDI+EAX*8-6f6f6f70h]",
    "XOR EAX, [EAX+ECX*8-6f6f6f70h]",
    "XOR EAX, [ECX+ECX*8-6f6f6f70h]",
    "XOR EAX, [EDX+ECX*8-6f6f6f70h]",
    "XOR EAX, [EBX+ECX*8-6f6f6f70h]",
    "XOR EAX, [ESP+ECX*8-6f6f6f70h]",
    "XOR EAX, [EBP+ECX*8-6f6f6f70h]",
    "XOR EAX, [ESI+ECX*8-6f6f6f70h]",
    "XOR EAX, [EDI+ECX*8-6f6f6f70h]",
    "XOR EAX, [EAX+EDX*8-6f6f6f70h]",
    "XOR EAX, [ECX+EDX*8-6f6f6f70h]",
    "XOR EAX, [EDX+EDX*8-6f6f6f70h]",
    "XOR EAX, [EBX+EDX*8-6f6f6f70h]",
    "XOR EAX, [ESP+EDX*8-6f6f6f70h]",
    "XOR EAX, [EBP+EDX*8-6f6f6f70h]",
    "XOR EAX, [ESI+EDX*8-6f6f6f70h]",
    "XOR EAX, [EDI+EDX*8-6f6f6f70h]",
    "XOR EAX, [EAX+EBX*8-6f6f6f70h]",
    "XOR EAX, [ECX+EBX*8-6f6f6f70h]",
    "XOR EAX, [EDX+EBX*8-6f6f6f70h]",
    "XOR EAX, [EBX+EBX*8-6f6f6f70h]",
    "XOR EAX, [ESP+EBX*8-6f6f6f70h]",
    "XOR EAX, [EBP+EBX*8-6f6f6f70h]",
    "XOR EAX, [ESI+EBX*8-6f6f6f70h]",
    "XOR EAX, [EDI+EBX*8-6f6f6f70h]",
    "XOR EAX, [EAX-6f6f6f70h]",
    "XOR EAX, [ECX-6f6f6f70h]",
    "XOR EAX, [EDX-6f6f6f70h]",
    "XOR EAX, [EBX-6f6f6f70h]",
    "XOR EAX, [ESP-6f6f6f70h]",
    "XOR EAX, [EBP-6f6f6f70h]",
    "XOR EAX, [ESI-6f6f6f70h]",
    "XOR EAX, [EDI-6f6f6f70h]",
    "XOR EAX, [EAX+EBP*8-6f6f6f70h]",
    "XOR EAX, [ECX+EBP*8-6f6f6f70h]",
    "XOR EAX, [EDX+EBP*8-6f6f6f70h]",
    "XOR EAX, [EBX+EBP*8-6f6f6f70h]",
    "XOR EAX, [ESP+EBP*8-6f6f6f70h]",
    "XOR EAX, [EBP+EBP*8-6f6f6f70h]",
    "XOR EAX, [ESI+EBP*8-6f6f6f70h]",
    "XOR EAX, [EDI+EBP*8-6f6f6f70h]",
    "XOR EAX, [EAX+ESI*8-6f6f6f70h]",
    "XOR EAX, [ECX+ESI*8-6f6f6f70h]",
    "XOR EAX, [EDX+ESI*8-6f6f6f70h]",
    "XOR EAX, [EBX+ESI*8-6f6f6f70h]",
    "XOR EAX, [ESP+ESI*8-6f6f6f70h]",
    "XOR EAX, [EBP+ESI*8-6f6f6f70h]",
    "XOR EAX, [ESI+ESI*8-6f6f6f70h]",
    "XOR EAX, [EDI+ESI*8-6f6f6f70h]",
    "XOR EAX, [EAX+EDI*8-6f6f6f70h]",
    "XOR EAX, [ECX+EDI*8-6f6f6f70h]",
    "XOR EAX, [EDX+EDI*8-6f6f6f70h]",
    "XOR EAX, [EBX+EDI*8-6f6f6f70h]",
    "XOR EAX, [ESP+EDI*8-6f6f6f70h]",
    "XOR EAX, [EBP+EDI*8-6f6f6f70h]",
    "XOR EAX, [ESI+EDI*8-6f6f6f70h]",
    "XOR EAX, [EDI+EDI*8-6f6f6f70h]"
};

void disas_test1(BOOL x64, const unsigned char* code, disas_address adr, const char *must_be)
{
    strbuf t=STRBUF_INIT;
    Da* d=Da_Da(x64, (BYTE*)code, adr);
    size_t i;

    //printf (__FUNCTION__"() begin\n");
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

void disas_test2_2op(BOOL x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be, int value2_must_be)
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

void disas_test2_1op(BOOL x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be)
{
    strbuf t=STRBUF_INIT;
    Da* d=Da_Da(x64, (BYTE*)code, adr); 

    //printf (__FUNCTION__"() begin\n");
    assert (d!=NULL);
    Da_ToString(d, &t);
    if (_stricmp(t.buf, must_be)!=0 || d->_op[0]->value_width_in_bits!=value1_must_be)
    {
        printf ("disas_test1(FALSE, )->[%s]\n", t.buf);
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
    d=Da_Da(TRUE, (BYTE*)"\x74\x2F", 0x14000114c);
#else
    d=Da_Da(TRUE, (BYTE*)"\x74\x2F", 0x4000114c);
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

    //disas_test1(FALSE, (const unsigned char*)"", 0x123456, "");

#include "test32_2.h"

    for (modrm=0; modrm<=0xff; modrm++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=(char)modrm;

        disas_test1(FALSE, buf, 0x200, XOR_modrm_test[modrm]);
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=0x04; // modrm
        buf[2]=(char)SIB;

        disas_test1(FALSE, buf, 0x200, XOR_SIB_04_test[SIB]);
        //cout << disas_test1(FALSE, buf, 0x200) << endl;
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=0x44; // modrm
        buf[2]=(char)SIB;

        disas_test1(FALSE, buf, 0x200, XOR_SIB_44_test[SIB]);
        //cout << disas_test1(FALSE, buf, 0x200) << endl;
    };

    for (SIB=0; SIB<=0xff; SIB++)
    {
        memset (buf, 0x90, sizeof(buf));
        buf[0]=0x33;
        buf[1]=0x84; // modrm
        buf[2]=(char)SIB;

        disas_test1(FALSE, buf, 0x200, XOR_SIB_84_test[SIB]);
        //cout << disas_test1(FALSE, buf, 0x200) << endl;
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

    //disas_test1(FALSE, buf, 0x200, XOR_modrm_test[modrm]);

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
    
    disas_test1(TRUE, (const unsigned char*)"\x48\x05\x90\x90\x90\x90", 0x8855, "ADD RAX, 0FFFFFFFF90909090h"); // checked in IDA
    disas_test1(TRUE, (const unsigned char*)"\x48\x0F\x90\x90\x90\x90\x90\x90", 0x88ff, "SETO [RAX-6F6F6F70h]");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\xFC\xE0", 0x1234, "PADDB XMM4, XMM0");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\x76\xC0", 0x1234, "PCMPEQD XMM0, XMM0");
    disas_test1(FALSE, (const unsigned char*)"\x66\x8C\xC0", 0x1234, "MOV AX, ES");
    disas_test1(FALSE, (const unsigned char*)"\x66\x8C\xE0", 0x1234, "MOV AX, FS");
    disas_test1(FALSE, (const unsigned char*)"\x66\x8C\xE8", 0x1234, "MOV AX, GS");
    disas_test1(FALSE, (const unsigned char*)"\x66\xA1\xD0\x02\xFE\x7F", 0x1234, "MOV AX, [7FFE02D0h]");
    disas_test1(FALSE, (const unsigned char*)"\x0F\x03\xC1", 0x1234, "LSL EAX, ECX");
    disas_test1(FALSE, (const unsigned char*)"\x66\x87\x45\x06", 0x1234, "XCHG AX, [EBP+6]");
    disas_test1(FALSE, (const unsigned char*)"\x0F\x10\x02", 0x1234, "MOVUPS XMM0, [EDX]");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\x38\xDB\xC0", 0x1234, "AESIMC XMM0, XMM0");
    disas_test1(FALSE, (const unsigned char*)"\xF0\x66\x0F\xB1\x0B", 0x1234, "LOCK CMPXCHG [EBX], CX");
    disas_test1(FALSE, (const unsigned char*)"\xF3\x0F\x12\xC0", 0x1234, "MOVSLDUP XMM0, XMM0");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\x3A\xDF\xC0\x00", 0x1234, "AESKEYGENASSIST XMM0, XMM0, 0");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\x38\xDC\xC1", 0x1234, "AESENC XMM0, XMM1");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\x38\xDD\xC1", 0x1234, "AESENCLAST XMM0, XMM1");
    disas_test1(FALSE, (const unsigned char*)"\x66\x0F\x28\xC1", 0x1234, "MOVAPD XMM0, XMM1");
    disas_test1(FALSE, (const unsigned char*)"\x0F\x01\xD0", 0x1234, "XGETBV");
    disas_test1(TRUE, (const unsigned char*)"\x0F\x1F\x40\x00", 0x1234, "NOP [RAX]");
    disas_test1(FALSE, (const unsigned char*)"\xD9\xCA", 0x1234, "FXCH ST0, ST2");
    disas_test1(FALSE, (const unsigned char*)"\xDE\xF9", 0x1234, "FDIVP ST1, ST0");
    disas_test1(FALSE, (const unsigned char*)"\xDE\xF1", 0x1234, "FDIVRP ST1, ST0");

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
