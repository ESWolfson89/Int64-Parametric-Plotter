comp=g++
flags=-std=c++11
FLCF=`fltk-config --cxxflags`
FLLF=`fltk-config --ldflags`

primarygui: main.o
	$(comp) $(flags) main.o $(FLLF) -o primarygui
main.o: main.cpp
	$(comp) $(flags) $(FLCF) -c main.cpp $(FLLF) -o main.o

.PHONY: clean

clean:
	rm primarygui main.o
