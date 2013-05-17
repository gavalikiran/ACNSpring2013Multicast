/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "MulticastRouting.h"

MulticastRouting::MulticastRouting(char id, Channel& opChannel, LinkStateRouting linkState, bool isReceiver, char source) : myId(id), myOutputChannel(opChannel), linkState(linkState), isReceiver(isReceiver), rootSource(source)
{
    cout << "\n MCAST CREATED: " << id << " isRecieve= " << isReceiver << " source= " << rootSource;    
    //joinMessageQueue;
    //multicastDB;
}

void MulticastRouting::processJoinMessageQueue()
{
    queue<Message> unprocessedMsgs;

    while (!joinMessageQueue.empty())
    {
        // Get the first message in the  queue
        Message msg = joinMessageQueue.front();
        joinMessageQueue.pop();

        // See if this is my join message back to me
        if(msg.getSenderID() == myId && msg.getParentID() == myId)
        {
            // Do nothing.
            continue;
        }

        // Check if this message was sent by me.
        if (msg.getSenderID() == myId && isReceiver)
        {
            // Join Msg by me and I'm receiver. This was earlier attempt to join source tree which could not
            // happen because of No path from Src-dst or dst-parent.
            if(!joinSourceTree(msg))
            {
                // Put the message back in the queue to be processed later
                unprocessedMsgs.push(msg);
                cout << "\nNOT ABLE TO JOIN SOURCE TREE " + msg.toString();
            }
        }
        else if(msg.getSenderID() != myId)
        {
            // This join message was not sent by me. Process it
            // Check if this is a JOIN MESSAGE or a SourceRouting Message
            if (msg.getIntermediateNodes().empty())
            {
                // JOIN Message. Check if its for me
                if(msg.getParentID() == myId)
                {
                    // My child wants to join a source tree
                    // - Join the source tree if not already done
                    // - Update the expiration interval for this child on the tree if already present
                    if(!joinSourceTree(msg))
                    {
                        // Put the message back in the queue to be processed later
                        unprocessedMsgs.push(msg);
                        cout << "\nCannot currently process JOIN MSG " + msg.toString();
                    }
                }
            }
            else
            {
                // Source Routing Message. Route it to the appropriate neigbor
                // Check if I'm supposed to process the msg
                char nextNode = msg.getIntermediateNodes().front();

                if(nextNode == myId)
                {
                    // Remove my ID from the list of intermediate nodes and fwd
                    deque<char> tmpQueue = msg.getIntermediateNodes();
                    tmpQueue.pop_front();
                    msg.setIntermediateNodes(tmpQueue);
                    myOutputChannel.write(msg.toString());
                } // else just ignore the message
            }
        } // end of "if(msg.getSenderID() != myId)"
    }

    // Copy the unprocessed messages back to the joinMessagequeue
    joinMessageQueue = unprocessedMsgs;
}

bool MulticastRouting::joinSourceTree(Message& msg)
{    
    // Validate SourceID
    if(!(msg.getRootSourceID() >= '0' && msg.getRootSourceID() <= '9'))
    {
        // Invalid root id. // Drop this msg
        return true;
    }

    // Check if the tree exists
    map<char,SourceTree>::iterator sit = multicastDB.find(msg.getRootSourceID());

    if(sit != multicastDB.end())
    {
        // SourceTree exists. Update info: Reset the Expire interval for this child
        sit->second.children[msg.getSenderID()] = Constants::MULTICAST_REFRESH_FROM_CHILD_LIFETIME;
    }
    else
    {
        // SourceTree does not exist. Create a new one
        // Get parent on the path from Root - Me
        deque<char> pathFromRoot;
        
        if(!linkState.getShortestPath(msg.getRootSourceID(), myId, pathFromRoot))
        {
            // No path found from Root to Me. LinkState Routing hasn't stabilized.
            // Stop processing for now
            return false;
        }

        pathFromRoot.pop_back(); // Delete myId
        char myParentOnTree = pathFromRoot.back(); // Get the Id of the node before me
        
        // Get Shortest path from Me to my parent
        deque<char> pathFromMeToParent;
        
        if(!linkState.getShortestPath(myId, myParentOnTree, pathFromMeToParent))
        {
            // No path found from Me to Myparent. LinkState Routing hasn't stabilized.
            // Stop processing for now
            return false;
        }

        // Create a new Tree for this source
        SourceTree tree;
        tree.parent = myParentOnTree;
        tree.children[msg.getSenderID()] = Constants::MULTICAST_REFRESH_FROM_CHILD_LIFETIME;
        multicastDB[msg.getRootSourceID()] = tree;

        // Check if I'm the root of this tree
        if(myId == msg.getRootSourceID() && myId == myParentOnTree)
        {
            // I'm the Root. We're done        
        }
        else
        {
            // I'm not the root of the tree. Send join msg upward towards the Root
            // Update the current msg and write to output channel with SourceRouting Information
            Message joinMsg = msg;
            joinMsg.setMessageType(MsgType::JOIN);
            //joinMsg.setSenderID(myId);
            joinMsg.setRootSourceID(msg.getRootSourceID());
            joinMsg.setSenderID(msg.getSenderID());
            joinMsg.setParentID(myParentOnTree);
            pathFromMeToParent.pop_back(); // Remove the parent
            pathFromMeToParent.pop_front(); // Remove me
            joinMsg.setIntermediateNodes(pathFromMeToParent);

            myOutputChannel.write(joinMsg.toString());
        }
    }

    return true; // Default
}

