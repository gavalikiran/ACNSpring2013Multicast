/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "Message.h"
#include<iostream>

Message::Message(Message const& other)
{
    type = other.getMessageType();
    senderID = other.getSenderID();
    timeStamp = other.getTimeStamp();
    
    set<char>::const_iterator nit = other.getIncomingNeighbors().begin();
    while (nit != other.getIncomingNeighbors().end())
    {
        incomingNeighbors.insert(*nit);
    }
    
    rootSourceID = other.getRootSourceID();
    parentID = other.getParentID();
    
    intermediateNodes = other.getIntermediateNodes();
    //rootID = other.getRootID();
    dataStr = other.getDataStr();
    timeRecieved = other.getTimeReceived();
}

void Message::processMessageString(string& messageStr, int timeReceived)
{
    unsigned found = messageStr.find_first_of(" ");
    unsigned last = 0;
    int partNum = 1;
    string part;

    //while (found != string::npos) THIS DOES NOT WORK ON UNIX
    while (found >= 0 && found <= 200)
    {
        //cout << "\n Found= " << found;
        part = messageStr.substr(last, found - last);
        last = found + 1;

        // Process the current part of the message
        switch (partNum)
        {
        case 1: // All: Message Type
            if(Constants::HELLO == part)
            {
                type = MsgType::HELLO;
            } 
            else if (Constants::LINKSTATE == part)
            {
                type = MsgType::LINKSTATE;
            }
            else if (Constants::JOIN == part)
            {
                type = MsgType::JOIN;
            }
            else if (Constants::DATA == part)
            {
                type = MsgType::DATA;
            }

            ++partNum;
            break;

        case 2: // All: Sender ID
            senderID = part.at(0);            
            ++partNum;
            break;

        case 3: 
            if (MsgType::LINKSTATE == type)
            {
                // Linkstate: TS 
                timeStamp = stoi(part);
            }
            else if (MsgType::JOIN == type)
            {
                // Join: SourceID
                rootSourceID = part.at(0);
            }
            else if (MsgType::DATA == type)
            {
                // Data: rootid
                rootSourceID = part.at(0);
            }

            ++partNum;
            break;

        case 4:
            if (MsgType::LINKSTATE == type)
            {
                // Linkstate: incoming neighbor ID
                incomingNeighbors.insert(part.at(0));
            }
            else if (MsgType::JOIN == type)
            {
                // join: parent ID
                parentID = part.at(0);
            }
            else if (MsgType::DATA == type)
            {
                // Data: datastring
                dataStr = part;
            }
            
            ++partNum;
            break;

        default: // Called for partNum > 4; 
            if (MsgType::LINKSTATE == type)
            {
                // Linkstate: incoming neighbor ID
                incomingNeighbors.insert(part.at(0));
            }
            else if (MsgType::JOIN == type)
            {
                // Join: intermediate node ids
                intermediateNodes.push_back(part.at(0));
            }
            else if (MsgType::DATA == type)
            {
                // Data: datastring
                dataStr.append(" " + part);
            }
            
            ++partNum;
            break;
        }

        // Find next part
        found = messageStr.find_first_of(" ",last);
	//cout << "\n Found at end= " << found;
    }

    // Process the last part of the msg betn the last space and the end of line
    part = messageStr.substr(last);
    if(part.length() > 1)
    {
        if (MsgType::DATA == type)
        {
            // Data: datastring
            dataStr.append(" " + part);
        } 
        else if (MsgType::LINKSTATE == type)
        {
           // Linkstate: TS 
           if(partNum == 3)
           {
               timeStamp = stoi(part);
            }
        }
    }
    else if(1 == part.size())
    {
        if (MsgType::HELLO == type)
        {
            // HELLO: incoming neighbor ID
            incomingNeighbors.insert(part.at(0));
            senderID = part.at(0);
        }
        else if (MsgType::JOIN == type)
        {
            if(partNum++ == 4)
            {
                // join: parent ID
                parentID = part.at(0);
            }
            else if (partNum++ > 4 )
            {
                // Join: intermediate node ids
                intermediateNodes.push_back(part.at(0));
            }
        }
        else if (MsgType::LINKSTATE == type)
        {
            // Linkstate: TS 
            if(partNum == 3)
            {
                timeStamp = stoi(part);
            }
            else if (partNum++ >= 4)
            {
                // Join: incoming neighbor ids
                incomingNeighbors.insert(part.at(0));
            }
        }
    }

    this->timeRecieved = timeReceived;
}

string Message::toString()
{
    string tmp;

    if (MsgType::HELLO == type)
    {
        tmp.append(Constants::HELLO + " ");
        tmp.push_back(senderID);
    }
    else if(MsgType::DATA == type)
    {
        tmp.append(Constants::DATA + " ");
        tmp.push_back(senderID);
        tmp.append(" ");
        tmp.push_back(rootSourceID);
        tmp.append(" " + dataStr);
    }
    else if(MsgType::LINKSTATE == type)
    {
        tmp.append(Constants::LINKSTATE + " ");
        tmp.push_back(senderID);
        string ts = to_string((long double) timeStamp);
        if(ts.length() == 1)
        {
            tmp.append("  0" + ts);
        }
        else
        {
            tmp.append(" " + ts);
        }
        
        set<char>::const_iterator neighbor = incomingNeighbors.begin();
        while (neighbor != incomingNeighbors.end())
        {
            tmp.append(" " + *neighbor);
        }
    }
    else if(MsgType::JOIN == type)
    {
        tmp.append(Constants::JOIN + " ");
        tmp.push_back(senderID);
        tmp.append(" ");
        tmp.push_back(rootSourceID);
        tmp.append(" ");
	tmp.push_back(parentID);
        
        if (!intermediateNodes.empty())
        {
            tmp.append(" ");
	    tmp.push_back(intermediateNodes.front());
            intermediateNodes.pop_front();
        }
    }

	//cout << "\nMSG TOSTRING: " << tmp;
    return tmp;
}
