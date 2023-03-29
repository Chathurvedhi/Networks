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
float packet_gen_rate = 10; 
int max_buffer_size = 10;
int packet_len = 10;
int window_size = 16;
float timeout = 100;                        // in ms

//Variables for GBN
int start_g = 0;
int end_g = window_size;
queue<string> packets;
int timeout_val = -1;
bool timeout_check = false;

//Mutex for synchronization
mutex m;

/*

void packetcreation()
{
    int i = 0;
    while(1)
    {
        // Random packet generation
        string packet = "";
        for(int j = 0; j < packet_len; j++)
        {
            packet += (char)(rand()%26 + 'a');
        }
        packet = to_string(i) + "|" + packet;
        m.lock();
        if(packets.size() == max_buffer_size)
            continue;
        packets.push(packet);
        m.unlock();
        i++;
        sleep(1/packet_gen_rate);
    }
}

void timeout_ack(int count)
{
    sleep(timeout/1000);
    m.lock();
    timeout_check = true;
    timeout_val = count;
    m.unlock();
}

void packetaccept(int sock, struct sockaddr_in &sendGBN, socklen_t &len, int count)
{
    char buffer[MAX_LINE] = {0};
    thread timeout = thread(timeout_ack, count);
    timeout.join();
    recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
    timeout.detach();
    cout<<"ACK "<<buffer<<" received"<<endl;
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
        int count = start_g;
        thread packet_threads[window_size];
        while(count << window_size)
        {
            if(timeout_check)
            {
                count = timeout_val;
                for(int i = 0; i < window_size; i++)
                {
                    packet_threads[i].detach();
                }
                break;
            }
            m.lock();
            string packet = packets.front();
            packets.pop();
            m.unlock();
            sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
            cout<<"Packet "<<count <<" sent"<<endl;
            packet_threads[count] = thread(packetaccept, sock, ref(sendGBN), ref(len), count);
            packet_threads[count].join();
            count++;
        }
        m.lock();
        if(timeout_check)
        {
            timeout_check = false;
            start_g = timeout_val;
            end_g = start_g + window_size;
            timeout_val = -1;
        }
        else
        {
            start_g = count;
            end_g = start_g + window_size;
        }
        m.unlock();
    }
    
}
*/

void packet_creation_temp()
{
    int i = 0;
    for(int i = 0; i < 10; i++)
    {
        string packet = "";
        for(int j = 0; j < packet_len; j++)
        {
            packet += (char)(rand()%26 + 'a');
        }
        packet = to_string(i) + "|" + packet;
        packets.push(packet);
    }
}

void packet_sender()
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
    while(packets.size())
    {
        string packet = packets.front();
        packets.pop();
        sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
        cout<<"Packet "<<packet<<" sent"<<endl;
        recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, &len);
        cout<<"ACK "<<buffer<<" received"<<endl;
    }   
}

int main()
{
    // Sender GBN as UDP Client
    // One thread for creating packets and storing them in a buffer
    // The other for sending packets and acknowledgements of ACKs

    srand(time(0));

    packet_creation_temp();
    thread packetsend_thread(packet_sender);

    packetsend_thread.join();
}

