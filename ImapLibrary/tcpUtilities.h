/**
 * @file tcpUtilities.h
 * @author Alejandro Ocampos Veiga
 * @brief Adapted in C from Douglas Comer's 'Internetworking with TCP/IP' 
 * by Jeff Ondich and Lauren Jantz, summer 1995. Rewritten in C++, 
 * Jeff Ondich, January 2000. These are utility functions that are 
 * intended to make very simple TCP-based client/server programs 
 * relatively easy to write. Adapted from Althea Project by Alejandro 
 * Ocampos for ImapFuse.
 * @date 30/03/2011
 */

// changes to allow comile on Suns and BSD Boxen thanks to Thomas Strömberg
#ifndef TCPUTILITIES_H
#define TCPUTILITIES_H
namespace std {};
using namespace std;

#include        <iostream>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <arpa/inet.h>
#include        <unistd.h>
#include        <netdb.h>
#include        <errno.h>
#include        <string.h>
#include        <string>
#include        "connection.h"


// Solaris needs this for bzero/bcopy, and probably others.
#include               <strings.h>

// Solaris does not define this. 
#ifndef INADDR_NONE
       #define INADDR_NONE ((unsigned long int) 0xffffffff)
#endif

#define		DEFAULT_QUEUE_LENGTH 	5

/////////////////////////////////////
// Client function prototypes.
/////////////////////////////////////

AConPtr MakeConnection( const char *hostName, int port, bool usessl );


/////////////////////////////////////
// Client and server function
// prototypes.
/////////////////////////////////////

int ReadFromSocket( AConPtr, char *buffer, int nBytes );
void WriteToSocket( AConPtr, const char *buffer, int nBytes );
void GetPeerHostName( AConPtr, char *name, int nameArrayLimit );

#endif
