CC=gcc

ifeq ($(BUILD),debug)
bsuffix=debug
CPPFLAGS_ADD=-D_DEBUG
else
bsuffix=release
CPPFLAGS_ADD=-O3
endif

OCTOTHORPE=../octothorpe
CPPFLAGS=-I$(OCTOTHORPE) $(CPPFLAGS_ADD)
CFLAGS=-Wall -g -std=gnu99
SOURCES=value.c x86_disas.c X86_register.c x86_tbl.c
TEST_SOURCES=x86_disasm_test_x64.c x86_disasm_tests.c
OUTDIR=$(MSYSTEM)_$(bsuffix)
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBRARY=$(OUTDIR)/x86_disasm.a
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)/$(MSYSTEM)_$(bsuffix)/octothorpe.a

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
