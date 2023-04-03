#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <random>
#include <ctime>

using namespace std;

#define port_no 20020
#define MAX_LINE 1024
#define localhost "127.0.0.1"

void connect()
{
    srand(time(0));
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        cout<<"Socket creation failed"<<endl;
        exit(0);
    }
    struct sockaddr_in recvGBN, sendGBN;
    memset(&recvGBN, 0, sizeof(recvGBN));   //Receiver Server
    memset(&sendGBN, 0, sizeof(sendGBN));   //Sender Client

    // Filling server information
    recvGBN.sin_family = AF_INET;
    recvGBN.sin_port = htons(port_no);
    recvGBN.sin_addr.s_addr = inet_addr(localhost);

    // Binding the socket with the server address
    int bind_status = bind(sock, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
    if(bind_status < 0)
    {
        cout<<"Bind failed"<<endl;
        exit(0);
    }

    char buffer[MAX_LINE] = {0};
    socklen_t len;
    len = sizeof(sendGBN);
    recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);

    int seq_num = -1;

    seq_num = int(buffer[0]);
    cout<<"seq_num: "<<seq_num<<endl;
    //Seperate the packet
    string packet = "";
    for(int i = 1; i < MAX_LINE; i++)
    {
        if(buffer[i] == '\0')
            break;
        packet += buffer[i];
    }
    cout<<"Packet: "<<packet<<endl;

    //Send Ack
    char ack = (char)seq_num;
    sendto(sock, (const char *)&ack, 1, MSG_CONFIRM, (const struct sockaddr *)&sendGBN, len);\
    cout<<"Ack sent"<<endl;
}

int main()
{
    connect();
}