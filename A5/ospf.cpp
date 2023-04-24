#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <random>
#include <ctime>
#include <fstream>

using namespace std;

#define MAX_LINE 1024
#define localhost "127.0.0.1"

class file_info
{
public:
    int min_cost;
    int max_cost;
};

class lsa_info
{
public:
    int seqno;
    unordered_map<int, int> entries;
};

class path_info
{
public:
    int cost = INT_MAX;
    vector<int> path;
};

class ospf
{
public:
    int id;
    int hello_interval = 2;
    int lsa_interval = 5;
    int spf_interval = 15;
    int sockfd;
    int lsa_seqno = 1;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    struct sockaddr_in neighbour_addr;
    int len;
    unordered_map<int, file_info> neighbours_file;
    unordered_map<int, int> neighbours_cost;
    unordered_map<int, int> lsa_nodes;
    vector<vector<int>> topology_table;
    unordered_map<int, lsa_info> lsa_table;
    void ospf_init(string filename);
    void reciever();
    void hello_response();
    void helloreply_response();
    void lsa_response(string buffer);
    void hello_gen();
    void lsa_gen();
    void topology();
    void dijkstra();
};

void ospf::dijkstra()
{
    // Implement Dijkstra's Algorithm along with Shortest Path Finding with topology table
    int num_nodes = topology_table.size();
    vector<path_info> dist(num_nodes);
    vector<bool> visited(num_nodes, false);
    dist[id].cost = 0;
    for(int iter = 0; iter < num_nodes; iter++)
    {
        int min_dist = INT_MAX;
        int min_index = -1;

        for(int i=0;i<num_nodes;i++)
        {
            if(visited[i] == false && dist[i].cost <= min_dist)
            {
                min_dist = dist[i].cost;
                min_index = i;
            }
        }

        visited[min_index] = true;
        
        for(int i=0;i<num_nodes;i++)
        {
            if(!visited[i] && topology_table[min_index][i] && dist[min_index].cost != INT_MAX && dist[min_index].cost + topology_table[min_index][i] < dist[i].cost)
            {
                dist[i].cost = dist[min_index].cost + topology_table[min_index][i];
                dist[i].path = dist[min_index].path;
                dist[i].path.push_back(min_index);
            }
        }
    }
    
    fstream fp;
    fp.open("Outputs/output_" + to_string(id), ios::app);
    fp<<"Shortest Path from "<<id<<" to all other nodes:"<<endl;
    for(int i=0;i<num_nodes;i++)
    {
        if(i == id) continue;
        fp<<"Node "<<i<<": ";
        for(auto it = dist[i].path.begin(); it != dist[i].path.end(); it++)
        {
            fp<<*it<<"-";
        }
        fp << i << endl;
        fp<<" Cost: "<<dist[i].cost<<endl;
    }
    fp.close();
}

void ospf::lsa_response(string buffer)
{
    // Sender details
    char* sender_id;
    sender_id = strtok(NULL, "|");
    int n_id = atoi(sender_id);
    char* sender_seqno;
    sender_seqno = strtok(NULL, "|");
    int n_seqno = atoi(sender_seqno);

    // Check Sequence number
    if(lsa_nodes[n_id] >= n_seqno) return;
    lsa_nodes[n_id] = n_seqno;

    // Update topology table
    int num_entries;
    char* num_entries_str;
    num_entries_str = strtok(NULL, "|");
    num_entries = atoi(num_entries_str);

    for(int i=0; i<num_entries; i++)
    {
        char* entry;
        entry = strtok(NULL, "|");
        int entry_id = atoi(entry);
        entry = strtok(NULL, "|");
        int entry_cost = atoi(entry);
        topology_table[n_id][entry_id] = entry_cost;
        topology_table[entry_id][n_id] = entry_cost;
    }

    // Send LSA to all neighbours
    for(auto it = neighbours_file.begin(); it != neighbours_file.end(); it++)
    {
        if(it->first == n_id) continue;
        // Send to port 10000 + it->first
        neighbour_addr.sin_family = AF_INET;
        neighbour_addr.sin_port = htons(10000 + it->first);
        neighbour_addr.sin_addr.s_addr = inet_addr(localhost);
        sendto(sockfd, (const char *)buffer.c_str(), strlen(buffer.c_str()), MSG_CONFIRM, (const struct sockaddr *)&neighbour_addr, len);
    }
}

