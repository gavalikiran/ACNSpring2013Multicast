/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

//#pragma once
#include "Channel.h"
#include "Constants.h"
#include<map>
#include<set>

class Controller
{
private:
    int nodeCount;
    set<char> nodes;                 // Holds the IDs of the nodes currently in the network
    map<char,set<char> > topology;   // Topology/Connectivity in the network
    map<char,Channel> channels;      // Input-Output channels for the nodes
    void init();
    void readTopologyConfiguration();
    void processMessage(char onNode, string& batchMessages);
    void addLink(char src, char dst);
    void removeLink(char src, char dst);
    int runForRounds;
    void updateTopology();

public:
    Controller(void);
    ~Controller(void);
    void doProcessing();
    void printState();
    void setRunForRounds(int num) {runForRounds = num;};
};

