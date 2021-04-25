# uncomment and/or set to match processor 
# ** ONLY IF ** automatic determination of these compile flags is
# not working properly. uncommenting these variables will override 
# automatic determination of them
# To disable -march set: ARCH = NONE
# -mtune cannot be disabled but can be manually set here
#ARCH = native
#TUNE = native

CC = gcc
CFLAGS = -O2 -Wall -I/usr/include -I./include
OFILES = main.o comm.o

INSTALLSITEMAN1DIR = ${mandir}/man1
DISTMAN1DIR = docs/man
bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
exec_prefix = ${prefix}
mandir = ${datarootdir}/man
prefix = /usr/local

BIT := $(shell getconf LONG_BIT)
BIT64 = -Dbit$(BIT)
# bit32/64 compiler flag no longer used
#CFLAGS += $(BIT64)

GCCVER = $(shell expr `gcc -dumpversion` \>= 4.2)

GFLAGS1 = $(shell echo 'int main(){return 0;}' > /tmp/test-arch-tune.c && gcc -v -Q -march=native -O2 /tmp/test-arch-tune.c -o /tmp/test-arch-tune 2>&1 | sed -e 's/[\(|\)]//g' | grep march && rm /tmp/test-arch-tune*)
GNONATIVE = $(shell echo ${GFLAGS1} | grep -e "error" -e "bad value native" | awk '{for (i =1; i <= NF; i++) print $$i}' | grep -e "bad" -e "error")
GGMTOFF = $(shell echo '\#include <time.h>' > /tmp/test-arch-tune.c && echo 'int main(){long int g;struct tm t;g = t.tm_gmtoff;return (g);}' >> /tmp/test-arch-tune.c && gcc -march=native -O2 /tmp/test-arch-tune.c -o /tmp/test-arch-tune 2>&1 && rm /tmp/test-arch-tune*)
ifeq "$(strip $(GNONATIVE))" ""
GFLAGS = $(shell echo ${GFLAGS1} | awk '{for (i =1; i <= NF; i++) print $$i}' | grep -e "^-march=" -e "^-mtune=" | sort -u)
endif

ifdef GFLAGS
ifndef ARCH
GARCH1 = $(shell echo ${GFLAGS} | awk '{for (i =1; i <= NF; i++) print $$i}' | grep -e "^-march=" | sed -e 's/-march=//')
GARCH = $(shell echo ${GARCH1} | sed -e 's/ native//' -e 's/native //')
endif
ifndef TUNE
GTUNE1 = $(shell echo ${GFLAGS} | awk '{for (i =1; i <= NF; i++) print $$i}' | grep -e "^-mtune=" | sed -e 's/-mtune=//')
GTUNE = $(shell echo ${GTUNE1} | sed -e 's/ native//' -e 's/native //')
endif
endif

ifndef ARCH
ifdef GARCH
ARCH = ${GARCH}
else
ifeq ($(GCCVER),1)
ifeq "$(strip $(GNONATIVE))" ""
ARCH = native
endif
endif
endif
endif

ifndef TUNE
ifdef GTUNE
TUNE = ${GTUNE}
else
ifeq ($(GCCVER),1)
ifneq "$(strip $(GNONATIVE))" ""
TUNE = native
endif
endif
endif
endif

ifneq "$(strip $(GNONATIVE))" ""
ifeq ($(ARCH),native)
ARCH =
endif
ifeq ($(TUNE),native)
TUNE = 
endif
endif

ifneq "$(strip $(GGMTOFF))" ""
CFLAGS += -DNO_TM_GMTOFF
endif
ifneq "$(ARCH)" "NONE"
ifneq "$(strip $(ARCH))" ""
ARCH2 = $(shell echo ${ARCH} | awk '{for (i =1; i <= NF; i++) print "-march="$$i}')
CFLAGS += $(ARCH2)
endif
endif
ifneq "$(strip $(TUNE))" ""
TUNE2 = $(shell echo ${TUNE} | awk '{for (i =1; i <= NF; i++) print "-mtune="$$i}')
CFLAGS += $(TUNE2)
endif

TARGET = aurora

.PHONY: install

all:	$(TARGET)

$(TARGET): check $(OFILES)
	$(CC) $(OFILES) -o $(TARGET)
	chmod 4711 $(TARGET)

check:
ifeq "$(ARCH)" "NONE"
	@echo -n "Compiling for "
else
ifeq ($(BIT),64)
	@echo -n "Compiling for 64bit "
else
	@echo -n "Compiling for 32bit "
endif
endif
ifneq "$(strip $(GNONATIVE))" ""
	@echo -n "NoNative "
endif
ifdef ARCH
ifneq "$(ARCH)" "NONE"
	@echo -n "$(ARCH) "
endif
endif
ifdef TUNE
ifneq ($(ARCH),$(TUNE))
	@echo -n "$(TUNE) "
endif
endif
	@echo

main.o: Makefile main.c include/main.h include/comm.h include/names.h include/states.h
	$(CC) $(CFLAGS) -c main.c

comm.o: Makefile comm.c include/main.h include/comm.h include/names.h include/states.h
	$(CC) $(CFLAGS) -c comm.c

clean:
	rm -f $(TARGET) *.o

install: aurora
	install -m 4711 $(TARGET) $(bindir)
	install -m 0644 ${DISTMAN1DIR}/$(TARGET).1 ${INSTALLSITEMAN1DIR}

uninstall:
	rm -f $(bindir)/$(TARGET) ${INSTALLSITEMAN1DIR}/$(TARGET).1
	