void ospf::hello_response()
{
    // Sender details
    char* sender_id;
    sender_id = strtok(NULL, "|");
    int n_id = atoi(sender_id);
        
    // Debug
    //cout<<"Received HELLO from "<<n_id<<endl;

    // Random cost between min and max
    int cost;
    cost = rand() % (neighbours_file[n_id].max_cost - neighbours_file[n_id].min_cost + 1) + neighbours_file[n_id].min_cost;
    neighbours_cost[n_id] = cost;
    
    // Send Hello Reply
    char hello_reply[MAX_LINE];
    strcpy(hello_reply, "HELLOREPLY|");
    strcat(hello_reply, to_string(id).c_str());
    strcat(hello_reply, "|");
    strcat(hello_reply, to_string(n_id).c_str());
    strcat(hello_reply, "|");
    strcat(hello_reply, to_string(cost).c_str());
    sendto(sockfd, (const char *)hello_reply, strlen(hello_reply), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
}

void ospf::helloreply_response()
{
    // Sender details
    char* sender_id;
    sender_id = strtok(NULL, "|");
    int n_id = atoi(sender_id);
    
    // Debug
    //cout<<"Received HELLOREPLY from "<<n_id<<endl;

    // Check if our id is correct
    char* receiver_id;
    receiver_id = strtok(NULL, "|");
    int our_id = atoi(receiver_id);
    if(our_id != id)
    {
        cout << "Error : Wrong Reciever ID" << endl;
        return;
    }

    // Cost update
    char* cost;
    cost = strtok(NULL, "|");
    int n_cost = atoi(cost);
    neighbours_cost[n_id] = n_cost;
}

void ospf::reciever()
{
    while(1)
    {
        char buffer[MAX_LINE];
        int n = recvfrom(sockfd, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
        buffer[n] = '\0';
        string packet = buffer;
        char* pack_type;
        pack_type = strtok(buffer, "|");
        if(strcmp(pack_type, "HELLO")==0) hello_response();
        else if(strcmp(pack_type, "HELLOREPLY")==0) helloreply_response();
        else if(strcmp(pack_type, "LSA")==0) lsa_response(packet);
    }
}

void ospf::hello_gen()
{
    while(1)
    {
        sleep(hello_interval);
        char hello[MAX_LINE];
        strcpy(hello, "HELLO|");
        strcat(hello, to_string(id).c_str());

        // Send to all neighbours
        for(auto it = neighbours_file.begin(); it != neighbours_file.end(); it++)
        {
            // Send to port 10000 + it->first
            neighbour_addr.sin_family = AF_INET;
            neighbour_addr.sin_port = htons(10000 + it->first);
            neighbour_addr.sin_addr.s_addr = inet_addr(localhost);
            sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&neighbour_addr, len);
        }
    }
}

void ospf::lsa_gen()
{
    while(1)
    {
        sleep(lsa_interval);
        string lsa;
        lsa = "LSA|";
        lsa += to_string(id);
        lsa += "|";
        lsa += to_string(lsa_seqno);
        lsa += "|";
        int num_neighbours = neighbours_cost.size();
        lsa += to_string(num_neighbours);
        lsa += "|";
        for(auto it = neighbours_cost.begin(); it != neighbours_cost.end(); it++)
        {
            lsa += to_string(it->first);
            lsa += "|";
            lsa += to_string(it->second);
            lsa += "|";
        }
        
        // Send to all neighbours
        for(auto it = neighbours_file.begin(); it != neighbours_file.end(); it++)
        {
            // cout<< " Sending LSA to " << it->first << endl;
            // Send to port 10000 + it->first
            neighbour_addr.sin_family = AF_INET;
            neighbour_addr.sin_port = htons(10000 + it->first);
            neighbour_addr.sin_addr.s_addr = inet_addr(localhost);
            sendto(sockfd, (const char *)lsa.c_str(), strlen(lsa.c_str()), MSG_CONFIRM, (const struct sockaddr *)&neighbour_addr, len);
        }
        lsa_seqno++;
    }
}

void ospf::topology()
{
    while(1)
    {
        sleep(spf_interval);
        fstream fp;
        fp.open("Outputs/output_" + to_string(id), ios::out);
        fp << "Node " << id << " : " << endl;
        
        // Print a Adjaency Matrix
        fp << "Adjacency Matrix : " << endl;
        for(int i = 0; i < topology_table.size(); i++)
        {
            for(int j = 0; j < topology_table[i].size(); j++)
            {
                fp << topology_table[i][j] << " ";
            }
            fp << endl;
        }
        fp << "Neighbours :" << endl;
        for(auto it = neighbours_cost.begin(); it != neighbours_cost.end(); it++)
        {
            fp << it->first << " " << it->second << endl;
        }
        fp.close();
        dijkstra();
    }
}


void ospf::ospf_init(string filename)
{
    fstream fp;
    fp.open(filename, ios::in);
    int num_nodes;
    fp >> num_nodes;
    int num_edges;
    fp >> num_edges;

    // initialise adjacency matrix
    for(int i = 0; i < num_nodes; i++)
    {
        vector<int> temp;
        for(int j = 0; j < num_nodes; j++)
        {
            temp.push_back(0);
        }
        topology_table.push_back(temp);
    }

    // Read lines to get all neighbours for id
    for(int i = 0; i < num_edges; i++)
    {
        int u, v, w_1, w_2;
        fp >> u >> v >> w_1 >> w_2;
        if(u == id)
        {
            file_info temp;
            temp.min_cost = w_1;
            temp.max_cost = w_2;
            neighbours_file[v] = temp;
        }
        else if(v == id)
        {
            file_info temp;
            temp.min_cost = w_1;
            temp.max_cost = w_2;
            neighbours_file[u] = temp;
        }
    }
    fp.close();

    cout << "Initialised OSPF for node " << id << endl;

    // UDP socket with port number 10000 + id
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        cout<<"Socket creation failed"<<endl;
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(10000 + id);
    if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        cout<<"Bind failed"<<endl;
        exit(0);
    }
    len = sizeof(cliaddr);

    // Threads
    thread ospf_reciever(&ospf::reciever, this);
    thread ospf_hello_gen(&ospf::hello_gen, this);
    thread ospf_topology(&ospf::topology, this);
    thread ospf_lsa_gen(&ospf::lsa_gen, this);

    ospf_reciever.join();
    ospf_hello_gen.join();
    ospf_topology.join();
    ospf_lsa_gen.join();
}

int main(int argc, char *argv[])
{
    string filename;
    ospf op;
    // Arguments for running the program
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-f") == 0)
        {
            // File name
            filename = argv[i+1];
        }
        else if(strcmp(argv[i], "-i") == 0)
        {
            // Router ID
            op.id = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-h") == 0)
        {
            // Hello interval
            op.hello_interval = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-a") == 0)
        {
            // LSA interval
            op.lsa_interval = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            // SPF interval
            op.spf_interval = atoi(argv[i+1]);
        }
    }
    op.ospf_init(filename);
}