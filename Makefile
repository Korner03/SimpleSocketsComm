CC= g++
CPPFLAGS= -Wextra -Wall -g -std=c++11
HEADERS= whatsappServer.h whatsappProtocol.h ClientSocket.h whatsappClient.h
TAR_FILES= whatsappServer.h whatsappProtocol.h ClientSocket.h whatsappClient.h whatsappServer.cpp whatsappProtocol.cpp ClientSocket.cpp whatsappClient.cpp

all: whatsappServer whatsappClient

whatsappServer: whatsappServer.o whatsappProtocol.o ClientSocket.o
	$(CC) $^ -o $@

whatsappClient: whatsappClient.o whatsappProtocol.o
	$(CC) $^ -o $@

%.o: %.cpp $(HEADERS)
	$(CC) $(CPPFLAGS) -c $<

tar:
	tar -cvf ex5.tar $(TAR_FILES) README Makefile

clean:
	rm -rf *.o whatsappServer whatsappClient

.PHONY: clean all tar
