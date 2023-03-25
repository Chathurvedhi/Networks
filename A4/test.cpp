#include<bits/stdc++.h>
#include<ctime>

using namespace std;

int main()
{
    time_t t = time(0);
    cout<<t<<endl;
    tm *now = localtime(&t);
    cout<<now->tm_hour<<":"<<now->tm_min<<":"<<now->tm_sec<<endl;
}