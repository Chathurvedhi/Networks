#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <mutex>

using namespace std;

#define port_no 20020
#define MAX_LINE 1024
#define localhost "127.0.0.1"

string packet;
int packet_len = 10;

void packet_generator()
{
    packet = "";
    for(int j = 0; j < packet_len; j++)
    {
        packet += (char)(rand()%26 + 'a');
    }
    int seq_num = 0;
    char seq_enc = (char)seq_num;
    packet = seq_enc + packet;
}

void connect()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        cout<<"Socket creation failed"<<endl;
        exit(0);
    }
    struct sockaddr_in recvGBN, sendGBN;
    memset(&recvGBN, 0, sizeof(recvGBN));   //Receiver Server

    // Filling server information
    recvGBN.sin_family = AF_INET;
    recvGBN.sin_port = htons(port_no);
    recvGBN.sin_addr.s_addr = inet_addr(localhost);

    char buffer[MAX_LINE] = {0};
    socklen_t len;
    packet_generator();
    sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
    cout<<"Packet "<<packet<<" sent"<<endl;
    recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
    cout<<"Ack:" << int(buffer[0]) << " received"<<endl;
}

int main()
{
    srand(time(0));

    connect();
}