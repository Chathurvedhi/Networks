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

mutex mtx;  // Mutex for locking

class file_info     // Storing min and max cost for each neighbour
{
public:
    int min_cost;
    int max_cost;
};

class lsa_info      // Storing LSA information for each node
{
public: 
    int seqno;
    unordered_map<int, int> entries;
};
 
class path_info     // Storing path information for each node
{
public:
    int cost = INT_MAX;
    vector<int> path;
};

class ospf          // Main class for OSPF
{
public:
    int id;
    int hello_interval = 1;
    int lsa_interval = 5;
    int spf_interval = 20;
    int sockfd;
    int lsa_seqno = 1;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    struct sockaddr_in neighbour_addr;
    int len;
    long long start_time_ms;                                // Start time in milliseconds
    unordered_map<int, file_info> neighbours_file;          // File information for each neighbour
    unordered_map<int, int> neighbours_cost;                // Cost for each neighbour after helloreply
    unordered_map<int, int> lsa_nodes;                      // Sequence number for each node
    vector<vector<int>> topology_table;                     // Adjacency matrix for topology
    unordered_map<int, lsa_info> lsa_table;                 // LSA info for each node
    string filename_out;                                    // Output file name
    void ospf_init(string filename, string outfile);
    void reciever();
    void hello_response();
    void helloreply_response();
    void lsa_response(string buffer);
    void remove_response();
    void add_response();
    void hello_gen();
    void lsa_gen();
    void topology();
    void compute_topology();
};

void ospf::compute_topology()   // Computes adjacency matrix with LSA information
{
    // Clear topology table
    for(int i = 0; i < topology_table.size(); i++)
    {
        for(int j = 0; j < topology_table.size(); j++)
        {
            topology_table[i][j] = 0;
        }
    }


    int num_nodes = topology_table.size();
    mtx.lock();
    for(int i = 0; i < num_nodes; i++)
    {
        if(i == id) continue;
        unordered_map<int, int> temp_entries;
        temp_entries = lsa_table[i].entries;
        for(auto it = temp_entries.begin(); it != temp_entries.end(); it++)
        {
            topology_table[i][it->first] = it->second;
            topology_table[it->first][i] = it->second;
        }
    }
    mtx.unlock();
}

void ospf::lsa_response(string buffer)    // LSA response
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

    // Update LSA table
    mtx.lock();
    lsa_info lsa_temp;
    lsa_temp.seqno = n_seqno;
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
        lsa_temp.entries[entry_id] = entry_cost;
    }
    lsa_table[n_id] = lsa_temp;
    mtx.unlock();

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
        
    // Random cost between min and max
    int cost;
    cost = rand() % (neighbours_file[n_id].max_cost - neighbours_file[n_id].min_cost + 1) + neighbours_file[n_id].min_cost;
    neighbours_cost[n_id] = cost;
    
    // Send Hello Reply with cost
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
    
    // Check if our id is correct
    char* receiver_id;
    receiver_id = strtok(NULL, "|");
    int our_id = atoi(receiver_id);
    if(our_id != id)
    {
        return;
    }

    // Cost update
    char* cost;
    cost = strtok(NULL, "|");
    int n_cost = atoi(cost);
    neighbours_cost[n_id] = n_cost;
}

void ospf::remove_response()
{
    char* nodes = strtok(NULL, "|");
    int node1 = atoi(nodes);
    nodes = strtok(NULL, "|");
    int node2 = atoi(nodes);
    int n_id;
    if(node1 == id) n_id = node2;
    else n_id = node1;
    neighbours_file.erase(n_id);
    neighbours_cost.erase(n_id);
}

void ospf::add_response()
{
    char* nodes = strtok(NULL, "|");
    int node1 = atoi(nodes);
    nodes = strtok(NULL, "|");
    int node2 = atoi(nodes);
    int n_id;
    if(node1 == id) n_id = node2;
    else n_id = node1;
    nodes = strtok(NULL, "|");
    int min_cost = atoi(nodes);
    nodes = strtok(NULL, "|");
    int max_cost = atoi(nodes);
    neighbours_file[n_id].min_cost = min_cost;
    neighbours_file[n_id].max_cost = max_cost;
}

