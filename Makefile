CXX := clang++
LDFLAGS := -L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system -lkfr_dft -lkfr_io
CXXFLAGS := -Wall -Werror -std=c++17 -I/usr/local/include -Iinclude/
SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp, obj/%.o, $(SRCS))

amp: $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(LDFLAGS) -o amp 

obj/%.o: src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	rm -rf *.exe
	rm -rf amp
	rm -rf obj/
	mkdir obj