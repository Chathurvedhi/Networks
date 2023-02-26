import os
import socket
import sys
pt_num = int(input("Enter port number: "))
pid = os.fork()
if pid == 0:
    localIP     = "127.0.0.1"
    localPort   = pt_num
    bufferSize  = 1024

    msgFromServer       = "Hello UDP Client"
    bytesToSend         = str.encode(msgFromServer)

    # Create a datagram socket
    UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Bind to address and ip
    UDPServerSocket.bind((localIP, localPort))

    print("UDP server up and listening")

    # Listen for incoming datagrams
    while (True):
        bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
        message = bytesAddressPair[0].decode()
        address = bytesAddressPair[1]
        clientMsg = "Message from Client: {}".format(message)
        clientIP  = "Client IP Address: {}".format(address)
        
        print(clientMsg)
        print(clientIP)

        # Sending a reply to client
        UDPServerSocket.sendto(msgFromServer.encode(), address)

msgFromClient       = input("Enter message to send:")
bytesToSend         = str.encode(msgFromClient)
serverAddressPort   = ("127.0.0.1", pt_num)
bufferSize          = 1024
# Create a UDP socket at client side
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
# Send to server using created UDP socket
UDPClientSocket.sendto(bytesToSend, serverAddressPort)
#Wait on recvfrom()
msgFromServer = UDPClientSocket.recvfrom(bufferSize)
#Wait completed
msg = "Message from Server: {}".format(msgFromServer[0].decode())
print(msg)
