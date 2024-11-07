NAME=GPU
OPATH=$(OBJDIR)/$(NAME)

all: $(OPATH)/GPUEngine.o

ifdef DEBUG
$(OPATH)/GPUEngine.o: GPUEngine.cu
	mkdir -p $(OPATH)
	$(NVCC) $(NVCCFLAGS) --compiler-options -fPIC -ccbin $(CXX) -m64 -g -I$(CUDA_INCLUDE) -o $(OPATH)/GPUEngine.o -c GPUEngine.cu
else
$(OPATH)/GPUEngine.o: GPUEngine.cu
	mkdir -p $(OPATH)
	$(NVCC) $(NVCCFLAGS) --compiler-options -fPIC -ccbin $(CXX) -m64 -O2 -I$(CUDA_INCLUDE) -o $(OPATH)/GPUEngine.o -c GPUEngine.cu
endif

clean:
	rm -f $(OPATH)/*.o
