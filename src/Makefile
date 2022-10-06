## ======================================================================
## partial Makefile provided to students
##

GTK_INCLUDE := `pkg-config --cflags gtk+-3.0`
GTK_LIBS := `pkg-config --libs gtk+-3.0`

.PHONY: clean new style feedback submit1 submit2 submit

CFLAGS += -Wall -pedantic -g

CPPFLAGS += -DALUEXT
CPPFLAGS += -DBLARGG
LDFLAGS += -L.
LDLIBS += -lcs212gbfinalext


# a bit more checks if you'd like to (uncomment)
# CFLAGS += -Wextra -Wfloat-equal -Wshadow                         \
# -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings \
# -Wconversion -Wunreachable-code

# uncomment if you want to add DEBUG flag
#CPPFLAGS += -DDEBUG

# ----------------------------------------------------------------------
# feel free to update/modifiy this part as you wish

# all those libs are required on Debian, feel free to adapt it to your box
#LDLIBS += -lcheck -lm -lrt -pthread -lsubunit
LDLIBS += -lm -lrt -pthread

all:: gbsimulator

TARGETS := 
CHECK_TARGETS := 
OBJS_NO_STATIC_TESTS =
OBJS_STATIC_TESTS = 
OBJS = $(OBJS_STATIC_TESTS) $(OBJS_NO_STATIC_TESTS)

# ----------------------------------------------------------------------

clean::
	-@/bin/rm -f *.o *~ $(CHECK_TARGETS) && rm gbsimulator

new: clean all

static-check:
	scan-build -analyze-headers --status-bugs -maxloop 64 make CC=clang new

style:
	astyle -n -o -A8 -xt0 *.[ch]

# all those libs are required on Debian, adapt to your box
$(CHECK_TARGETS): LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

check:: $(CHECK_TARGETS)
	$(foreach target,$(CHECK_TARGETS),./$(target) &&) true
	
# target to run tests
check:: all
	@if ls tests/*.*.sh 1> /dev/null 2>&1; then \
	  for file in tests/*.*.sh; do [ -x $$file ] || echo "Launching $$file"; ./$$file || exit 1; done; \
	fi

IMAGE=chappeli/feedback:latest
feedback:
	@docker pull $(IMAGE)
	@docker run -it --rm -v ${PWD}:/home/tester/done $(IMAGE)

SUBMIT_SCRIPT=../provided/submit.sh
submit1: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 1

submit2: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 2

submit:
	@printf 'what "make submit"??\nIt'\''s either "make submit1" or "make submit2"...\n'

libsid.so: sidlib.c
	$(CC) -fPIC -shared $(CPPFLAGS) $(CFLAGS) $(GTK_INCLUDE) $^ -o $@

libsid_demo.o: CFLAGS += $(GTK_INCLUDE)
libsid_demo: LDFLAGS += -L.
libsid_demo: LDLIBS += $(GTK_LIBS) -lsid
libsid_demo: libsid_demo.o libsid.so

alu.o: alu.c alu.h bit.h error.h
bit.o: bit.c bit.h
bit_vector.o: bit_vector.c bit_vector.h bit.h image.h error.h
bootrom.o: bootrom.c bootrom.h bus.h memory.h component.h gameboy.h cpu.h \
 alu.h bit.h cartridge.h timer.h error.h
bus.o: bus.c bus.h memory.h component.h error.h bit.h
cartridge.o: cartridge.c cartridge.h component.h memory.h bus.h error.h cpu-storage.h
component.o: component.c component.h memory.h error.h
cpu-alu.o: cpu-alu.c error.h bit.h alu.h cpu-alu.h opcode.h cpu.h bus.h \
 memory.h component.h cpu-storage.h cpu-registers.h alu_ext.h
cpu.o: cpu.c error.h opcode.h bit.h cpu.h alu.h bus.h memory.h \
 component.h cpu-alu.h cpu-registers.h cpu-storage.h util.h gameboy.h \
 cartridge.h timer.h
cpu-registers.o: cpu-registers.c cpu-registers.h cpu.h alu.h bit.h bus.h \
 memory.h component.h error.h
cpu-storage.o: cpu-storage.c error.h cpu-storage.h memory.h opcode.h \
 bit.h cpu.h alu.h bus.h component.h cpu-registers.h gameboy.h \
 cartridge.h timer.h util.h
error.o: error.c
gameboy.o: gameboy.c gameboy.h bus.h memory.h component.h cpu.h alu.h \
 bit.h cartridge.h timer.h error.h bootrom.h
gbsimulator.o: gbsimulator.c sidlib.h lcdc.h cpu.h alu.h bit.h bus.h \
 memory.h component.h image.h bit_vector.h error.h gameboy.h util.h \
 cpu-alu.h cpu-registers.h cpu-storage.h opcode.h timer.h cartridge.h bootrom.h
#gbsimulator: CPPFLAGS += -DTETRIS
gbsimulator: CFLAGS += $(GTK_INCLUDE)
gbsimulator: gbsimulator.o sidlib.o cpu.o alu.o bit.o bus.o \
 memory.o component.o image.o bit_vector.o error.o gameboy.o util.o\
 cpu-alu.o cpu-registers.o cpu-storage.o opcode.o timer.o cartridge.o bootrom.o
	gcc -g gbsimulator.o sidlib.o cpu.o alu.o bit.o bus.o \
	memory.o component.o image.o bit_vector.o error.o gameboy.o util.o cpu-alu.o \
	cpu-registers.o cpu-storage.o opcode.o timer.o cartridge.o bootrom.o \
	-o gbsimulator -lsid $(GTK_LIBS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS)

image.o: image.c error.h image.h bit_vector.h bit.h 
libsid_demo.o: libsid_demo.c sidlib.h

memory.o: memory.c memory.h error.h
opcode.o: opcode.c opcode.h bit.h
sidlib.o: CFLAGS += $(GTK_INCLUDE)
sidlib.o: sidlib.c sidlib.h 
timer.o: timer.c timer.h component.h memory.h bit.h cpu.h alu.h bus.h \
 error.h cpu-storage.h opcode.h
util.o: util.c util.h
