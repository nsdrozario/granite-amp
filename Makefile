CXX := clang++

amp:
	$(CXX) amp.cpp -o amp -lsfml-graphics -lsfml-window -lsfml-system -lkfr_dft -lkfr_io

clean:
	rm -rf *.exe
	rm -rf amp