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
float packet_gen_rate = 2; 
int max_buffer_size = 10;
int packet_len = 10;
int window_size = 5;
float timeout = 4000;                        // in ms

//Variables for GBN
int start_g = 0;
int end_g = window_size;
queue<string> packets;
bool timeout_check = false;
int prev_LFT = -1;
int LFT = -1;
unordered_map<int, thread> window_threads;
unordered_map<int, bool> thread_kills;

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

void timeout_ack(int seq_no, int start_val)
{
    usleep(timeout*1000);
    m.lock();
    if(!thread_kills[seq_no + start_val])
    {
        cout<<"Timeout for packet "<<seq_no << "Start val " << start_val <<endl;
        timeout_check = true;
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
        cout<<"Received ack for packet "<<buffer<<endl;
        if(stoi(buffer) > LFT)
        {
            prev_LFT = LFT;
            LFT = stoi(buffer) - start_g;
            // Detach all threads till LFT
            for(int i = prev_LFT + 1; i <= LFT; i++)
            {
                thread_kills[i + start_g] = true;
                window_threads[i].detach();
                window_threads.erase(i);
            }
        }
        cout << "Finito ack" << buffer << endl;
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
        int w_trav = 0;
        while(w_trav < window_size)
        {
            m.lock();
            bool temp = packets.empty();
            m.unlock();
            if(temp) continue;
            m.lock();
            if(timeout_check)   //Deal with error
            {
                m.lock();
                w_trav = LFT + 1;
                timeout_check = false;
                for(int i = LFT + 1; i < window_size; i++)
                {
                    window_threads[i].detach();
                    window_threads.erase(i);
                }
                m.unlock();
                break;
            }
            cout<<"Window traversal"<<endl;
            string packet = packets.front();
            packets.pop();
            m.unlock();
            window_threads[w_trav] = thread(timeout_ack, w_trav, start_g);
            sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, sizeof(recvGBN));
            cout<<"Packet "<<packet<<" sent"<<endl;
            w_trav++;
        }
        cout<<"Window Complete Data" << endl;
        while(1)
        {
            m.lock();
            if(timeout_check)   //Deal with error
            {
                m.lock();
                w_trav = LFT + 1;
                timeout_check = false;
                for(int i = LFT + 1; i < window_size; i++)
                {
                    window_threads[i].detach();
                    window_threads.erase(i);
                }
                m.unlock();
                break;
            }
            if(LFT == window_size - 1)
            {
                LFT = -1;
                if(window_threads.size() == 0) cout << "All threads detached";
                cout<<"------------------------------------"<<endl;
                m.unlock();
                break;
            }
            m.unlock();
        }
        m.lock();
        start_g = start_g + w_trav;
        end_g = start_g + window_size;
        m.unlock();
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

