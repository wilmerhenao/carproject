
CC=g++

car: car.cpp
	$(CC) -o car car.cpp -O2 -lglut -lGL -lGLU
