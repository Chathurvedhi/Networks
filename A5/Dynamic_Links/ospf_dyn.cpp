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

int hello_interval = 1;
int lsa_interval = 5;
int spf_interval = 20;

int main(int argc, char* argv[])
{
    string filename;
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-f") == 0)
        {
            // File name
            filename = argv[i+1];
        }
        else if(strcmp(argv[i], "-h") == 0)
        {
            // Hello interval
            hello_interval = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-a") == 0)
        {
            // LSA interval
            lsa_interval = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            // SPF interval
            spf_interval = atoi(argv[i+1]);
        }
    }

    // Read the file to get num_nodes and num_links
    fstream fp;
    fp.open(filename, ios::in);
    int num_nodes, num_links;
    fp >> num_nodes >> num_links;
    
}