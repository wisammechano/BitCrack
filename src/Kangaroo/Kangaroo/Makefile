NAME=Kangaroo
SRCS=$(wildcard *.cpp)
OPATH=$(OBJDIR)/$(NAME)
OBJS = $(addprefix $(OPATH)/, $(SRCS:.cpp=.o))

#CXXFLAGS += -m64 -mssse3 -Wno-unused-result -Wno-write-strings


all: $(OBJS)

$(OPATH)/%.o: %.cpp
	mkdir -p $(OPATH)
	$(CXX) $(CXXFLAGS) ${INCLUDE} -c $< -o $@

clean:
	rm -f $(OPATH)/*.o
