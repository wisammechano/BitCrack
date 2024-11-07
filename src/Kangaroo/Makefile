#---------------------------------------------------------------------
# Makefile for BSGS
#
# Author : Jean-Luc PONS


# Compiler and flags
CUR_DIR=$(shell pwd)
export OBJDIR     = $(CUR_DIR)/obj

CXXFLAGS += -m64 -mssse3 -Wno-unused-result -Wno-write-strings
LFLAGS = -lpthread
NVCCFLAGS += -maxrregcount=0 --compile
INCLUDE = -I.

ifeq ($(BUILD_CUDA),1)
CXXFLAGS += -DWITHGPU
LFLAGS   += -L$(CUDA_LIB) -lcudart
endif


# Object files
SECPK1_OBJS = $(OBJDIR)/SECPK1/Int.o $(OBJDIR)/SECPK1/IntGroup.o $(OBJDIR)/SECPK1/IntMod.o \
              $(OBJDIR)/SECPK1/Point.o $(OBJDIR)/SECPK1/Random.o $(OBJDIR)/SECPK1/SECP256K1.o

KANGAROO_OBJS = $(OBJDIR)/Kangaroo/Backup.o $(OBJDIR)/Kangaroo/Check.o $(OBJDIR)/Kangaroo/Kangaroo.o \
                 $(OBJDIR)/Kangaroo/main.o $(OBJDIR)/Kangaroo/Merge.o $(OBJDIR)/Kangaroo/Network.o \
                 $(OBJDIR)/Kangaroo/PartMerge.o $(OBJDIR)/Kangaroo/Thread.o
                

HASHTABLE_OBJS = $(OBJDIR)/HashTable/HashTable.o

TIMER_OBJS = $(OBJDIR)/Timer/Timer.o

GPU_OBJS = $(OBJDIR)/GPU/GPUEngine.o

ifeq ($(BUILD_CUDA), 1)
ALL_OBJS = $(SECPK1_OBJS) $(KANGAROO_OBJS) $(GPU_OBJS) $(TIMER_OBJS) $(HASHTABLE_OBJS)
else
ALL_OBJS = $(SECPK1_OBJS) $(KANGAROO_OBJS) $(TIMER_OBJS) $(HASHTABLE_OBJS)
endif
# Targets
all: driverquery $(OBJDIR) bsgs

$(OBJDIR):
	mkdir -p $(OBJDIR)

driverquery:
ifdef gpu
	. ./detect_cuda.sh
endif

bsgs: $(TIMER_OBJS) $(HASHTABLE_OBJS) $(SECPK1_OBJS) $(KANGAROO_OBJS) $(GPU_OBJS)
	@echo "Making Executable"
	$(CXX) $(ALL_OBJS) $(LFLAGS) -o kangaroo-256.bin
	mkdir -p $(BINDIR)
	cp kangaroo-256.bin $(BINDIR)/kangaroo

# Call subdirectory makefiles
$(TIMER_OBJS): | dir_timer
$(HASHTABLE_OBJS): | dir_hashtable
$(SECPK1_OBJS): | dir_sec_pk1
$(KANGAROO_OBJS): | dir_kangaroo
$(GPU_OBJS): | dir_gpu

dir_sec_pk1:
	make --directory SECPK1

dir_kangaroo:
	make --directory Kangaroo

dir_timer:
	make --directory Timer

dir_hashtable:
	make --directory HashTable

dir_gpu:
ifeq ($(BUILD_CUDA), 1)
	make --directory GPU
endif

clean:
	make --directory SECPK1 clean
	make --directory Kangaroo clean
	make --directory Timer clean
	make --directory HashTable clean

ifeq ($(BUILD_CUDA), 1)
	make --directory GPU clean
endif
	rm -f kangaroo-256
