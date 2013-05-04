OCTOTHORPE=../octothorpe/
CC=gcc
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE)
CFLAGS=-Wall -g
SOURCES=value.c x86_disas.c X86_register.c x86_tbl.c
TEST_SOURCES=x86_disasm_test_x64.c x86_disasm_tests.c
OBJECTS=$(SOURCES:.c=.o)
LIBRARY=x86_disasmd.a
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a

all: $(LIBRARY)($(OBJECTS)) x86_disasm_tests.exe

clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIBRARY)
	$(RM) x86_disasm_tests.exe

-include $(OBJECTS:.o=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

# test:

x86_disasm_tests.exe: x86_disasm_tests.o x86_disasm_test_x64.o $(LIBRARY)
	$(CC) $(CFLAGS) $(CPPFLAGS) x86_disasm_tests.c x86_disasm_test_x64.o $(LIBRARY) $(OCTOTHORPE_LIBRARY) -o $@
