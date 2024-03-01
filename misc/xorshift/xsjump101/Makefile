GTEST_INCLUDE=/opt/gtest-1.8.1/include
GTEST_LIB=/opt/gtest-1.8.1/lib

NTL_INCLUDE=/opt/ntl-11.0.0/include
NTL_LIB=/opt/ntl-11.0.0/lib
GMP_LIB=/opt/gmp-6.1.2/lib

CXXFLAGS=-std=c++11 -g -O3

HS=runhaskell

all: xstests xstime

clean:
	-rm *.o *.exe

check: xstests
	./xstests

xs.o: xs.cc xs.h
	g++ $(CXXFLAGS) -c xs.cc -I$(NTL_INCLUDE)

xsT.o: xsT.cc
	g++ $(CXXFLAGS) -c xsT.cc

xsT.cc: xsTgen.hs
	$(HS) xsTgen.hs > xsT.cc

xstests: xstests.o xs.o xsT.o
	g++ $(CXXFLAGS) -o xstests xstests.o xs.o xsT.o -L$(NTL_LIB) -L$(GMP_LIB) -L$(GTEST_LIB) -lntl -lgmp -lgtest -lgtest_main

xstests.o: xstests.cc xs.h
	g++ $(CXXFLAGS:c++11=gnu++11) -c xstests.cc -I$(NTL_INCLUDE) -I$(GTEST_INCLUDE)

xstime: xstime.o xs.o xsT.o
	g++ $(CXXFLAGS) -o xstime xstime.o xs.o xsT.o -L$(NTL_LIB) -L$(GMP_LIB) -lntl -lgmp

xstime.o: xstime.cc xs.h
	g++ $(CXXFLAGS) -c xstime.cc -I$(NTL_INCLUDE)
