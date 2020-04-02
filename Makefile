BIN	= VCKeccakTree
CC	= gcc
STRIP	= strip

#ECHO	=
#OFLAGS	= -g
ECHO	= @
OFLAGS	= -O3 -flto
CFLAGS	= $(OFLAGS) -Wall -fdata-sections -ffunction-sections -I/usr/local/cuda/include
LDFLAGS	= $(OFLAGS) -Wl,--gc-sections -Wl,-Map,$(BIN).map -L/usr/local/cuda/lib -L/usr/local/cuda-10.0/targets/aarch64-linux/lib
LDFLAGS	+= -lcuda -lcudart

SRC	= $(wildcard *.c)
SRCU	= $(wildcard *.cu)
OBJ	= $(SRC:.c=.o) $(SRCU:.cu=.o)
DEP	= $(SRC:.c=.d)

all: $(BIN)

-include $(DEP)

$(BIN): $(OBJ)
	@echo "$^ -> $@"
	$(ECHO)$(CC) -o $@ $^ $(LDFLAGS)
	$(ECHO)$(STRIP) $@

%.o: %.c
	@echo "$< -> $@"
	$(ECHO)$(CC) $(CFLAGS) -MMD -o $@ -c $<

%.o: %.cu
	@echo "$< -> $@"
	$(ECHO)nvcc -O3 -o $@ -c $<

.PHONY: clean install

clean:
	@rm -f *.o *.d *.map $(BIN)
