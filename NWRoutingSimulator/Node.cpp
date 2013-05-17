/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "Node.h"
#include<iostream>
#include<vector>
#include<unistd.h>
//#include<Windows.h> // *** TODO *** Change this for Unix

using std::cout;

Node::Node(char id) : id(id), inputChannel(Channel(id, false)), outputChannel(Channel(id, false)), multicast(MulticastRouting(id, outputChannel, LinkStateRouting(id), false, source))
{ 
    currentRound = timeStamp = 0;
    roundsToRun = 150;
    isReceiver = false;
    isSender = false;
}

Node::Node(char id, bool receiver, char source): id(id), isReceiver(receiver), source(source), inputChannel(Channel(id, false)), outputChannel(Channel(id, false)),  multicast(MulticastRouting(id, outputChannel, LinkStateRouting(id), isReceiver, source))
{
    currentRound = timeStamp = 0;
    roundsToRun = 150;
    isSender = false;
}

Node::Node(char id, bool sender, string data): id(id), isSender(sender), sourceData(data), inputChannel(Channel(id, false)), outputChannel(Channel(id, false)), multicast(MulticastRouting(id, outputChannel, LinkStateRouting(id), false, source))
{
    currentRound = timeStamp = 0;
    roundsToRun = 150;
    isReceiver = false;
}

Node::~Node(void)
{
}

void Node::sendHello()
{
    outputChannel.write(Constants::HELLO + " " + id);
}

string Node::readInputChannel()
{
    return inputChannel.readNewBatchMessages();
}

void Node::processBatchMessages(string& batchMessages)
{
    // Process each message separately
    vector<string> messages;
    Channel::separateMessages(batchMessages, messages);
    cout << "\n Done separating messages";

    for(vector<string>::iterator mit = messages.begin() ; mit != messages.end(); ++mit)
    {
        Message msg;
        msg.processMessageString(*mit, currentRound);

        switch (msg.getMessageType())
        {
        case MsgType::HELLO:
            // Handle HELLO
            multicast.getLinkState().helloMessageReceived(msg);
            break;

        case MsgType::LINKSTATE:
            // Handle linkstate
            if(msg.getSenderID() != id)
            {
                if(multicast.getLinkState().linkStateMessageReceived(msg, *mit, currentRound))
                {
                    // New LSA received
                    forwardLinkStateAdvertisement(*mit);
                    multicast.linkStateChanged(currentRound);
                }
            }

            break;
        
        case MsgType::JOIN:
            // Handle join
            multicast.addToJoinMsgQueue(msg);
            multicast.processJoinMessageQueue();
            break;

        case MsgType::DATA:
            // Handle Data
            multicast.dataMessageReceived(msg);
            break;

        default:
            break;
        }
    }
}

void Node::sendLinkStateAdvertisement()
{
    // Increment timestamp and send LinkStateAD
    outputChannel.write(multicast.getLinkState().getLinkStateAd(++timeStamp));
}

void Node::forwardLinkStateAdvertisement(string& lsa)
{
    // Send LSA as is to neigbors
    outputChannel.write(lsa);
}

void Node::run()
{

    for (currentRound = 0; currentRound < roundsToRun; ++currentRound)
    {
        // Read the input file and process each new message received
        string msgBatch = readInputChannel();
        cout << "Input: " << msgBatch.size() << "\n";    
        cout << msgBatch;

	    if(msgBatch.size() > 0)
	    {
            processBatchMessages(msgBatch);
	    }
     
        // Update the LinkState Info
        if(multicast.getLinkState().updateState(currentRound))
        {
            // Send new LSA
            sendLinkStateAdvertisement();

            // Update multicast state
            multicast.linkStateChanged(currentRound);
        }

        multicast.processJoinMessageQueue();

        // Update Multicast State
        multicast.updateState(currentRound);

        // send hello message to neighbors if its time
        if(0 == (currentRound % Constants::HELLO_INTERVAL))
        {
            sendHello();
        }

        // Send link state advertisements if its time
        if(0 == (currentRound % Constants::LINKSTATE_AD_INTERVAL))
        {
            sendLinkStateAdvertisement();
        }

        // Send a join message to each parent of each tree if its time
        // TODO Change this to keep track of last join sent separately for each tree
        if(0 == (currentRound % Constants::MULTICAST_REFRESH_TO_PARENT_INTERVAL))
        {
            multicast.refreshParent(currentRound);
        }

        // If this node is sender, send source data string if its time
        if(isSender && 0 == (currentRound % Constants::SRC_DATA_SEND_INTERVAL))
        {
            multicast.sendData(sourceData);
        }

        // Sleep for 1 second
      
	cout << "\nSleeping :zZZ...";
    //Sleep(1000); 
	usleep(1000000);
	cout << "\nAwaken...";
    }

}

int main(int argc, char* argv[])
{
    // 1. Process command line arguments
    // 2. Call appropriate constructor to create the Node
    // 3. Call node.run() to start processing

    // 1.
    vector<string> args(argv + 1, argv + argc);
    bool isSender = false;
    bool isReceiver = false;
    char nodeId;

    vector<string>::iterator it = args.begin();

    // First is nodeId
    if(it!= args.end())
    {
        nodeId = it->at(0);
        
        ++it;
        //Second is "sender" or "receiver"
        if(it!= args.end())
        {
            if("sender" == *it)
            {
                isSender = true;
            } 
            else if("receiver" == *it)
            {
                isReceiver = true;
            }

            ++it;
            // Third is sourceNode Id or the string data
            if(it!= args.end())
            {
                if(isSender)
                {                   
                    // 2.
                    Node node(nodeId, isSender, *it);
                    cout << "\nNode " << nodeId << " is sender and will send \"" << *it << "\"";

                    // 3.
                    node.setRoundsToRun(150);
                    node.run();
                } 
                else if(isReceiver)
                {
                    // 2.
                    Node node(nodeId, isReceiver, it->at(0));
                    cout << "\nNode " << nodeId << " is receiver of Source Node " << it->at(0);

                    // 3.
                    node.setRoundsToRun(150);
                    node.run();
                }
            }
        }
        else
        {
            // This node is neither sender nor receiver
            // 2.
            Node node(nodeId);
            cout << "\nNode " << nodeId << " is NEITHER sender NOR receiver";

            // 3.
            node.setRoundsToRun(150);
            node.run();
        }
    }    

   // getchar();
}