void MulticastRouting::linkStateChanged(int currentTime)
{
    // For every source tree of which I'm part of
    map<char,SourceTree>::iterator treeIt = multicastDB.begin();

    while (treeIt != multicastDB.end())
    {
        // Check if I'm the root for this tree. if so skip
        if(treeIt->first == myId)
        {
            ++treeIt;
            continue;
        }
        
        // Get parent on the path from Root - Me
        deque<char> pathFromRoot;
        
        if(!linkState.getShortestPath(treeIt->first, myId, pathFromRoot))
        {
            // No path found from Root to Me. LinkState Routing hasn't stabilized.
            // Or the path from Src-dst is gone. Remove entry for parent from mcast tree if one exists.
            if(!multicastDB.empty())
            {
                multicastDB.erase(treeIt++->first);
            }
            else
            {
                ++treeIt;
            }

            continue;
        }

        pathFromRoot.pop_back(); // Delete myId
        char newParent = pathFromRoot.back();
        
        // Check if the path from Root to Me(Parent) has changed
        if(treeIt->second.parent != newParent)
        {
            // Get Shortest path from Me to my parent
            deque<char> pathFromMeToParent; // TODO Can store this in the Multicast DB to avoid calculation everytime
        
            if(!linkState.getShortestPath(myId, newParent, pathFromMeToParent))
            {
                // No path found from Me to Myparent. LinkState Routing hasn't stabilized.
                // This parent & tree will be updated in the next check. 
            }
            else
            {
                // yes, update new parent info in the tree
                treeIt->second.parent = newParent;

                // Send join message to new parent
                Message joinMsg;
                joinMsg.setMessageType(MsgType::JOIN);
                joinMsg.setSenderID(myId);
                joinMsg.setParentID(newParent);
                pathFromMeToParent.pop_back(); // Remove the parent
                pathFromMeToParent.pop_front(); // Remove me
                joinMsg.setIntermediateNodes(pathFromMeToParent);

                myOutputChannel.write(joinMsg.toString());
            }

        }// if(treeIt->second.parent != pathFromRoot.back()) -- else No update required
        
        ++treeIt;
    }
    
}

void MulticastRouting::refreshParent(int currentTime)
{
    // For all source trees I'm part of send refresh msg to parent separately on each tree
    map<char,SourceTree>::const_iterator treeIt = multicastDB.begin();

    while (treeIt != multicastDB.end())
    {

        // Get Shortest path from Me to my parent
        deque<char> pathFromMeToParent; // TODO Can store this in the Multicast DB to avoid calculation everytime
        
        if(!linkState.getShortestPath(myId, treeIt->second.parent, pathFromMeToParent))
        {
            // No path found from Me to Myparent. LinkState Routing hasn't stabilized.
            // TODO Put message in JoinQueue and mark as refresh? 
        }
        else
        {
            // Got shortest path. Send join msg to parent
            Message joinMsg;
            joinMsg.setMessageType(MsgType::JOIN);
            joinMsg.setSenderID(myId);
            joinMsg.setRootSourceID(treeIt->first);
            joinMsg.setParentID(treeIt->second.parent);
            pathFromMeToParent.pop_back(); // Remove the parent
            pathFromMeToParent.pop_front(); // Remove me
            joinMsg.setIntermediateNodes(pathFromMeToParent);
            joinMsg.setTimeReceived(currentTime);

            myOutputChannel.write(joinMsg.toString());
        }

        ++treeIt;
    }

    // If I'm receiver, create a new source tree if one does not exist
    if(isReceiver)
    {
        // Join the tree of the source
        Message msg;
        msg.setMessageType(MsgType::JOIN);
        msg.setSenderID(myId);
        msg.setRootSourceID(rootSource);
        msg.setParentID(myId);
        msg.setTimeReceived(currentTime);

        if(!joinSourceTree(msg))
        {
            joinMessageQueue.push(msg);
        }
    }

}

