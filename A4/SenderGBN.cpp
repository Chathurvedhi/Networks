#include<bits/stdc++.h>
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

//Constant variables for GBN
float packet_gen_rate = 1; 
int max_buffer_size = 10;
int packet_len = 10;
int window_size = 3;

//Variables for GBN
int start_g = 0;
int end_g = window_size;
queue<string> packets;

//Mutex for synchronization
mutex m;

void packetcreation()
{
    int i = 0;
    while(1)
    {
        if(packets.size() == max_buffer_size)
            continue;
        // Random packet generation
        string packet = "";
        for(int j = 0; j < packet_len; j++)
        {
            packet += (char)(rand()%26 + 'a');
        }
        packet = to_string(i) + "|" + packet;
        m.lock();
        packets.push(packet);
        m.unlock();
        i++;
        sleep(1/packet_gen_rate);
    }
}

void packetaccept(int sock, struct sockaddr_in &sendGBN, socklen_t &len)
{
    char buffer[MAX_LINE] = {0};
    recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
    cout<<"ACK "<<buffer<<" received"<<endl;
    m.lock();
    start_g = stoi(buffer) + 1;
    end_g = start_g + window_size;
    m.unlock();
}

void packetsend()
{
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

    char buffer[MAX_LINE] = {0};
    socklen_t len;
    
    while(1)
    {
        if(packets.size() == 0)
            continue;
        for(int i = start_g; i < end_g; i++)
        {
            m.lock();
            string packet = packets.front();
            packets.pop();
            m.unlock();
            sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
            cout<<"Packet "<<i<<" sent"<<endl;
            thread packetaccept_thread(packetaccept, sock, ref(sendGBN), ref(len));
        }
    }
    
}

int main()
{
    // Sender GBN as UDP Client
    // One thread for creating packets and storing them in a buffer
    // The other for sending packets and acknowledgements of ACKs

    srand(time(0));

    
    thread packetcreation_thread(packetcreation);
    thread packetsend_thread(packetsend);

    packetcreation_thread.join();
    packetsend_thread.join();
}

