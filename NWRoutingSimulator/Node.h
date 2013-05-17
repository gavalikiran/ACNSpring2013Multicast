/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#pragma once
#include "Constants.h"
#include "Channel.h"
#include "Message.h"
#include "LinkStateRouting.h"
#include "MulticastRouting.h"

class Node
{
private:
    char id;
    Channel inputChannel;
    Channel outputChannel;
    bool isReceiver;
    bool isSender;
    string sourceData;
    char source;
    //LinkStateRouting linkstate;
    MulticastRouting multicast;
    
    int currentRound;
    int roundsToRun;
    int timeStamp;

    void sendHello();
    string readInputChannel();
    void processBatchMessages(string& batchMessages);
    void sendLinkStateAdvertisement();
    void forwardLinkStateAdvertisement(string& lsa);
    
public:
    Node(char id);
    Node(char id, bool receiver, char source);
    Node(char id, bool sender, string data);
    ~Node(void);
    void setRoundsToRun(int rnd){ roundsToRun = rnd; };
    void run();
};

