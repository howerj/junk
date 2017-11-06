CXXFLAGS=-std=c++11
TARGET=logic
OBJECTS:=main.o variant.o

all: ${TARGET}

logic: ${OBJECTS}
	${CXX} ${CXXFLAGS} $^ -o $@

clean:
	rm -vf *.o ${TARGET}
