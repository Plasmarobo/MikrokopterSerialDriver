BIN_DIR=bin
BIN_NAME=koptercom
CFILES=main.cpp raw_channel.cpp serial_frame.cpp kopter_control.cpp dummy.cpp
OFILES=raw_channel.o serial_frame.o kopter_control.o
CFLAGS=-c -g -Wall  
LFLAGS=


all: clean build link link_dummy fix_permission

clean:
	mkdir -p $(BIN_DIR)
	rm -rf *.o
	rm -rf $(BIN_DIR)/*

build: $(CFILES)
	g++ $(CFLAGS) $(CFILES)
 
link: $(OFILES)
	g++ $(LFLAGS) main.o $(OFILES) -o $(BIN_DIR)/$(BIN_NAME)

fix_permission: $(BIN_DIR)/$(BIN_NAME)
	chmod +x $(BIN_DIR)/dummy
	chmod +x $(BIN_DIR)/$(BIN_NAME)

link_dummy: $(OFILES) 
	g++ $(LFLAGS) dummy.o $(OFILES) -o $(BIN_DIR)/dummy