void ospf::reciever()
{
    while(1)
    {
        // Recieve packet
        char buffer[MAX_LINE];
        int n = recvfrom(sockfd, (char *)buffer, MAX_LINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
        buffer[n] = '\0';
        string packet = buffer;

        // Check packet type
        char* pack_type;
        pack_type = strtok(buffer, "|");

        // Call appropriate function
        if(strcmp(pack_type, "HELLO")==0) hello_response();
        else if(strcmp(pack_type, "HELLOREPLY")==0) helloreply_response();
        else if(strcmp(pack_type, "LSA")==0) lsa_response(packet);
        else if(strcmp(pack_type,"r") == 0) remove_response();
        else if(strcmp(pack_type,"a") == 0) add_response();
        else if(strcmp(pack_type,"q") == 0) exit(0);
    }
}

void ospf::hello_gen()
{
    while(1)
    {
        // Sleep for hello interval
        sleep(hello_interval);

        // Packet to send
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
        
        /* Printing topology
        fstream fp;
        fp.open("Outputs/output_" + to_string(id), ios::out);
        fp << "Node " << id << " : " << endl;
        
        // Compute topology and print as adjacency matrix
        compute_topology();
        for(int i = 0; i < topology_table.size(); i++)
        {
            for(int j = 0; j < topology_table[i].size(); j++)
            {
                fp << topology_table[i][j] << " ";
            }
            fp << endl;
        }
        */

        // Compute topology with lsa_table
        compute_topology();
        
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

        // Calculate time now
        auto now = chrono::high_resolution_clock::now();
        long long int time_now = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        time_now = time_now - start_time_ms;
        time_now = time_now / 1000; 

        // Print to file
        fstream fp;
        fp.open("Outputs/" + filename_out, ios::out);
        fp<<"Shortest Path from "<<id<<" to all other nodes at time : " << time_now << endl;
        fp<<"Node | Cost | Path"<<endl;
        for(int i=0;i<num_nodes;i++)
        {
            if(i == id) continue;
            fp<< i << " | ";
            if(dist[i].cost == INT_MAX)
            {
                fp<<"No path exists"<<endl;
                continue;
            }
            fp<< dist[i].cost << " | ";
            for(auto it = dist[i].path.begin(); it != dist[i].path.end(); it++)
            {
                fp<<*it<<"-";
            }
            fp << i << " |" << endl;
        }
        fp.close();
    }
}

void ospf::ospf_init(string filename, string outfile)
{
    // Basic data
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

    // Default filename or given filename
    if(outfile == "")
    {
        outfile = "output_" + to_string(id);
    }
    filename_out = outfile;

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

    // store time of initialisation
    auto start_time = chrono::high_resolution_clock::now();
    start_time_ms = chrono::duration_cast<chrono::milliseconds>(start_time.time_since_epoch()).count();

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

    // Threads for all processes
    thread ospf_reciever(&ospf::reciever, this);
    thread ospf_hello_gen(&ospf::hello_gen, this);
    thread ospf_topology(&ospf::topology, this);
    thread ospf_lsa_gen(&ospf::lsa_gen, this);

    ospf_reciever.join();
    ospf_hello_gen.join();
    ospf_topology.join();
    ospf_lsa_gen.join();
}

int main(int argc, char* argv[])
{
    string filename = argv[1];
    fstream fp;
    fp.open(filename, ios::in);
    int num_nodes;
    fp >> num_nodes;
    cout << "Number of nodes : " << num_nodes << endl;
    fp.close();
    
    // Fork num_nodes processes
    for(int i = 0; i < num_nodes ; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            // Child process
            ospf node;
            node.id = i;
            node.ospf_init(filename, "");
        }
    }

    // Create client socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);    
    if(sock < 0)
    {
        cout<<"Socket creation failed"<<endl;
        exit(0);
    }
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;

    char buffer[MAX_LINE] = {0};
    socklen_t len = sizeof(servaddr);

    sleep(5);   // Wait for all nodes to initialise

    while(1)
    {
        cout << "Enter a command : " << endl;
        char c;
        cin >> c;
        if(c == 'q')
        {
            cout << "Exiting and shutting down all the servers" << endl;
            for(int i = 0; i < num_nodes; i++)
            {
                string msg = "q";
                servaddr.sin_port = htons(10000 + i);
                sendto(sock, (const char *)msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
            }
            exit(0);
        }
        else if(c == 'r')
        {
            cout << " Enter the link to be removed : " << endl;
            int u, v;
            cin >> u >> v;
            string msg = "r|" + to_string(u) + "|" + to_string(v);

            // Send to u and v
            servaddr.sin_port = htons(10000 + u);
            sendto(sock, (const char *)msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
            servaddr.sin_port = htons(10000 + v);
            sendto(sock, (const char *)msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
        }
        else if(c == 'a')
        {
            cout << "Enter the link to be added : " << endl;
            int u, v, w_1, w_2;
            cin >> u >> v >> w_1 >> w_2;
            string msg = "a|" + to_string(u) + "|" + to_string(v) + "|" + to_string(w_1) + "|" + to_string(w_2);

            // Send to u and v
            servaddr.sin_port = htons(10000 + u);
            sendto(sock, (const char *)msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
            servaddr.sin_port = htons(10000 + v);
            sendto(sock, (const char *)msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
        }
    }
    
}