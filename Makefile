CC=gcc
CFLAGS=-Wall -Wextra -fPIC -g -O0 -I.
LFLAGS=-fPIC -shared -g -O0 -ldl
OBJS=obj/mini_open.o obj/hook_open.o  obj/hook.o obj/decode.o obj/itab.o obj/syn-att.o obj/syn-intel.o obj/syn.o obj/udis86.o obj/report_io.o obj/report_so_loaded.o
HEADERS=udis86.h libudis86/decode.h libudis86/extern.h libudis86/itab.h libudis86/syn.h libudis86/types.h libudis86/udint.h src/hook_int.h
RM=rm -rf

# in cmd of windows
ifeq ($(SHELL),sh.exe)
    RM := del /f/q
endif

all: hook_open.so mini_open.so

mini_open.so: mini_open.o
	$(CC) $(LFLAGS) -o mini_open.so obj/mini_open.o

mini_open.o: examples/mini_open.c
	$(CC) $(CFLAGS) -c -o obj/mini_open.o $<

hook_open.so: hook_open.o hook.o decode.o itab.o syn-att.o syn-intel.o syn.o udis86.o
	$(CC) $(LFLAGS) -o hook_open.so obj/hook_open.o obj/hook.o obj/decode.o obj/itab.o obj/syn-att.o obj/syn-intel.o obj/syn.o obj/udis86.o

hook_open.o: examples/hook_open.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/hook_open.o $<

hook.o: src/hook.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/hook.o $<

decode.o: libudis86/decode.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/decode.o $<

itab.o: libudis86/itab.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/itab.o $<

syn-att.o: libudis86/syn-att.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/syn-att.o $<

syn-intel.o: libudis86/syn-intel.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/syn-intel.o $<

syn.o: libudis86/syn.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/syn.o $<

udis86.o: libudis86/udis86.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o obj/udis86.o $<


clean:
	$(RM) obj/*.o mini_open.so hook_open.so

$(shell   mkdir -p obj)
