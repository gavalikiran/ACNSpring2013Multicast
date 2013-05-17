/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#pragma once
#include<string>
#include<queue>

using namespace std;

typedef struct NextHopInfo
{
    char nextHop;
    int hopCount;
    queue<char> path; 
} NextHop;

class Constants
{
private:
    Constants(void){};
    ~Constants(void){};

public:
    static const string TOPOLOGY_FILE_NAME;
    static const string LOG_FILE_NAME;
    static const string INPUT_PREFIX;
    static const string OUTPUT_PREFIX;
    static const string RECEIVER_FILE_PREFIX;
    static const string RECEIVER_FILE_INFIX;
    static const string HELLO;
    static const string JOIN;
    static const string DATA;
    static const string LINKSTATE;
    static const int HELLO_INTERVAL;
    static const int LINKSTATE_AD_INTERVAL;
    static const int LINKSTATE_AD_LIFETIME;
    static const int MULTICAST_REFRESH_TO_PARENT_INTERVAL;
    static const int MULTICAST_REFRESH_FROM_CHILD_LIFETIME;
    static const int MAX_TIMESTAMP;
    static const int SRC_DATA_SEND_INTERVAL;    
};

