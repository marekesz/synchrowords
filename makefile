all: main

format:
	find synchrolib/ -iname "*.hpp" -o -iname "*.cpp" | xargs clang-format -i *.cpp *.hpp

main:
	$(MAKE) -B -f makefile_main

clean:
	rm -rf build/synchrolib*

.PHONY: all main format clean
