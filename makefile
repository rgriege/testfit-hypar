CC      := gcc
CCFLAGS := -std=gnu99
CCFLAGS += -Wall -Werror -Wno-missing-braces -Wno-unused-local-typedefs -Wshadow
CCFLAGS += -DAPP_CODENAME=\"tfmp\"
CCFLAGS += -O3

HEADERS := testfit/ctyd.h testfit/debug.h testfit/mass_placement.h testfit/measure.h testfit/mp_utility.h testfit/opt.h testfit/opts.h testfit/poly.h testfit/types.h testfit/utility.h
SOURCES := testfit/mass_placement.c testfit/measure.c testfit/mp_utility.c testfit/opt.c testfit/opts.c testfit/poly.c testfit/utility.c tfmp.c

libtfmp.so: $(HEADERS) $(SOURCES)
	$(CC) -fPIC -shared -I. $(CCFLAGS) $(SOURCES) -o libtfmp.so -ldl -lm

tfmp: main.c libtfmp.so json/json.c json/json.h
	$(CC) -I. main.c json/json.c -o tfmp -L. -Wl,-rpath=. -ltfmp -lm

.PHONY: clean
clean:
	rm -f libtfmp.so
	rm -f tfmp
