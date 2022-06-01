all:
	make clean
	cd build && cmake .. && make

example:
	make clean
	cd build && cmake .. && make && ./example

clean:
	@rm -rf build/*
