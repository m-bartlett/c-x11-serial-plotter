TARGET = main
SOURCE_FILES=$(TARGET).* *.hpp
# SOURCE_FILES=$(wildcard $(SOURCE_DIR)/*.c
CC=g++
C_FLAGS=-std=c++11
LD_FLAGS=-lX11 -lXt
OBJECTS=$(SOURCE_FILES:.cpp=.o)

#all: library.cpp main.cpp
# $@ evaluates to all
# $< evaluates to library.cpp
# $^ evaluates to library.cpp main.cpp

all: clean $(TARGET) run
	
$(TARGET):
	$(CC) $(SOURCE_FILES) $(C_FLAGS) $(INCLUDE_FLAGS) $(LIBRARY_FLAGS) $(LD_FLAGS) -o $(TARGET)

run:
	sh -c "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$$PWD/lib ./$(TARGET)" || true

libserial:
	bash -c "gcc send_receive.c $(C_FLAGS) $(INCLUDE_FLAGS) $(LIBRARY_FLAGS) -lserialport -o send_receive && LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$$PWD/lib ./send_receive /dev/ttyUSB0 "


.c.o.cpp:
	$(CC)  $< -o $@

clean:
	rm -rf $(SOURCE_DIR)/*.o $(TARGET) &>/dev/null


