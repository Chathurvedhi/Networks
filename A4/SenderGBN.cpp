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
#define ip_val "192.168.1.5"

// Constant parameters for the GBN protocol
float gen_rate = 2;
int buffer_size = 10;
int packet_len = 10;
int window_size = 10;
float timeout = 4000;
bool debug = true;

mutex m;

// Variables for the GBN protocol
queue<string> packet_buffer;
vector<string> packet_window;
int start = 0;
bool timeout_check = false;
int prev_LFT = -1;
int LFT = -1;
thread timeout_thread;
unordered_map<int, bool> thread_kills;
unordered_map<int, int> transmit_count;
// map for RTT times
unordered_map<int, long> RTT_start;
int win_trav = 0;
int window_count = 0;
long RTT_avg = 0;
int trans_count = 0;
int ack_count = 0;

void exit_function()
{
    cout << "Packet_gen_rate : " << gen_rate << endl;
    cout << "Packet_len : " << packet_len << endl;
    cout << "Average RTT : " << RTT_avg << endl;
    cout << "Retransmission Ratio : " << float(trans_count) / ack_count << endl;
    exit(0);
}

void packet_generator()
{
    int seq_no = 0;
    while (1)
    {
        if (packet_buffer.size() == buffer_size)
            continue;
        string packet = "";
        for (int i = 0; i < packet_len; i++)
        {
            packet += (char)(rand() % 26 + 'a');
        }
        char temp_seq_no = char(seq_no);
        packet = temp_seq_no + packet;
        m.lock();
        packet_buffer.push(packet);
        m.unlock();
        seq_no = (seq_no + 1) % 256;
        usleep(1000000 / gen_rate);
    }
}


void timeout_ack(int seq_no, int start_val, int win)
{
    usleep(timeout * 1000);
    m.lock();
    if(win != window_count)
    {
        m.unlock();
        return;
    }
    if(thread_kills[seq_no + start_val])
    {
        m.unlock();
        return;
    }
    cout << "Timeout for packet " << seq_no + start_val << " " << win << endl;
    window_count++;
    timeout_check = true;
    // Remove packets from 0 to LFT from packet_window
    for(int i = 0; i <= LFT; i++)
    {
        packet_window.erase(packet_window.begin());
    }
    m.unlock();
}


void packet_sender(int sock, struct sockaddr_in &recvGBN, socklen_t &len)
{
    while(1)
    {
        win_trav = 0;
        while(win_trav < window_size)
        {   
            if(timeout_check) break;
            m.lock();
            bool temp = packet_buffer.empty();
            m.unlock();
            if(temp) continue;
            m.lock();
            string packet;
            if(win_trav < packet_window.size())
            {   
                packet = packet_window[win_trav];
            }
            else
            {
                packet = packet_buffer.front();
                packet_buffer.pop();
                packet_window.push_back(packet);
            }
            sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, len);
            trans_count++;
            auto temp_time = chrono::high_resolution_clock::now();
            //Time in microseconds
            long time = chrono::duration_cast<chrono::microseconds>(temp_time.time_since_epoch()).count();
            RTT_start[win_trav + start] = time;
            transmit_count[win_trav + start]++;
            if(transmit_count[win_trav + start] >= 5)
            {
                cout << "Max Retransmit" << endl;
                exit_function();
            }
            timeout_thread = thread(timeout_ack, win_trav, start, window_count);
            timeout_thread.detach();
            m.unlock();
            string data = packet.substr(1, packet.length() - 1);
            cout<<"Packet sent "<< int(packet[0]) << " " << data << endl;
            win_trav++;
        }
        while(1)
        {
            m.lock();
            if(timeout_check)
            {
                timeout_check = false;
                m.unlock();
                break;
            }
            if(LFT == window_size - 1)
            {
                packet_window.clear();
                cout<<"--------------------------" << endl;
                window_count++;
                m.unlock();
                break;
            }
            m.unlock();
        }
        m.lock();
        start = start + LFT + 1;
        LFT = -1;
        prev_LFT = -1;
        m.unlock();
    }
}

void ack_receiver(int sock, struct sockaddr_in &sendGBN, socklen_t &len, long time_micro)
{
    while(1)
    {
        char buffer[MAX_LINE] = {0};
        recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, (socklen_t *) &len);
        m.lock();
        if(buffer[0] == 'E')
        {
            cout << "END msg recieved" << endl;
            exit_function();
        }
        auto temp_time = chrono::high_resolution_clock::now();
        //Time in microseconds
        long time = chrono::duration_cast<chrono::microseconds>(temp_time.time_since_epoch()).count();
        long RTT = time - RTT_start[stoi(buffer)];
        RTT_avg = RTT_avg * stoi(buffer) + RTT;
        RTT_avg = RTT_avg / (stoi(buffer) + 1);
        RTT_start[stoi(buffer)] -= time_micro; 
        if(debug)
        {
            cout << "Seq #: " << buffer << " Time Generated: " << RTT_start[stoi(buffer)]/1000 << ":" << RTT_start[stoi(buffer)]%1000 << " RTT: " << RTT << " Number of Retransmissions: " << transmit_count[stoi(buffer)] << endl;
        }
        ack_count = max(ack_count, stoi(buffer) + 1);
        if(stoi(buffer) > LFT)
        {
            prev_LFT = LFT;
            LFT = stoi(buffer) - start;
            for(int i = prev_LFT + 1; i <= LFT; i++)
            {
                thread_kills[i + start] = true;
            }
        }
        m.unlock();
    }
}

void GBN_connect()
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
    recvGBN.sin_addr.s_addr = inet_addr(ip_val);

    char buffer[MAX_LINE] = {0};
    socklen_t len_ack;
    socklen_t len_packet = sizeof(recvGBN);

    auto start_time = chrono::high_resolution_clock::now();
    long time_micro = chrono::duration_cast<chrono::microseconds>(start_time.time_since_epoch()).count();

    thread packet_send_thread(packet_sender, sock, ref(recvGBN), ref(len_packet));
    thread ack_recv_thread(ack_receiver, sock, ref(sendGBN), ref(len_ack), time_micro);

    packet_send_thread.join();
    ack_recv_thread.join();
}

int main()
{
    srand(time(0));


    thread packet_gen_thread(packet_generator);
    thread GBN_connect_thread(GBN_connect);

    packet_gen_thread.join();
    GBN_connect_thread.join();
}