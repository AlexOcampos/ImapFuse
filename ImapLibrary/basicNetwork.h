/**
 * @file basicNetwork.h
 * @author Alejandro Ocampos Veiga
 * @brief Basic IMAP functions (header). Adapted from Althea Project by Alejandro Ocampos for ImapFuse.
 * @date 30/03/2011
 */
 
#ifndef BASICNETWORK_H
#define BASICNETWORK_H
namespace std {};
using namespace std;

#include        <iostream>
#include        <stdlib.h>
#include        <errno.h>
#include        <unistd.h>
#include        <string.h>
#include        <string>
#include        <list>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <stdio.h>
#include        "tcpUtilities.h"
#include        "errors.h"
#include        "connection.h"

// reads in a line from sock
string readTilEOL(AConPtr the_connection);

// reads in lines until they contain either of the expected strings
string readTilEOLTilExpected(AConPtr the_connection, const string &expected1, const string &expected2);

// same as above but it checks for errors
string readTilEOLTilExpected(AConPtr the_connection, const string &expected1, const string &expected2, int &error);

//converts integers to strings
string itoa(const int i);

#endif
