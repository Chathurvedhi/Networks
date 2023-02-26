#https://pythontic.com/modules/socket/udp-client-server-example
import socket
import sys

localIP     = "127.0.0.1"
localPort   = 20001
if(len(sys.argv) == 2):
    localPort = int(sys.argv[1])
bufferSize  = 1024

msgFromServer       = "Hello UDP Client"
bytesToSend         = str.encode(msgFromServer)

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))

print("UDP server up and listening")
print("***********************")

# Listen for incoming datagrams
while (True):
    bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
    message = bytesAddressPair[0].decode()
    address = bytesAddressPair[1]
    command = format(message)
    clientMsg = "Message Recieved: {}".format(message)
    clientIP  = "Client IP Address: {}".format(address)
    
    print(clientMsg)
    print(clientIP)

    cmd = command.split()
    if cmd[0] == "add":
        result = int(cmd[1]) + int(cmd[2])
        msgFromServer = str(result)
    elif cmd[0] == "mul":
        result = int(cmd[1]) * int(cmd[2])
        msgFromServer = str(result)
    elif cmd[0] == "mod":
        result = int(cmd[1]) % int(cmd[2])
        msgFromServer = str(result)
    elif cmd[0] == "hyp":
        result = (int(cmd[1]) ** 2 + int(cmd[2]) ** 2) ** 0.5
        result = int(result)
        msgFromServer = str(result)

    # Sending a reply to client
    UDPServerSocket.sendto(msgFromServer.encode(), address)
    print("-----------------------")

