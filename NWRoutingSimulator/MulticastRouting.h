/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#pragma once
#include "Constants.h"
#include "Message.h"
#include "LinkStateRouting.h"
#include "Channel.h"
#include<map>
#include<deque>

typedef struct SourceTreeInfo
{
    char parent;
    map<char,int> children;
} SourceTree;

class MulticastRouting
{
private:
    const char myId;
    Channel myOutputChannel;
    LinkStateRouting linkState;
    map<char,SourceTree> multicastDB;
    queue<Message> joinMessageQueue;
    const bool isReceiver;
    char rootSource;

    bool joinSourceTree(Message& msg);

public:
    MulticastRouting(char id, Channel& outputChannel, LinkStateRouting linkState, bool isReceiver, char source);
    ~MulticastRouting(void){};

    void addToJoinMsgQueue(Message msg){ joinMessageQueue.push(msg); };
    void linkStateChanged(int currentTime);
    void refreshParent(int currentTime);
    void processJoinMessageQueue();

    LinkStateRouting& getLinkState() { return linkState; };

    void sendData(string& dataStr); // Called by node only if its a sender
    void dataMessageReceived(Message& msg);

    void updateState(int currentTime);
};

