FLAGS = -g -Wall -O3

all : sender receiver

sender : SenderGBN.cpp
	g++ -o SenderGBN SenderGBN.cpp $(FLAGS)

receiver : ReceiverGBN.cpp
	g++ -o ReceiverGBN ReceiverGBN.cpp $(FLAGS)

clean :
	rm SenderGBN ReceiverGBN