CC=gcc
OCTOTHORPE=../octothorpe
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE)
CFLAGS=-Wall -g
SOURCES=value.c x86_disas.c X86_register.c x86_tbl.c
TEST_SOURCES=x86_disasm_test_x64.c x86_disasm_tests.c
OUTDIR=$(MSYSTEM)_debug
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBRARY=$(OUTDIR)/x86_disasmd.a
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)/$(MSYSTEM)_debug/octothorped.a

all: $(OUTDIR) $(LIBRARY)($(OBJECTS)) $(OUTDIR)/x86_disasm_tests.exe

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIBRARY)
	$(RM) $(OUTDIR)/x86_disasm_test_x64.o
	$(RM) $(OUTDIR)/x86_disasm_tests.exe

-include $(OBJECTS:.o=.d)

$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

# test:

$(OUTDIR)/x86_disasm_tests.exe: $(OUTDIR)/x86_disasm_test_x64.o $(LIBRARY)
	$(CC) $(CFLAGS) $(CPPFLAGS) x86_disasm_tests.c $(OUTDIR)/x86_disasm_test_x64.o $(LIBRARY) $(OCTOTHORPE_LIBRARY) -o $@
