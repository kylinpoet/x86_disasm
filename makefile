OCTOTHORPE=../octothorpe/
CC=gcc
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE)
#CPPFLAGS=
CFLAGS=-Wall -g $(CPPFLAGS)
COMPILE_ONLY_CFLAGS=-c $(CFLAGS)
SOURCES=value.c x86_disas.c X86_register.c x86_tbl.c
TEST_SOURCES=x86_disasm_test_x64.c x86_disasm_tests.c
OBJECTS=$(SOURCES:.c=.o)
TEST_OBJECTS=$(TEST_SOURCES:.c=.o)
TEST_EXECS=$(TEST_SOURCES:.c=.exe)
LIBRARY=x86_disasmd.a
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a

all: $(LIBRARY) x86_disasm_tests.exe

clean:
	rm $(OBJECTS)
	rm $(TEST_OBJECTS)
	rm $(LIBRARY)
	rm x86_disasm_tests.exe

$(LIBRARY): $(OBJECTS)
	ar -mc $(LIBRARY) $(OBJECTS)

-include $(OBJECTS:.o=.d)

%.o: %.c
	$(CC) $(COMPILE_ONLY_CFLAGS) $*.c -o $*.o
	$(CC) -MM $(COMPILE_ONLY_CFLAGS) $*.c > $*.d

# test:

x86_disasm_tests.exe: x86_disasm_tests.o x86_disasm_test_x64.o $(LIBRARY)
	$(CC) $(CFLAGS) x86_disasm_tests.c x86_disasm_test_x64.o $(LIBRARY) $(OCTOTHORPE_LIBRARY) -o $@

