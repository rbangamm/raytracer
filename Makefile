CXX = g++
CXXFLAGS = -std=c++14 -Wall -O3
EXEC = ray
OBJECTS = raytracer.o

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

raytracer.o: raytracer.cpp shapes.h

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC}