void MulticastRouting::updateState(int currentTime)
{
    // Check to see if any join msg from child has expired. If so remove it from the tree
    // Decrement time to expiry for all children in all trees
    map<char,SourceTree>::iterator treeIt = multicastDB.begin();

    while (treeIt != multicastDB.end())
    {
        map<char,int>::iterator childIt = treeIt->second.children.begin();

        while (childIt != treeIt->second.children.end())
        {
            if(childIt->first == myId)
            {
                // I'm a receiver of this source. Ignore expiration for me
                ++childIt;
            }
            else
            {
                // Update expiration for this child
                if(--childIt->second == 0)
                {
                    // Haven't heard from child in the last Constants::MULTICAST_REFRESH_FROM_CHILD_LIFETIME
                    // Delete the child
                    treeIt->second.children.erase(childIt++);
                }
                else
                {
                    ++childIt;
                }
            }
        }

        // Check if there are any children for this source tree
        if(treeIt->second.children.size() < 1)
        {
            // There are no receivers for this source where I'm on tree. Delete the tree
            multicastDB.erase(treeIt++);
        }
        else
        {
            ++treeIt;
        }
    }
}

void MulticastRouting::sendData(string& dataStr)
{
    // Send source string to all children in my source tree
//    map<char,SourceTree>::const_iterator treeIt = multicastDB.find(myId);

        cout << "\nSource sneding data: data " << myId << " " << myId <<  " " << dataStr; 
	Message dataMsg;
        dataMsg.setMessageType(MsgType::DATA);
        dataMsg.setSenderID(myId);
        dataMsg.setRootSourceID(myId);
        dataMsg.setDataStr(dataStr);
        
    myOutputChannel.write(dataMsg.toString());
/*
    // Check if there are receivers connected to me.
    if(treeIt != multicastDB.end())
    {
        // My Source Tree exists!
        map<char,int>::const_iterator childIt = treeIt->second.children.begin();
        
        while (childIt != treeIt->second.children.end())
        {
            // Send data to this child   
            myOutputChannel.write(dataMsg.toString());
            ++childIt;
        }
    }
*/
}

void MulticastRouting::dataMessageReceived(Message& dataMsg)
{
    if (dataMsg.getRootSourceID() == myId)
    {
        // Msg originally sent by me. Do nothing. 
    }
    else
    {
        map<char,SourceTree>::const_iterator treeIt = multicastDB.find(dataMsg.getRootSourceID());

        if(treeIt != multicastDB.end())
        {
            // I'm on this source's tree. Check if data came from parent on tree
            if(dataMsg.getSenderID() == treeIt->second.parent)
            {
                // Data came from parent on tree. Forward to children with updated senderID
                map<char,int>::const_iterator childIt = treeIt->second.children.begin();
        
                while (childIt != treeIt->second.children.end())
                {
                    if(childIt->first == myId)
                    {
                        if(isReceiver && rootSource == dataMsg.getRootSourceID())
                        {
                            // I have received data from my source. Write to file
                            ofstream opfile;
                            opfile.open(Constants::RECEIVER_FILE_PREFIX + myId + Constants::RECEIVER_FILE_INFIX + rootSource + ".txt", ios::out | ios::app);

                            if (opfile.is_open())
                            {
                                //opfile << dataMsg.getDataStr() << endl;
                                opfile << dataMsg.toString() << "\n";
                            }
                            opfile.close();
                        }
                    }
                    else
                    {
                        // Send data to my child on the tree
                        dataMsg.setSenderID(myId);
                        myOutputChannel.write(dataMsg.toString());
                    }

                    ++childIt;
                }
            } // End of "if(dataMsg.getSenderID() == treeIt->second.parent)"
        }
        // No source tree for thi source yet. Check if directly came from source to me
        else if (dataMsg.getRootSourceID() == rootSource && dataMsg.getSenderID() == rootSource && isReceiver)
        {
            // Message came from my intended source directly to me
            ofstream opfile;
            opfile.open(Constants::RECEIVER_FILE_PREFIX + myId + Constants::RECEIVER_FILE_INFIX + rootSource + ".txt", ios::out | ios::app);

            if (opfile.is_open())
            {
                //opfile << dataMsg.getDataStr() << endl;
                opfile << dataMsg.toString() << "\n";
            }

            opfile.close();
        }
    }
}
