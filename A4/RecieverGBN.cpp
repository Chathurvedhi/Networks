#include<bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <random>
#include <ctime>

using namespace std;

#define port_no 20020
#define MAX_LINE 1024

int main()
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
    recvGBN.sin_addr.s_addr = INADDR_ANY;

    // Binding the socket with the server address
    int bind_status = bind(sock, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
    if(bind_status < 0)
    {
        cout<<"Bind failed"<<endl;
        exit(0);
    }

    int NFE = -1;                           //Next Frame Expected
    float random_drop_prob = 0.2;           //Probability of dropping a packet
    int max_packets = 10;                   //Maximum number of packets to be recieved
    int packets_recieved = 0;               //Number of packets recieved
    bool debug = false;                     //Debug mode

    auto start = chrono::high_resolution_clock::now(); //Start time

    while(max_packets > packets_recieved)
    {
        // Recieve the packet
        char buffer[MAX_LINE] = {0};
        socklen_t len = sizeof(sendGBN);
        int n = recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
        buffer[n] = '\0';

        // First 4 bytes of the packet is the sequence number 
        int seq_num = 0;
        for(int i = 0; i < 4; i++)
            seq_num = seq_num*10 + (buffer[i] - '0');

        // Random Packet Drop 
        int temp = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(temp < random_drop_prob)
            continue;

        // Drop the packet if it is not the next frame expected
        if(seq_num != NFE)
            continue;

        // If debug mode is on, print the recieved packet
        if(debug)
        {
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            int micro = duration.count();
            int milli = micro/1000;
            micro = micro%1000;
            cout<<"Seq No: "<<seq_num<< "  ";
            cout<<"Time: " << milli << ":" << micro << "  ";
            cout<<"Packet dropped: False "<<buffer<<endl;
        }

        // Send ACK
        string ack = to_string(seq_num);
        sendto(sock, ack.c_str(), ack.length(), MSG_CONFIRM, (const struct sockaddr *)&sendGBN, len);

        // Increment NFE and packets recieved
        NFE = (NFE + 1);
        packets_recieved++; 
    }
}