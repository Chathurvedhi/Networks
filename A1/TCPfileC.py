#https://pythontic.com/modules/socket/send
import socket
import sys

serverName = 'localhost'
serverPort = 12001
if(len(sys.argv) == 2):
    serverPort = int(sys.argv[1])

clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

clientSocket.connect((serverName,serverPort))

sentence = input("Enter Filename and number of bytes: ")

fil = sentence.split()
file = fil[0]
file = file+"1"

clientSocket.send(sentence.encode())

modifiedSentence = clientSocket.recv(1024)

if(modifiedSentence.decode() == "SORRY!"):
    print("Server says file doesn't exist.")
    clientSocket.close()
else:
    f = open(file, "w")
    f.write(modifiedSentence.decode())
    f.close()

clientSocket.close()
