/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "Controller.h"
#include<fstream>
#include "Message.h"
#include<unistd.h>
//#include<Windows.h> // *** TODO Replace on Unix ***


Controller::Controller(void)
{
    nodeCount = 0;
    set<char> nodes;
    map<char,set<char> > topology;
    map<char,Channel> channels;
    runForRounds = 10000;
    init();
}

void Controller::init()
{
    // Read the configuration
    readTopologyConfiguration();
    
    nodeCount = nodes.size();

    // Initialize the Channels for the nodes
    for (set<char>::iterator it = nodes.begin() ; it != nodes.end(); ++it)
    {
        Channel chan(*it, true);
        channels[*it] = chan;
        cout << "\nInitialized Channel for Node #" << *it;
    }

}

Controller::~Controller(void)
{
}

void Controller::readTopologyConfiguration()
{
    ifstream config(Constants::TOPOLOGY_FILE_NAME);

    if(config.is_open())
    {
        string line;

        while (config.good())
        {
            getline(config, line);
            char src = 'n';
            char dst = 'n';
            
            for(unsigned int i=0; i< line.size(); i++)
            {
                char c = line.at(i);
                if(c >= '0' && c <= '9')
                {
                    cout << "\nNew node # " << c;
                    if('n' == src)
                    {
                        // This is the src of the unidirectional link
                        src = c;
                        nodes.insert(src);
                    }
                    else
                    {
                        // This is the destination of the unidirectional link
                        dst = c;
                        nodes.insert(dst);
                    }
                } // end of "if(c >= '0' && c <= '9')"
            }

            // Add link from src->dst
            addLink(src,dst);
        }           

        config.close();
    }
    else
    {
        cout << "\nERROR; Could not open file: " << Constants::TOPOLOGY_FILE_NAME;
    }
}

void Controller::addLink(char src, char dst)
{
    // Add a unidirectional link from src->dst
    map<char,set<char> >::iterator found = topology.find(src);

    if(topology.end() == found)
    {
        if(src >= '0' && src <= '9' && dst >= '0' && dst <= '9')
        {
            // No links from this src found. Create a new set with dst and add to the map
            set<char> newSet;
            newSet.insert(dst);
            //topology.emplace(src, newSet);
            topology[src] = newSet;
        }
    }
    else
    {
        // Found a list of links from this src. Add dst to the list
        found->second.insert(dst);
    }

    cout << "\nAdded link: " << src << " -> " << dst;
}

void Controller::removeLink(char src, char dst)
{
    // Remove unidirectional link from src->dst
    map<char,set<char> >::iterator found = topology.find(src);

    if(topology.end() != found)
    {
        // Found a list of links from this src. Remove dst from the list
        found->second.erase(dst);

        // If the list is empty, remove node from the map
        if(found->second.empty())
        {
            topology.erase(src);
        }
    }
}

void Controller::doProcessing()
{
    for(int i = 0; i <= runForRounds; ++i)
    {
        // For each node
        for (set<char>::const_iterator it = nodes.begin() ; it != nodes.end(); ++it)
        {
            // Read the node's output channel and process it
            string newData = channels[*it].readNewBatchMessages(); 
            cout << "\n\nRead Output Channel for Node #" << *it;

            if(newData.empty())
            {
                cout << "\nNo new messages from Node #" << *it;
            }
            else
            {
                cout << "\nProcessing new messages from Node #" << *it;
                processMessage(*it, newData);
            }
        }

        
	cout << "\nSleeping ...";
    //Sleep(1000);
	usleep(1000000);
	cout << "\nAwaken...";
    }
}

void Controller::processMessage(char onNode, string& batchMessages)
{
    cout << "\nProcessing messages: \n" << batchMessages << "\n";
    
    // Process each message separately
    vector<string> messages;
    Channel::separateMessages(batchMessages, messages);

    getchar();
    for(vector<string>::iterator mit = messages.begin() ; mit != messages.end(); ++mit)
    {
        cout << "\nController:processMessage--- Msg= " << *mit;

        // TODO if this is hello message update time stamp for neighbor connectivity

        // Get the sender of the message
        //char senderId = Message::getSenderId(*mit);
        //cout << "\nController:processMessage--- Sender= " << senderId;

        // Get topology info for the sender
        map<char,set<char> >::const_iterator it = topology.find(onNode);

        if(it != topology.end())
        {
            cout << "\nController:processMessage--- Found sender in map";

            // Found the node in map. Get the neighbors
            set<char> neighbors = topology[onNode];

            for (set<char>::const_iterator neigbor = neighbors.begin(); neigbor != neighbors.end(); ++neigbor)
            {
                // write msg to the input file of the neighbor
                channels[*neigbor].write(*mit);
                cout << "\n MESSAGE delivered from \'" << onNode << "\' to \'" << *neigbor << "\'";;
            }
        }
    }
}


void Controller::printState()
{
    cout << "\n\n --- NODES --- \nCount= " << nodeCount << "\n";
    for (set<char>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        cout << *it << ", ";
    }

    cout << "\n\n --- TOPOLOGY ---";
    map<char,set<char> >::const_iterator it = topology.begin();

    while(topology.end() != it)
    {
        cout << "\nSource Node " << it->first << " -> ";

        set<char>::const_iterator setIt = it->second.begin();
        while (it->second.end() != setIt)
        {
            cout << " " << *setIt++;
        }

        ++it;
    }
}

void Controller::updateTopology()
{
    // TODO Check if any node/connection is down
    // - go through hello message timestamps. If haven't received from any node in last X seconds remove connections for that node
}

int main(int argc, char* argv[])
{
    Controller ctrler;
    ctrler.setRunForRounds(300);
    ctrler.doProcessing();
    ctrler.printState();

    //getchar();
    return 0;
}
