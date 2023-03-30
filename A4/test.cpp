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
int window_size = 10;
float timeout = 4000;                        // in ms

//Variables for GBN
int start_g = 0;
int end_g = window_size;
queue<string> packets;
int timeout_val = -1;
bool timeout_check = false;
int LFT = -1;

//Mutex for synchronization
mutex m;

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

void packet_creation()
{
    int pack_num = 0;
    while(1)
    {
        //cout<<"creating packets"<<packets.size()<<endl;
        if(packets.size() == max_buffer_size)
            continue;
        string packet = "";
        for(int j = 0; j < packet_len; j++)
        {
            packet += (char)(rand()%26 + 'a');
        }
        packet = to_string(pack_num) + "|" + packet;
        m.lock();
        packets.push(packet);
        m.unlock();
        pack_num++;
        usleep(1000000/packet_gen_rate);
    }
}

void timeout_ack(int seq_no)
{
    usleep(timeout*1000);
    m.lock();
    if(seq_no >= LFT + 1)
    {
        timeout_check = true;
        timeout_val = LFT + 1;
    }
    m.unlock();
}

void packet_ack(int sock, struct sockaddr_in &sendGBN, socklen_t &len)
{
    while(1)
    {
        char buffer[MAX_LINE] = {0};
        recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, (socklen_t *) &len);
        m.lock();
        if(stoi(buffer) > LFT)
        {
            LFT = stoi(buffer);
        }
        m.unlock();
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
    thread packet_ack_thread(packet_ack, sock, ref(sendGBN), ref(len));
    while(1)
    {   
        cout<<"Start window"<<endl;
        int w_trav = start_g;
        vector<thread> window_threads;
        while(w_trav < end_g)
        {
            m.lock();
            bool temp = packets.empty();
            m.unlock();
            if(temp) continue;
            if(timeout_check)
            {
                m.lock();
                w_trav = timeout_val;
                timeout_check = false;
                m.unlock();
                break;
            }
            cout<<"Window traversal"<<endl;
            m.lock();
            string packet = packets.front();
            packets.pop();
            m.unlock();
            sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
            cout<<"Packet "<<packet<<" sent"<<endl;
            window_threads.push_back(thread(timeout_ack, w_trav));
            w_trav++;
        }
        while(!window_threads.empty())
        {
            window_threads.back().detach();
            window_threads.pop_back();
        }
        cout<<"Finished window" << endl;
        start_g = w_trav;
        end_g = start_g + window_size;
    }   
    packet_ack_thread.join();
}

int main()
{
    // Sender GBN as UDP Client
    // One thread for creating packets and storing them in a buffer
    // The other for sending packets and acknowledgements of ACKs

    srand(time(0));

    //packet_creation_temp();
    thread packetcreation_thread(packet_creation);
    thread packetsend_thread(packet_sender);

    packetcreation_thread.join();
    packetsend_thread.join();
}

