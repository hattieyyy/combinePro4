#ifndef UDPRELAY_H_

#define UDPRELAY_H_

#include <iostream>

#include <string.h>

#include <string>

#include <stdexcept>

#include <stdio.h>

#include <stdlib.h>

#include <sstream>

#include <semaphore.h>

#include <map>

#include "UdpMulticast.h"

#include "Socket.h"

#include <queue>

using namespace std;



const int PORT_SIZE = 5;          //Size of a string representing port #

const int IP_SIZE = 15;           //Size of string representing IP address

const int ARGUMENT_SIZE = 21;     //Size of string from command line execution

const int SIZE = 1024;            //Size of messages to be sent to remote nodes

const int PORT_NUM = 24779;       //Default port number

const int MAX_HOST_LENGTH = 20;   //Max length of a host IP address

const int GROUP_LENGTH = 11;      //Max length of a group name (uw1-320-10\0)



//-----------------------------------------------------------------------------

// Class:       UdpRelay

// Description: This class acts as a relay between UDP messages broadcast

//              locally and remote groups (nodes) it's connected to via TCP.

//              A command thread allows the user to enter all appropriate

//              commands for the UdpRelay object. The threads are:

//

//              Command Thread:   Spun up after execution, only a single thread

//                                which takes user commands (add remote group,

//                                delete remote group, show TCP connections,

//                                quit).

//              relayIn Thread:   Spun up after execution, only a single thread

//                                which accepts local UDP broadcasts and sends

//                                them via TCP to all active connections.

//              acceptThread:     Spun up after execution, only a single thread

//                                which accepts TCP connection requests from a

//                                remote group.

//              relayOut Thread:  Spun up after a TCP connection request is

//                                accepted, it accepts TCP messages and

//                                broadcasts them locally via UDP

//

//              Messages sent are in a packet format as follows:

//              Packet header: -32, -31, -30, hop, 4-byte IP addresses of all

//                             nodes that have received the message

//              Followed By:   Actual message terminated by \0

//              The "hop" is the number of IP addresses in the header

//-----------------------------------------------------------------------------

class UdpRelay {

 public:

  //---------------------------------------------------------------------------

  // UdpRelay Constructor

  // Parses command line input into IP and port numbers, instantiates all data

  // members, spins up command, relayIn and accept threads then calls a

  // semaphore

  // wait until a "quit" command is issued.

  //

  // @pre:   char* parameter is a valid IP number concatenated with a port

  //         number

  // @post:  3 threads are spun up, IP and port numbers are saved

  // @param *ipPlusPort:  The IP address and port number:

  //        (XXX.XXX.XXX.XXX:YYYYY)

  //---------------------------------------------------------------------------

  UdpRelay(const char* ipPlusPort);

  //---------------------------------------------------------------------------

  // UdpRelay Destructor

  // Deletes any dynamically allocated data members

  //

  // @pre:   None

  // @post:  char * ipNumber is deleted

  //---------------------------------------------------------------------------

  ~UdpRelay();

  //---------------------------------------------------------------------------

  // sendLocalMessage

  // Broadcasts the char* parameter via UDP

  //

  // @pre:   currentMessage is not NULL

  // @post:  currentMessage is broadcast via UDP

  // @param  *currentMessage: The message to broadcast UDP

  //---------------------------------------------------------------------------

  void sendLocalMessage(char * currentMessage);

  //---------------------------------------------------------------------------

  // recvLocalMessage

  // Receives a local UDP broadcast

  //

  // @pre:   currentMessage is not NULL and is of at least SIZE length

  // @post:  The message received via UDP is copied into currentMessage

  // @param  *currentMessage: The buffer that will contain the message received

  //---------------------------------------------------------------------------

  void recvLocalMessage(char * currentMessage);

  //---------------------------------------------------------------------------

  // commandThread

  // A static class method that is a thread function for the command thread

  //

  // @pre:   *arg parameter represents a valid UdpRelay object

  // @post:  Acts based on user input, eventually terminating upon quit command

