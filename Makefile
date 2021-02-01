IMGUI_DIR = ../imgui/ # replace this with where your local Dear ImGui repository is
IMGUI_SFML_DIR = ../imgui-sfml/ # replace this with where your local ImGui-SFML repository is

CXX := g++
LDFLAGS := -L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32
CXXFLAGS := -Wall -Werror -std=c++17 -I/usr/local/include -Iinclude/ -I$(IMGUI_DIR) -I$(IMGUI_SFML_DIR)
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