all:
	make clean
	mkdir -p build && cd build && cmake .. && make

example:
	make clean
	mkdir -p build && cd build && cmake .. && make && ./example

clean:
	@rm -rf build/*
