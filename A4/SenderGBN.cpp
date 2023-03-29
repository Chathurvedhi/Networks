#include<bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define port_no 20020
#define MAX_LINE 1024
#define localhost "127.0.0.1"

int main()
{
    // Sender GBN as UDP Client
    // One thread for creating packets and storing them in a buffer
    // The other for sending packets and acknowledgements of ACKs

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        cout<<"Socket creation failed"<<endl;
        exit(0);
    }
    struct sockaddr_in recvGBN, sendGBN;
    memset(&recvGBN, 0, sizeof(recvGBN));
    
    // Filling server information
    recvGBN.sin_family = AF_INET;
    recvGBN.sin_port = htons(port_no);
    recvGBN.sin_addr.s_addr = inet_addr(localhost);

    vector<string> packets;
    packets.push_back("Hello");
    packets.push_back("World");
    packets.push_back("This");
    packets.push_back("is");
    packets.push_back("Sender");
    packets.push_back("GBN");

    int window_size = 3;
    int start = 0;
    int end = window_size;
    char buffer[MAX_LINE] = {0};

    for(int i = 0; i < packets.size(); i++)
    {
        string packet = to_string(i) + packets[i];
        sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
        cout<<"Packet "<<i<<" sent"<<endl;
        recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&recvGBN, (socklen_t *)sizeof(recvGBN));
        cout<<"ACK "<<i<<" received"<<endl;
    }

}