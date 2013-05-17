/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "LinkStateRouting.h"
#include<iostream>

LinkStateRouting::LinkStateRouting(char id): myId(id)
{
    //incomingNeighors;
    //linkStateDB;
    //linkStateDBTimeStamps;
    //topology;
}

bool LinkStateRouting::linkStateMessageReceived(Message& newMsg, string lsa, int currentTime)
{
    // Add the sender of the LSA to the nodes list
    //nodes.insert(newMsg.getSenderID());

    // Check if this LSA is newer than already received from the sender
    map<char,string>::iterator found = linkStateDB.find(newMsg.getSenderID());
    bool newInfo = false;

    if(linkStateDB.end() != found)
    {
        // Found a LSA from this sender. Compare timeStamps and update accordingly
        Message oldMsg;
        oldMsg.processMessageString(found->second, currentTime);

        if(newMsg.getTimeStamp() > oldMsg.getTimeStamp() )
        {
            // Newer LSA. remove previous ones 
            linkStateDB.erase(newMsg.getSenderID());
            linkStateDBTimeStamps.erase(newMsg.getSenderID());
            newInfo = true;
        }
    } 
    else
    {
        // No prevous LSA received from this sender. Add this one to the LSA DB    
        newInfo = true;
    }

    if (newInfo)
    {
        // Store new info in the LSA DB and send
        linkStateDB.insert(pair<char,string>(newMsg.getSenderID(), lsa));
        linkStateDBTimeStamps.insert(pair<char,int>(newMsg.getSenderID(), currentTime));

        // Check to see if new nodes are discovered in this msg
        /*set<char>::const_iterator nodeIt = newMsg.getIncomingNeighbors().begin();

        while (nodeIt != newMsg.getIncomingNeighbors().end())
        {
            nodes.insert(*nodeIt++);
        }*/
    }

    return newInfo;
}

void LinkStateRouting::helloMessageReceived(Message& msg)
{
    // Add the msg sender to the incoming neigbors list
    if(msg.getSenderID() >= '0' && msg.getSenderID() <= '9' && msg.getSenderID() != myId)
    {
        incomingNeighors.insert(msg.getSenderID());
        //nodes.insert(msg.getSenderID());
    }
    
    // TODO Update timestamp for hello message receipt?
}

string LinkStateRouting::getLinkStateAd(int timeStamp)
{
    string ad = Constants::LINKSTATE + " " + myId;

    string ts = to_string((long long int)timeStamp % 100);
        if(ts.length() == 1)
        {
            ad.append(" 0" + ts);
        }
        else
        {
            ad.append(" " + ts);
        }

    for (set<char>::const_iterator incomingNeighbor = incomingNeighors.begin(); incomingNeighbor != incomingNeighors.end(); ++incomingNeighbor)
    {
        ad.append(" ");
        ad.push_back(*incomingNeighbor);
    }

    return ad;
}

bool LinkStateRouting::updateState(int currentTime)
{
    bool isChange = false;
    // TODO If node down (check hello interval), update incoming neihbors; isChange= true

    // Go through the LSAs and remove expired entries
    map<char,int>::iterator lsaTSIt = linkStateDBTimeStamps.begin();
    
    while (lsaTSIt != linkStateDBTimeStamps.end())
    {
        if ((currentTime - lsaTSIt->second) >= Constants::LINKSTATE_AD_LIFETIME)
        {
            // Last LSA from this sender was received LIFETIME or more seconds ago. Remove LSA for it
            char sender = lsaTSIt++->first;
            linkStateDB.erase(sender);
            linkStateDBTimeStamps.erase(sender);

            // Also remove it from the incoming neighbors list. Because if neighbot hasn't sent linkState for 30 sec it is down
            if (!incomingNeighors.empty())
            {
                incomingNeighors.erase(sender);
            }

            isChange = true;
        } 
        else
        {
            ++lsaTSIt;
        }
    }

    //if(isChange)
    //{
        // update topology
        updateTopology();
    //}

    return isChange;
}

void LinkStateRouting::addLink(char src, char dst)
{
    // Add a unidirectional link from src->dst
    map<char,set<char> >::iterator found = topology.find(src);

    if(topology.end() == found)
    {
        // No links from this src found. Create a new set with dst and add to the map
        set<char> newSet;
        newSet.insert(dst);
        //topology.emplace(src, newSet);
        topology.insert(std::pair<char,set<char> >(src,newSet));
        //topology[src] = newSet;
    }
    else
    {
        // Found a list of links from this src. Add dst to the list
        found->second.insert(dst);
    }

    cout << "\nAdded link: " << src << " -> " << dst;
}

void LinkStateRouting::updateTopology()
{
    // Create/Update topology based on IncomingNeighbors and LinkStateDB info
    //set<char>::const_iterator nodesIt = nodes.begin();
    topology.clear();

    // Go through all the LSAs
    map<char,string>::iterator lsaIt = linkStateDB.begin();

    while (lsaIt != linkStateDB.end())
    {
        Message msg;
        msg.processMessageString(lsaIt->second, linkStateDBTimeStamps[lsaIt->first]);

        set<char>::const_iterator inNeighborIt = msg.getIncomingNeighbors().begin();

        while (inNeighborIt != msg.getIncomingNeighbors().end())
        {
            addLink(*inNeighborIt++, lsaIt->first);
        }

        ++lsaIt;
    }

    // Process incoming nodes
    set<char>::const_iterator inNeighbor = incomingNeighors.begin();
    while (inNeighbor != incomingNeighors.end())
    {
        addLink(*inNeighbor++, myId);
    }
}

bool LinkStateRouting::getShortestPath(char src, char dst, deque<char>& path)
{
    if(src == dst)
    {
        path.push_front(src);
        path.push_front(src);
        return true;
    }

    // Use BFS: It will find the shortest path (one) from src-dst
    map<char,char> bfsTree;
    queue<char> children;
    children.push(src);
    bool pathFound = false;

// TODO TEST START REMOVE
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
// TEST END REMOVE

    while (!pathFound && !children.empty())
    {
        // Get child
        char currentNode = children.front();
        children.pop();

        // Process connected nodes
        map<char,set<char> >::const_iterator connNodeIt = topology.find(currentNode);

        if(connNodeIt != topology.end())
        {
            // Found connected nodes which are possible children. Process them
            set<char>::const_iterator possibleChild = connNodeIt->second.begin();

            while (possibleChild != connNodeIt->second.end())
            {
                // Check if this child is visited.
                map<char,char>::const_iterator childIt = bfsTree.find(*possibleChild);
                if(childIt == bfsTree.end())
                {
                    // This child is not visited. Add it to the tree: Parent = currentNode, Child=*possibleChild
                    bfsTree[*possibleChild] = currentNode;

                    // Check if the child is destination. If so, stop processing graph.
                    if(*possibleChild == dst)
                    {
                        pathFound = true;
                        break;
                    }

                    // Add it to the queue for processing subtree rooted at this child
                    children.push(*possibleChild);
                }

                ++possibleChild;
            }
        } // end of "if(connNodeIt != topology.end())"
    }

    if(pathFound)
    {
        // Build path from Src-Dst in reverse order traversing from dst to src on tree
        char currentNode = dst;

        while (currentNode != src)
        {
            path.push_front(currentNode); // Add to the front of the queue/path
            
            // Get parent on bfstree
            currentNode = bfsTree[currentNode];
        }

        // Add src to the beginning of the path
        path.push_front(src);
    }

    return pathFound;
}
