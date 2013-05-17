/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "Channel.h"
#include<vector>

Channel::Channel(char id, bool controller): id(id)
{
    string nodeId = to_string((long long int) atoi(&this->id));

    if(controller)
    {
        inputFile += Constants::OUTPUT_PREFIX;
        inputFile += nodeId;
        inputFile += ".txt";
    
        outputFile += Constants::INPUT_PREFIX;
        outputFile += nodeId;
        outputFile += ".txt";
    }
    else
    {
        inputFile += Constants::INPUT_PREFIX;
        inputFile += nodeId;
        inputFile += ".txt";
    
        outputFile += Constants::OUTPUT_PREFIX;
        outputFile += nodeId;
        outputFile += ".txt";
    }
}

Channel::Channel(const Channel& other): id(other.id), inputFile(other.inputFile), outputFile(other.outputFile), lastEnd(other.lastEnd)
{
}

Channel& Channel::operator=(Channel& other)
{
    id = other.id;
    inputFile = other.inputFile;
    outputFile = other.outputFile;
    lastEnd = other.lastEnd;

    return *this;
}

Channel::~Channel(void)
{
}


string Channel::readNewBatchMessages()
{
    cout << "\n--- READ ---\n";
    cout << "Before read lastend= " << lastEnd << "\n";

    string newBatchMessages;

    // Open file in read mode
    ifstream ipstream;
    ipstream.open(inputFile, ios::in);

    if(ipstream.is_open()){
        // Read data updated since the last read
        ipstream.seekg(0, ios::end);
        ios::streampos newEnd = ipstream.tellg();
        cout << "\nBefore read newend= " << newEnd << "\n";
        string line;

        if (newEnd > lastEnd)
        {
            // New data written after last read.. Read it
            ios::streamoff newData = newEnd - lastEnd;
            ipstream.seekg(-newData, ios::end);
            
            while (ipstream.good())
            {
                getline(ipstream,line);
                newBatchMessages += line + "\n";
            }

            cout << "\nData read: " << newBatchMessages.size() << "\n";
        }
        
        // Remember the end of the file
        lastEnd = newEnd;
        cout << "\nAfter read lastend= " << lastEnd << "\n";
	//ipstream.close();
    }
    else
    {
        // Error opening file
    }

    // Close the file
    ipstream.close();
    cout << "\nDone reading data.\n";

    return newBatchMessages;
}

void Channel::write(string text)
{
    cout << "\n --- Write ---";
    cout << "\nBefore write LastEnd=" << lastEnd << "\n";

    // Open file in write,append mode
    ofstream opstream;
    opstream.open(outputFile, ios::out | ios::app);
    
    if(opstream.is_open())
    {
        // Add "\n" to the message
        text += "\n";

        // Append data to the end of the file
        int size = text.size();
        //opstream.write(text.c_str(), size);
        //opstream.write("\n", 1);
        opstream << text;

        cout << "\n Bytes written= " << size << "\n";
        cout << "\nAfter write LastEnd=" << opstream.tellp() << "\n";

	//opstream.flush();
	//opstream.close();
    }
    else
    {
        // Error opening file
    }

    // Close the file
    opstream.flush();
    opstream.close();
    cout << "\nDone writing data.\n";
    
}

void Channel::separateMessages(string& batchMessages, vector<string>& result)
{
    cout << "\nChannel::separateMessages--- Input= " << batchMessages;
    unsigned found = batchMessages.find_first_of("\n");
    unsigned last = 0;
    string msg;
    cout << "\n length= " << batchMessages.length();

    while (last < batchMessages.length() && found != string::npos)
    {
        msg = batchMessages.substr(last, found - last).data();
        cout << "\n Message: " << msg << " Last= " << last;

        if(!msg.empty())
        {
            result.push_back(msg.data());
        }
        
        last = found + 1;
        found = batchMessages.find_first_of("\n",last);
    }

    cout << "\nResult size: " << result.size();
    cout << "\nExiting Separate messages";
    //return result;
}
