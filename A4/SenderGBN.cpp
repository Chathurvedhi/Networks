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
#include <thread>
#include <mutex>

using namespace std;

#define MAX_LINE 1024

// Constant parameters for the GBN protocol
float gen_rate = 50;
int buffer_size = 10;
int packet_len = 512;
int window_size = 3;
float timeout = 100000;
bool debug = false;
int port_no = 20020;
int max_packets = 100;
string ip_val =  "127.0.0.1";

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
bool empty_window = false;

void exit_function()            // Called when Sender is terminated
{
    cout << "Packet_gen_rate : " << gen_rate << endl;
    cout << "Packet_len : " << packet_len << endl;
    cout << "Average RTT : " << RTT_avg << endl;
    cout << "Retransmission Ratio : " << float(trans_count) / ack_count << endl;
    exit(0);
}

void packet_generator()     // Thread to generate packets
{
    int seq_no = 0;
    while (1)
    {
        if (packet_buffer.size() == buffer_size)    // If buffer is full, wait
            continue;
        string packet = "";
        for (int i = 0; i < packet_len; i++)
        {
            packet += (char)(rand() % 26 + 'a');
        }
        char temp_seq_no = char(seq_no);
        packet = temp_seq_no + packet;              // Add sequence number to the packet
        m.lock();
        packet_buffer.push(packet);
        m.unlock();
        seq_no ++;
        if(seq_no == 256)                           // For remapping sequence numbers to start from 0
        {
            seq_no = 0;
            for(int i = 0; i < 256 - window_size; i++)      //Cleaning the variables for new remapping
            {
                empty_window = true;
                thread_kills[i] = false;
                transmit_count[i] = 0;
                RTT_start[i] = 0;
            }
        }
        usleep(1000000 / gen_rate);
    }
}


void timeout_ack(int seq_no, int start_val, int win)
{
    usleep(timeout);                            // Wait for timeout for individual packet
    m.lock();
    if(win != window_count)                     // Wrong window, timeout pointless
    {
        m.unlock();
        return;
    }
    if(thread_kills[(seq_no + start_val)%256])  // Packet recived
    {
        m.unlock();
        return;
    }
    window_count++;                             // New window to stop all previous window timers
    timeout_check = true;
    for(int i = 0; i <= LFT; i++)               // Clean window of packets
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
        while(win_trav < window_size)           // Each window
        {   
            if(timeout_check) break;            // Timeout occured
            m.lock();
            bool temp = packet_buffer.empty();
            m.unlock();
            if(temp) continue;
            m.lock();
            string packet;
            if(win_trav < packet_window.size()) // Retransmitting packets
            {   
                packet = packet_window[win_trav];
            }
            else                                // New packets
            {   
                packet = packet_buffer.front();
                packet_buffer.pop();
                packet_window.push_back(packet);
            }
            sendto(sock, (const char *)packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&recvGBN, len);
            trans_count++;                      // Incrementing transmission count
            auto temp_time = chrono::high_resolution_clock::now();
            //Time in microseconds
            long time = chrono::duration_cast<chrono::microseconds>(temp_time.time_since_epoch()).count();
            RTT_start[(win_trav + start)%256] = time;       // Storing start time for RTT calculation
            transmit_count[(win_trav + start)%256]++;
            if(transmit_count[(win_trav + start)%256] >= 5) // Max retransmit check
            {
                cout << "Max Retransmit" << endl;
                exit_function();
            }
            timeout_thread = thread(timeout_ack, win_trav, start, window_count);    // Starting timeout thread
            timeout_thread.detach();
            win_trav++;
            m.unlock();
        }
        while(1)                                // Wait for all packets to be acked or timeout
        {
            m.lock();
            if(timeout_check)                   // Timeout occured
            {
                timeout_check = false;
                m.unlock();
                break;
            }
            if(LFT == window_size - 1)          // All packets acked
            {
                packet_window.clear();          // Clear window
                window_count++;
                m.unlock();
                break;
            }
            m.unlock();
        }
        m.lock();
        if(empty_window)                        // Remapping sequence numbers
        {
            empty_window = false;
            for(int i = LFT + 1; i<256; i++)
            {
                thread_kills[i] = false;
                transmit_count[i] = 0;
                RTT_start[i] = 0;
            }
        }
        start = (start + LFT + 1)%256;          // Resetting start value
        LFT = -1;
        prev_LFT = -1;
        m.unlock();
    }
}

