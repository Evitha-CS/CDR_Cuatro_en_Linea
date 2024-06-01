# Variables
CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lpthread

# Objetivos
all: servidor cliente

servidor: servidor_p.o
	$(CXX) $(CXXFLAGS) -o servidor servidor_p.o $(LDFLAGS)

cliente: cliente_p.o
	$(CXX) $(CXXFLAGS) -o cliente cliente_p.o $(LDFLAGS)

servidor.o: servidor_p.cpp
	$(CXX) $(CXXFLAGS) -c servidor_p.cpp

cliente.o: cliente_p.cpp
	$(CXX) $(CXXFLAGS) -c cliente_p.cpp

clean:
	rm -f *.o servidor cliente

.PHONY: all clean
