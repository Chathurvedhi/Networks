#https://pythontic.com/modules/socket/udp-client-server-example
import socket
import sys

if len(sys.argv) ==1:
    port_num = 20001
else:
    port_num = int(sys.argv[1])

serverAddressPort   = ("127.0.0.1", port_num)
bufferSize          = 1024

while(True):
    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    msg = input("Enter message: ")
    if msg == "exit":
        break
    bytesToSend = str.encode(msg)
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    msg = "Message from Server: {}".format(msgFromServer[0].decode())
    print("-----------------------")
