BIN	= VCKeccakTree
CC	= gcc
STRIP	= strip

#ECHO	=
#OFLAGS	= -g
ECHO	= @
OFLAGS	= -O3 -flto
CFLAGS	= $(OFLAGS) -Wall -fdata-sections -ffunction-sections -I/usr/local/cuda/include
CUFLAGS = \
	-O3 \
	-gencode arch=compute_35,code=sm_35 \
	-gencode arch=compute_50,code=sm_50 \
	-gencode arch=compute_52,code=sm_52 \
	-gencode arch=compute_53,code=sm_53 \
	-gencode arch=compute_60,code=sm_60 \
	-gencode arch=compute_61,code=sm_61 \
	-gencode arch=compute_62,code=sm_62 \
	-gencode arch=compute_70,code=sm_70 \
	-gencode arch=compute_75,code=sm_75 \
	--ptxas-options=-v
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
	$(ECHO)nvcc $(CUFLAGS) -o $@ -c $<

.PHONY: clean install

clean:
	@rm -f *.o *.d *.map $(BIN)
