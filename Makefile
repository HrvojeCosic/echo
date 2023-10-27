#=================================================================================================================
#### VARIABLES
#=================================================================================================================
VPATH=     . src
INCDIRS=   . ./include
BINARY=    bin
BUILD_DIR= build
TEST_DIR=  test
CXX=       g++
OPT=       -O0
CPP_VER=   -std=c++20

#=================================================================================================================
#### FLAGS & FILES
#=================================================================================================================
DEPFLAGS=-MP -MD #Generate files including make rules for .h deps
FLAGS=-Wall -Wextra -Wno-missing-braces -g $(foreach dir,$(INCDIRS),-I$(dir)) $(OPT) $(DEPFLAGS)

CPPFILES= $(foreach dir, $(VPATH), $(wildcard $(dir)/*.cpp))
OFILES=   $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(notdir $(CPPFILES)))
HPPFILES= $(foreach dir, $(INCDIRS), $(wildcard $(dir)/*.hpp))
DEPFILES= $(patsubst %.c, %.d, $(CFILES)) $(patsubst %.cpp, %.d, $(CPPFILES))

-include $(DEPFILES)

#=================================================================================================================
#### BUILD
#=================================================================================================================
all: $(BINARY) format

$(BINARY): $(OFILES)
	$(CXX) $(FLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPP_VER) $(FLAGS) -c $< -o $@ 

$(BUILD_DIR): 
	mkdir $@

#=================================================================================================================
#### TEST
#=================================================================================================================
TESTS=       $(wildcard $(TEST_DIR)/*.cpp)
TESTBINS=    $(patsubst $(TEST_DIR)/%.cpp, $(TEST_DIR)/bin/%, $(TESTS))
TESTFLAGS=  -fsanitize=address -static-libasan
GTEST_LIBS= -lgtest -lgtest_main -lpthread

test: $(TEST_DIR)/bin $(TESTBINS) format
	@for test in $(TESTBINS) ; do ./$$test ; done

$(TEST_DIR)/bin/%: $(TEST_DIR)/%.cpp $(CPPFILES) $(HPPFILES)
	$(CXX) $(CPP_VER) $(FLAGS) -o $@ $< $(filter-out src/main.cpp,$(CPPFILES)) $(GTEST_LIBS) $(TESTFLAGS)

$(TEST_DIR)/bin:
	mkdir -p $@

#=================================================================================================================
#### LIBS
#=================================================================================================================
install_libgtest:
	sudo apt-get install libgtest-dev
	cd /usr/src/gtest; \
		sudo cmake CMakeLists.txt; \
		sudo make; \
		sudo cp ./lib/*.a /usr/lib;

#=================================================================================================================
#### MISC
#=================================================================================================================
clean:
	rm -rf $(BINARY) $(BUILD_DIR) $(OFILES) $(DEPFILES) $(TESTBINS)

format:
	$(shell find . -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i)

commit: format
	@if [ -z "$(m)" ]; then \
		echo "param 'm' is not provided. Run: make commit m='My comment'"; \
		exit 1; \
	fi
	git add .
	git commit -m "$(m)"
	git push