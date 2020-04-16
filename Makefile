CXX=gcc 
CFLAGS= -c
CXXFLAGS= -std=c++11 -Wall -Wextra 
OUTPUT= router
all: router

router: routing_table.o router.o 
	$(CXX) $(CXXFLAGS)  routing_table.o router.o  -o $(OUTPUT)
# main.o: main.cpp
# 	$(CXX) $(CFLAGS) main.cpp 
router.o: router.cpp 
	$(CXX) $(CFLAGS) router.cpp 
routing_table.o: routing_table.cpp
	$(CXX) $(CFLAGS) routing_table.cpp routing_table.h 

clean:
	rm -rf *.o  *.gch

distclean:
	rm -rf *.o *.gch $(OUTPUT)
	