  // @param  *arg:  A void pointer to the UdpRelay object creating the thread

  //---------------------------------------------------------------------------

  static void* commandThread(void* arg);

  //---------------------------------------------------------------------------

  // acceptThread

  // A static class method that is a thread function for the accept thread,

  // which loops continually accepting TCP connection requests

  //

  // @pre:   *arg parameter represents a valid UdpRelay object

  // @post:  Opens a TCP connection socket and updates a connection map

  // @param  *arg:  A void pointer to the UdpRelay object creating the thread

  //---------------------------------------------------------------------------

  static void* acceptThread(void* arg);

  //---------------------------------------------------------------------------

  // relayInThread

  // A static class method that is a thread function for the relayIn thread. It

  // loops continually, receiving UDP messages and sending them via TCP to

  // nodes connected to this UdpRelay object

  //

  // @pre:   *arg parameter represents a valid UdpRelay object

  // @post:  None

  // @param  *arg:  A void pointer to the UdpRelay object creating the thread

  //---------------------------------------------------------------------------

  static void* relayInThread(void* arg);

  //---------------------------------------------------------------------------

  // relayOutThread

  // A static class method that is a thread function for the relayOut thread,

  // which spins up when a TCP connection is established. Receives messages via

  // TCP connection and broadcasts them using UDP, if not a duplicate message

  //

  // @pre:   *arg parameter represents a valid outThreadInfo struct

  // @post:  Removes the connection from the map when the thread terminates

  // @param  *arg:  Pointer to the outThreadInfo struct with socket and

  //         UdpRelay

  //---------------------------------------------------------------------------

  static void* relayOutThread(void *arg);

  //---------------------------------------------------------------------------

  // isDuplicatePacket

  // Checks the header of the packet and returns true if the local group IP is

  // already contained in the header (a duplicate message), false otherwise

  //

  // @pre:   currentPacket parameter is of the correct packet format

  // @post:  None

  // @param  currentPacket: The packet received via UDP or TCP

  // @returns bool:         True if the current UdpRelay's IP is contained in

  //                        the packet header, false otherwise

  //---------------------------------------------------------------------------

  bool isDuplicatePacket(char* currentPacket);

  //---------------------------------------------------------------------------

  // putIPIntoPacket

  // Takes a char* packet passed as parameter and adds group IP address of the

  // current UdpRelay node into the header in the format of a single byte for

  // each 3-digit portion of the IP, then increments the "hop" number

  //

  // @pre:   currentPacket has valid packet format

  // @post:  None

  // @param  currentPacket: A packet in valid format described in UdpRelay

  //         header

  //---------------------------------------------------------------------------

  void putIPIntoPacket(char* currentPacket);

  //---------------------------------------------------------------------------

  // getIPNumber

  // Returns the group IP number used at command line execution

  //

  // @pre:   ipNumber is not NULL

  // @post:  None

  // @returns char*:  Group IP number

  //---------------------------------------------------------------------------

  char* getIPNumber();

  //---------------------------------------------------------------------------

  // addExpiredOutThread

  // Pushes a socket descriptor that has been closed into the expiredOutThreads

  // queue

  //

  // @pre:   sd is a closed socket descriptor

  // @post:  sd is added to expiredOutThreads

  // @param  sd: a socket descriptor

  //---------------------------------------------------------------------------

  void addExpiredOutThread(int sd);

  //---------------------------------------------------------------------------

  // checkForDuplicateCxn

  // Checks if the remote connection already exists and if it does, it deletes

  // the already existing connection

  //

  // @pre:   string shall be a valid remote group id host name

  // @post:  if a duplicate connection already existed, the previous connection

  //         will be closed and deleted from the connections map. Otherwise,

  //         there are no changes.

  // @param  const string& GRP_ID: remote host name

  //---------------------------------------------------------------------------

  void checkForDuplicateCxn(const string& GRP_ID);



 private:

  UdpRelay() {}

  //---------------------------------------------------------------------------

  // addRemoteIp

  // Takes a group IP/name and port number parameter and opens a TCP connection

