/*
* ACN 6390 Project Spring 2013
* @author kiiranh
*/

#include "Constants.h"

const string Constants::TOPOLOGY_FILE_NAME = "topology.cfg";
const string Constants::LOG_FILE_NAME = "acn.log";
const string Constants::INPUT_PREFIX = "messages/input_";
const string Constants::OUTPUT_PREFIX = "messages/output_";
const string Constants::RECEIVER_FILE_PREFIX = "messages/";
const string Constants::RECEIVER_FILE_INFIX = "_received_from_";
const string Constants::HELLO = "hello";
const string Constants::JOIN = "join";
const string Constants::DATA = "data";
const string Constants::LINKSTATE = "linkstate";
const int Constants::HELLO_INTERVAL = 5;
const int Constants::LINKSTATE_AD_INTERVAL = 10;
const int Constants::LINKSTATE_AD_LIFETIME = 30;
const int Constants::MULTICAST_REFRESH_TO_PARENT_INTERVAL = 10;
const int Constants::MULTICAST_REFRESH_FROM_CHILD_LIFETIME = 20;
const int Constants::MAX_TIMESTAMP = 99;
const int Constants::SRC_DATA_SEND_INTERVAL = 10;
