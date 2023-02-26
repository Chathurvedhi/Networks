#https://pythontic.com/modules/socket/udp-client-server-example
import socket
import sys

localIP     = "127.0.0.1"
localPort   = 20001
if(len(sys.argv) == 2):
    localPort = int(sys.argv[1])
bufferSize  = 1024

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
    echoMsg = format(message)
    clientMsg = "Message from Client: {}".format(message)
    clientIP  = "Client IP Address: {}".format(address)
    
    print(clientIP)
    print(clientMsg)

    # Sending a reply to client
    UDPServerSocket.sendto(echoMsg.encode(), address)
    print("-----------------------")

