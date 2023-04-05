// NAME: Chathurvedhi Talapaneni
// Roll Number: CS20B021
// Course: CS3205 Jan. 2023 semester
// Lab number: 4
// Date of submission: 05/04/2023
// I confirm that the source file is entirely written by me without
// resorting to any dishonest means.
// Website(s) that I used for basic socket programming code are:
// URL(s): 

#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <random>
#include <ctime>

using namespace std;

int port_no = 20020;
#define MAX_LINE 1024

// New system check pt 2

int main(int argc, char *argv[])
{
    srand(time(0));

    float random_drop_prob = 0.00001;       //Probability of dropping a packet
    int max_packets = 100;                  //Maximum number of packets to be received
    bool debug = false;                     //Debug mode

    for(int i = 1; i < argc; i++)           //Parsing the command line arguments
    {
        if(strcmp(argv[i], "-p") == 0)
        {
            port_no = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            debug = true;
        }
        else if(strcmp(argv[i], "-r") == 0)
        {
            random_drop_prob = atof(argv[i+1]);
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            max_packets = atoi(argv[i+1]);
        }
    }

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

    int NFE = 0;                            //Next Frame Expected
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
        int seq_num = (uint8_t) buffer[0];
        //cout<< seq_num << endl;
        string packet = "";
        for(int i = 1; i < n; i++)
        {
            packet += buffer[i];
        }
        
        // Drop the packet if it is not the next frame expected
        if(seq_num != NFE)
        {
            continue;
        }
        
        // Random Packet Drop 
        float temp_num = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(temp_num < random_drop_prob)             //If random number is less than the probability of dropping a packet
        {
            if(debug)                               //If debug mode is on, print the dropped packet information
            {
                auto end = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
                int micro = duration.count();
                int milli = micro/1000;
                micro = micro%1000;
                cout<<"Seq No: "<<seq_num<< "  ";
                cout<<"Time: " << milli << ":" << micro << "  ";
                cout<<"Packet dropped: True "<<endl;
            }
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
            cout<<"Packet dropped: False "<<endl;
        }

        // Send ACK
        string ack = to_string(seq_num);
        sendto(sock, ack.c_str(), ack.length(), MSG_CONFIRM, (const struct sockaddr *)&sendGBN, len);
        // Increment NFE and packets received
        NFE = (NFE + 1)%256;
        max_packets--;
        if(max_packets == 0)            //If all packets are received, send END signal
        {
            string ack = "END";
            sendto(sock, ack.c_str(), ack.length(), MSG_CONFIRM, (const struct sockaddr *)&sendGBN, len);
            cout<<"End signal Sent" << endl;
            break;
        }
    }
}