all: loopback stdpipes

stdpipes: Makefile *.cpp *.hpp
	g++ -g -O0 stdpipes.cpp my_interface.cpp -o stdpipes -Wall -Wextra -Wno-noexcept-type

loopback: Makefile *.cpp *.hpp
	g++ -g -O0 loopback.cpp my_interface.cpp -o loopback -Wall -Wextra -Wno-noexcept-type
