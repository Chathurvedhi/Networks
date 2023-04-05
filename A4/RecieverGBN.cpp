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
    float random_drop_prob = 0;           //Probability of dropping a packet
    int max_packets = 100;                  //Maximum number of packets to be received
    bool debug = true;                     //Debug mode
    socklen_t len;
    len = sizeof(sendGBN);

    auto start = chrono::high_resolution_clock::now(); //Start time

    int drop_num = 4;
    while(1)
    {
        // Receive the packet
        char buffer[MAX_LINE] = {0};
        int n = recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
        buffer[n] = '\0';

        // First byte is the sequence number rest is packet
        int seq_num = buffer[0];
        string packet = "";
        for(int i = 1; i < n; i++)
        {
            packet += buffer[i];
        }
        
        // Drop the packet if it is not the next frame expected
        if(seq_num != NFE)
        {
            cout << seq_num << " ";
            continue;
        }
        
        // Random Packet Drop 
        float temp_num = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(temp_num < random_drop_prob)
        {
            cout << "Packet drop random " << seq_num << endl;
            cout << "NFE drops :";
            continue;
        }

        //cout<<"Seq packet: "<<seq_num<<" "<<NFE<<" "<<packet<<endl;


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
            cout<<"Packet dropped: False "<<endl;
        }

        // Send ACK
        string ack = to_string(seq_num);
        sendto(sock, ack.c_str(), ack.length(), MSG_CONFIRM, (const struct sockaddr *)&sendGBN, len);
        //cout<<"ACK sent: "<<ack<<endl;
       
        // Increment NFE and packets received
        NFE = (NFE + 1);
        max_packets--;
        if(max_packets == 0)
        {
            string ack = "END";
            sendto(sock, ack.c_str(), ack.length(), MSG_CONFIRM, (const struct sockaddr *)&sendGBN, len);
            cout<<"End signal Sent" << endl;
            break;
        }
    }
}