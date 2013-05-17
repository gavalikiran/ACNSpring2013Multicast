/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#pragma once
#include "Constants.h"
#include "Message.h"
#include<set>
#include<map>
#include<deque>

class LinkStateRouting
{
private:
    char myId;    
    set<char> incomingNeighors;
    map<char,set<char> > topology;
    map<char,string> linkStateDB;
    map<char,int> linkStateDBTimeStamps;
    
    void addLink(char src, char dst);
    void updateTopology();

public:
    LinkStateRouting(char id);
    ~LinkStateRouting(void){};
    
    bool updateState(int currentTime);

    string getLinkStateAd(int timeStamp);
    set<char> const& getIncomingNeighbors();
    bool getShortestPath(char src, char dst, deque<char>& path); // Returns true if path from Src-Dst(stored in "path") found; false otherwise
    
    bool linkStateMessageReceived(Message& msg, string lsa, int currentTime);
    void helloMessageReceived(Message& msg);

};

