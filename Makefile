CXX=g++ 
CFLAGS= -c
CXXFLAGS= -std=c++11 -Wall -Wextra
OUTPUT= router
all: router

router: routing_table.o router.o socket_funcs.o
	$(CXX) $(CXXFLAGS) socket_funcs.o routing_table.o router.o -o $(OUTPUT)
router.o: router.cpp 
	$(CXX) $(CFLAGS) router.cpp 
routing_table.o: routing_table.cpp
	$(CXX) $(CFLAGS) routing_table.cpp routing_table.h 
socket_funcs.o: socket_funcs.cpp
	$(CXX) $(CFLAGS) socket_funcs.cpp socket_funcs.h
clean:
	rm -rf *.o  *.gch

distclean:
	rm -rf *.o *.gch $(OUTPUT)
	