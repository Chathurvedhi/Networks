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

// New system check pt 2

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
    recvGBN.sin_addr.s_addr = inet_addr(localhost);

    // Binding the socket with the server address
    int bind_status = bind(sock, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
    if(bind_status < 0)
    {
        cout<<"Bind failed"<<endl;
        exit(0);
    }

    int NFE = 0;                            //Next Frame Expected
    float random_drop_prob = 0.2;           //Probability of dropping a packet
    int max_packets = 100;                  //Maximum number of packets to be received
    bool debug = false;                     //Debug mode
    socklen_t len;
    len = sizeof(sendGBN);

    auto start = chrono::high_resolution_clock::now(); //Start time

    for(int i_t = 0; i_t < max_packets; i_t++)
    {
        // Receive the packet
        char buffer[MAX_LINE] = {0};
        int n = recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
        buffer[n] = '\0';

        // Split buffer by '|' to get the sequence number and the packet
        string temp_buf(buffer);
        int pos = temp_buf.find("|");
        int seq_num = stoi(temp_buf.substr(0, pos));
        temp_buf = temp_buf.substr(pos+1, temp_buf.length());

        cout<<"Seq, NFE, temp_buf: "<<seq_num<<" "<<NFE<<" "<<temp_buf<<endl;
        // Random Packet Drop 
        
        /*
        float temp_num = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(temp_num < random_drop_prob)
        {
            cout << "Packet drop random" << endl;
            continue;
        }
        */

        // Drop the packet if it is not the next frame expected
        if(seq_num != NFE)
        {
            cout << "Packet drop NFE" << endl;
            continue;
        }

        // If debug mode is on, print the received packet
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
        cout<<"ACK sent: "<<ack<<endl;
       
        // Increment NFE and packets received
        NFE = (NFE + 1);
    }
}