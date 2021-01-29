CXX := clang++
INCLUDE := /usr/local/include
LIB := /usr/local/lib
CXXFLAGS := -std=c++17


clean:
	rm -rf *.exe
	rm -rf amp
	rm -rf obj/
	mkdir obj