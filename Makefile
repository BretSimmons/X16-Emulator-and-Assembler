CC=gcc
CPP=g++
CFLAGS=-I. -g
CPPFLAGS=-I. -g -std=c++11
DEPS = x16.h bits.h control.h instruction.h trap.h io.h
OBJ = x16.o bits.o control.o instruction.o trap.o io.o decode.o
MAIN = main.o
ASOBJ = xas.o instruction.o bits.o
AS = xas
ODOBJ = xod.o bits.o instruction.o decode.o
OD = xod
TARGET = x16
TESTTARGET = test_x16
TESTOBJ = tests/test_main.o tests/test_bits.o tests/test_instruction.o \
	tests/test_control_add.o tests/test_control_and.o tests/test_control_br.o \
	tests/test_control_not.o tests/test_control_jmp.o \
	tests/test_control_jsr.o tests/test_control_ld.o \
	tests/test_control_ldi.o tests/test_control_ldr.o \
	tests/test_control_lea.o tests/test_control_st.o \
	tests/test_control_sti.o tests/test_control_str.o \
	tests/test_control_trap.o  \
	tests/test_xas.cpp tests/test_giza.cpp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CPPFLAGS)

x16: $(OBJ) $(MAIN)
	$(CC) -o $(TARGET) $^ $(CFLAGS)

clean:
	rm -rf *.o tests/*.o $(TARGET) $(TESTTARGET) $(AS) test_x16.dSYM xod
	-docker image rm seemongtan/build:latest

run: x16
	./$(TARGET)

$(AS): $(ASOBJ)
	$(CC) -o $(AS) $^ $(CFLAGS)

$(OD): $(ODOBJ)
	$(CC) -o $(OD) $^ $(CFLAGS)


$(TESTTARGET): $(TESTOBJ) $(OBJ)
	$(CPP) -o $(TESTTARGET) $(TESTOBJ) $(OBJ) $(CPPFLAGS)

test-build: $(TESTTARGET) $(AS) $(TARGET)

test: $(TESTTARGET) xas x16 giza.x16s
	./$(TESTTARGET) $(ARGS)

test-bits: $(TESTTARGET)
	./$(TESTTARGET) "[bits]"

test-instruction: $(TESTTARGET)
	./$(TESTTARGET) "[instruction]"

test-registerfile: $(TESTTARGET)
	./$(TESTTARGET) "[registerfile]"

test-alu: $(TESTTARGET)
	./$(TESTTARGET) "[alu]"

test-control-add: $(TESTTARGET)
	./$(TESTTARGET) "[control.add]"

test-control-and: $(TESTTARGET)
	./$(TESTTARGET) "[control.and]"

test-control-br: $(TESTTARGET)
	./$(TESTTARGET) "[control.br]"

test-control-jmp: $(TESTTARGET)
	./$(TESTTARGET) "[control.jmp]"

test-control-jsr: $(TESTTARGET)
	./$(TESTTARGET) "[control.jsr]"

test-control-ld: $(TESTTARGET)
	./$(TESTTARGET) "[control.ld]"

test-control-ldi: $(TESTTARGET)
	./$(TESTTARGET) "[control.ldi]"

test-control-ldr: $(TESTTARGET)
	./$(TESTTARGET) "[control.ldr]"

test-control-lea: $(TESTTARGET)
	./$(TESTTARGET) "[control.lea]"

test-control-not: $(TESTTARGET)
	./$(TESTTARGET) "[control.not]"

test-control-st: $(TESTTARGET)
	./$(TESTTARGET) "[control.st]"

test-control-sti: $(TESTTARGET)
	./$(TESTTARGET) "[control.sti]"

test-control-str: $(TESTTARGET)
	./$(TESTTARGET) "[control.str]"

test-control-trap: $(TESTTARGET)
	./$(TESTTARGET) "[control.trap]"

test-xas: $(TESTTARGET) xas x16
	./$(TESTTARGET) "[xas]"

test-memory-x16: $(TESTTARGET) x16
	./$(TESTTARGET) "Memory.x16"

test-memory-xas: $(TESTTARGET) x16 xas
	./$(TESTTARGET) "Memory.xas"

test-giza: $(TESTTARGET) x16 xas giza.x16s
	./xas giza.x16s
	./$(TESTTARGET) "Giza"

test-style:
	cpplint *.c

run-on-docker:
	docker run -it --rm --mount type=bind,src=.,dst=/app seemongtan/build:latest
