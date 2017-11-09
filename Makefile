single_cycle: simulator.o main.o
	g++ simulator.o main.o -o single_cycle
simulator.o: simulator.cpp
	g++ -c simulator.cpp
main.o: main.cpp
	g++ -c main.cpp
clean:
	rm -f single_cycle simulator.o main.o