  // to that node. Sends the hostname of this machine to the remote node and

  // updates the tcpCxns map

  //

  // @pre:   remoteGroupID parameter is a valid group IP and port number

  // @post:  Socket is opened for TCP and tcpCxns map is updated

  // @param  remoteGroupID: An group IP/name and port (XXX.XXX.XXX.XXX:YYYYY)

  //---------------------------------------------------------------------------

  void addRemoteIP(string remoteGroupID);

  //---------------------------------------------------------------------------

  // terminateRemoteCxn

  // Closes the socket to the remote node IP/name passed as parameter, then

  // deletes that connection from the map

  //

  // @pre:   remoteGroupID is a valid group IP/name and map contains that group

  // @post:  tcpCxns map is updated with group IP/name entry removed

  // @param  remoteGroupID: A valid group IP/Name

  //---------------------------------------------------------------------------

  void terminateRemoteCxn(string remoteGroupID);

  //---------------------------------------------------------------------------

  // showTCPConnections

  // Displays all open TCP connections, either outgoing or incoming, to cout.

  // If no connections exist, tells user there are no open connections

  //

  // @pre:   None

  // @post:  None

  //---------------------------------------------------------------------------

  void showTCPConnections();

  //---------------------------------------------------------------------------

  // displayHelpMenu

  // Called by commandThread to send all available user commands to cout

  //

  // @pre:   None

  // @post:  None

  //---------------------------------------------------------------------------

  void displayHelpMenu();

  //---------------------------------------------------------------------------

  // setIpChars

  // Takes the group IP number of format (2XX.255.255.255) and puts each set of

  // three numbers into a single char value before storing them in ipChars[]

  //

  // @pre:   ipNumber is of valid format and ipChars is instantiated

  // @post:  ipChars[] array contains the 4-char version of the group IP

  //---------------------------------------------------------------------------

  void setIpChars();

  //---------------------------------------------------------------------------

  // tcpMulticastToRemoteGroups

  // Sends a message via TCP to all remote nodes connected to this UdpRelay

  // node, and informs the user what message was sent and how many bytes

  //

  // @pre:   outPacket has valid packet format

  // @post:  None

  // @param  outPacket: A packet received via UDP to be sent out via TCP

  //---------------------------------------------------------------------------

  void tcpMultiCastToRemoteGroups(char* outPacket);

  //---------------------------------------------------------------------------

  // createUdpRelayServerConnection

  // Called by acceptThread, this method creates a TCP server socket and

  // connects to a TCP client, receives their group name/IP and spins up a

  // relayOut thread before adding the new connection the outThread and

  // outThreadCxns maps

  //

  // @pre:   None

  // @post:  outThread and outThreadCxns maps are updated, a socket is opened

  //---------------------------------------------------------------------------

  void createUdpRelayServerConnection();

  //---------------------------------------------------------------------------

  // terminateAllTcpConnections

  // Closes all open TCP sockets and removes the connection entries from both

  // tcpCxns and outThreadCxns maps

  //

  // @pre:   None

  // @post:  Sockets are closed and both maps have all entries deleted

  //---------------------------------------------------------------------------

  void terminateAllTcpConnections();



  sem_t mutex;        //Halts the main thread until "quit"

  char ipChars[5];    //Chars representing the IP address of the local machine

  char* ipNumber;     //IP number read in from command line at execution

  int portNumber;     //Port number read in from command line at execution

  map<string, int> tcpCxns;  //All outgoing sockets mapped to group name

  map<int, pthread_t> outThreads; //pthread IDs mapped to active sockets

  queue<pthread_t> expiredOutThreads; //Holds terminated relayOut socket #'s

  Socket * relaySock;   //The Socket object used for TCP connections



  //As thread functions need to be static, this struct includes all needed data

  struct outThreadInfo {

    int socketNumber;         //Socket number just created for relayOut thread

    UdpRelay * currentRelay;  //Pointer to UdpRelay object

    string remoteHostName;

  };

};



#endif /* UDPRELAY_H_ */