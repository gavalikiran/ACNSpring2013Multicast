/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#pragma once
#include<set>
#include<deque>
#include "Constants.h"
#include<string>

using namespace std;

typedef enum MessageType
    {
        HELLO, JOIN, DATA, LINKSTATE
    } MsgType;

class Message
{
private:
    MsgType type;
    char senderID;
    
    // Link state
    int timeStamp;
    set<char> incomingNeighbors;
    
    // Multicast
    char rootSourceID;
    char parentID;
    deque<char> intermediateNodes;

    // Data
    string dataStr;

    // Time when message was received
    int timeRecieved;

public:
    Message(void){ timeRecieved = 0;};
    Message(Message const& other);
    ~Message(void){};
    MsgType getMessageType() const { return type; };
    char getSenderID() const { return senderID; };
    int getTimeStamp() const { return timeStamp; };
    set<char> const& getIncomingNeighbors() const { return incomingNeighbors; };
    deque<char> getIntermediateNodes() const { return intermediateNodes; };
    char getRootSourceID() const { return rootSourceID; };
    char getParentID() const { return parentID; };
    string getDataStr() const { return dataStr; };
    void processMessageString(string& messageStr, int timeRecieved);
    //static char getSenderId(string& msgStr);
    int getTimeReceived() const { return timeRecieved; };
    string toString();

    void setDataStr(string& data) { dataStr = data; };
    void setTimeReceived(int time) { timeRecieved = time; };
    void setRootSourceID(char root) { rootSourceID = root; };
    void setMessageType(MsgType type) { this->type = type; };
    void setSenderID(char id) { senderID = id; };
    void setParentID(char id) { parentID = id; };
    void setIntermediateNodes(deque<char>& nodeList) { intermediateNodes.clear(); intermediateNodes = nodeList; };
};