void ack_receiver(int sock, struct sockaddr_in &sendGBN, socklen_t &len, long time_micro)
{
    while(1)                                    // Recieve acks
    {
        char buffer[MAX_LINE] = {0};
        recvfrom(sock, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&sendGBN, (socklen_t *) &len); 
        m.lock();
        if(buffer[0] == 'E')                    // End message
        {
            cout << "END msg recieved" << endl;
            exit_function();
        }
        auto temp_time = chrono::high_resolution_clock::now();
        //Time in microseconds
        long time = chrono::duration_cast<chrono::microseconds>(temp_time.time_since_epoch()).count();
        long RTT = time - RTT_start[stoi(buffer)];  // Calculating RTT
        RTT_avg = RTT_avg * stoi(buffer) + RTT;
        RTT_avg = RTT_avg / (stoi(buffer) + 1);     // Calculating average RTT
        RTT_start[stoi(buffer) ] -= time_micro;     // Adjusting start time for RTT calculation
        if(debug)                                   // Debug RTT calculation
        {
            cout << "Seq #: " << buffer << " Time Generated: " << RTT_start[stoi(buffer)]/1000 << ":" << RTT_start[stoi(buffer)]%1000 << " RTT: " << RTT << " Number of Attempts: " << transmit_count[stoi(buffer)] << endl;
        }
        ack_count = max(ack_count, stoi(buffer) + 1);   // Incrementing ack count
        if(ack_count == max_packets)                    // Max packets received exit call
        {
            cout << "Max packets received" << endl;
            exit_function();
        }
        if(ack_count > 10)                              // Adjusting timeout after 10 packets
        {
            timeout = 2 * RTT_avg;
        }
        if(stoi(buffer) >= (LFT + start+1)%256)         // New ack and update LFT
        {
            prev_LFT = LFT;
            LFT = (stoi(buffer) - start)%256;
            if(LFT < 0) LFT += 256;
            for(int i = prev_LFT + 1; i <= LFT; i++)
            {
                if(thread_kills[(i + start)%256] == false)
                {
                    thread_kills[(i + start)%256] = true;
                }
            }
        }
        if(LFT < 0) LFT += 256;
        m.unlock();
    }
}

void GBN_connect()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);      // Creating socket
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
    recvGBN.sin_addr.s_addr = inet_addr(ip_val.c_str());

    char buffer[MAX_LINE] = {0};
    socklen_t len_ack;
    socklen_t len_packet = sizeof(recvGBN);

    auto start_time = chrono::high_resolution_clock::now();     // Time in microseconds for relative time
    long time_micro = chrono::duration_cast<chrono::microseconds>(start_time.time_since_epoch()).count();

    thread packet_send_thread(packet_sender, sock, ref(recvGBN), ref(len_packet));      // Starting packet sender thread
    thread ack_recv_thread(ack_receiver, sock, ref(sendGBN), ref(len_ack), time_micro); // Starting ack receiver thread

    packet_send_thread.join();
    ack_recv_thread.join();
}

int main(int argc, char *argv[])
{
    srand(time(0));

    for(int i=1; i<argc; i++)               // Parsing command line arguments
    {
        if(strcmp(argv[i], "-p") == 0)
        {
            port_no = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            ip_val = argv[i+1];
        }
        else if(strcmp(argv[i], "-w") == 0)
        {
            window_size = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            debug = true;
        }
        else if(strcmp(argv[i], "-f") == 0)
        {
            buffer_size = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            max_packets = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-l") == 0)
        {
            packet_len = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-r") == 0)
        {
            gen_rate = atoi(argv[i+1]);
        }
    }

    thread packet_gen_thread(packet_generator);     // Starting packet generator thread
    thread GBN_connect_thread(GBN_connect);         // Starting GBN connection thread

    packet_gen_thread.join();
    GBN_connect_thread.join();
}