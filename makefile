all: bellman.cpp
	g++ -fopenmp -Wall -o bf bellman.cpp -lm

clean: 
	$(RM) bf