#https://pythontic.com/modules/socket/send
import socket 
import os
import sys

serverPort = 12001
if(len(sys.argv) == 2):
    serverPort = int(sys.argv[1])

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

serverSocket.bind(('',serverPort))

serverSocket.listen(2)

print("The server is ready to receive")
print("***********************")

while (True):
    (connectionSocket, clientAddress) = serverSocket.accept()
    print("Accepted a connection request from %s:%s"%(clientAddress[0], clientAddress[1]))
    sentence = connectionSocket.recv(1024)
    sentence = sentence.decode()
    fil = sentence.split()
    file = fil[0]
    num = int(fil[1])
    print("File requested: " + file + "; Number of bytes: %d"%num)
    #if file is not found, send error message
    if not os.path.isfile(file):
        connectionSocket.send("SORRY!".encode())
        connectionSocket.close()
        continue
    f = open(file, "r")
    #read last n bytes
    f.seek(0,2)
    size = f.tell()
    f.seek(max(size-num, 0), 0)
    lines = f.readlines()
    f.close()
    #send lines as a string with newlines
    sentence = ""
    for line in lines:
        sentence += line
        sentence += "\n"
    connectionSocket.send(sentence.encode())
    connectionSocket.close()
    print("-----------------------")
