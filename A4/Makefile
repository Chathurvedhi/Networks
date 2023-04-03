FLAGS = -g -O3

all : sender receiver

sender : SenderGBN.cpp
	g++ -o SenderGBN SenderGBN.cpp $(FLAGS)

receiver : RecieverGBN.cpp
	g++ -o RecieverGBN RecieverGBN.cpp $(FLAGS)

clean :
	rm SenderGBN RecieverGBN