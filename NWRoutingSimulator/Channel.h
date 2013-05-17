/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#pragma once
#include<iostream>
#include<fstream>
#include<string>
#include<ios>
#include<vector>
#include "Constants.h"

using namespace std;

class Channel
{
private:
    char id;
    string inputFile;
    string outputFile;
    ios::streampos lastEnd;

public:
    Channel() {};
    //Channel(char id);
    Channel(char id, bool controller);
    ~Channel(void);
    Channel(const Channel& other);
    Channel& operator= (Channel& other);
    string readNewBatchMessages();
    void write(string text);
    static void separateMessages(string& batchMessages, vector<string>& result);
};

