TARGET := runsstmac
SRC := $(shell ls *.cc) 
DUMPI :=  -L/usr/sstmacro/dumpi/lib -ldumpi

CXX :=      sst++
CC :=       sstcc
CXXFLAGS := 
CPPFLAGS := 
LIBDIR :=   
PREFIX :=  install
LDFLAGS :=  -Wl,-rpath,$(PREFIX)/lib 


OBJ := $(SRC:.cc=.o) 
OBJ := $(OBJ:.cpp=.o)
OBJ := $(OBJ:.c=.o)

.PHONY: clean install 

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CXX) -o $@ $+ $(LDFLAGS) $(LIBS) 

%.o: %.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(TARGET) $(OBJ) 

install: $(TARGET)
	cp $< $(PREFIX)/bin

