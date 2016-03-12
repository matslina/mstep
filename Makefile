
SRC_LIB=$(wildcard lib/mstep/*.cpp)

all:	test ncurses

ncurses:	$(SRC_LIB) examples/ncurses.cpp
	g++ -g examples/ncurses.cpp $(SRC_LIB) -I lib/mstep/ -lcurses -lpthread -o ncurses

test:	test/test_displaywriter.cpp test/test_storagecontroller.cpp
	g++ test/test_displaywriter.cpp -I lib/mstep/ -o test_displaywriter
	g++ test/test_storagecontroller.cpp -I lib/mstep/ -o test_storagecontroller
	./test_displaywriter
	./test_storagecontroller
