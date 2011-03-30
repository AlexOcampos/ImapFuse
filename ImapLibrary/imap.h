/**
 * @file imap.h
 * @author Alejandro Ocampos Veiga
 * @brief  Imap basic functions (header). Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * Caution: Currently only the following functions were tested:
 * IMAPLogin, getIMAPFolders, SelectFolder, GetNewMail, IMAPLogout,
 * GetMessageHeader, GetLineText, GetFlags, tolower 
 * @date 30/03/2011
 * 
 */

#ifndef IMAP_H
#define IMAP_H
namespace std {};
using namespace std;

#include    <iostream>
#include    <stdlib.h>
#include    <errno.h>
#include    <unistd.h>
#include    <string.h>
#include    <string>
#include    <list>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    "tcpUtilities.h"
#include    "basicNetwork.h"
#include    "imapLiterals.h"
#include    "errors.h"
#include    "connection.h"
#include	"Folder.h"

/////////////////////////////////////
// IMAP function prototypes.
/////////////////////////////////////

// AUXILIARES
string tolower(string s);
void EraseBefore(string &haystack, const string &needle);

string GetFlags(string response, int msgNumber, int &errnum, list<string> &parts);

void GetLineText(const string &haystack, const string &keyword, string &value);

int GetNewMail(AConPtr the_connection, Folder &F);

int GetMessageHeader (AConPtr the_connection, int msgNumber, string &to, string &cc, string &from, string &subject, 
		int &uid,string & date,string & flags,list<string> & msgparts);
		
int SelectFolder(AConPtr the_connection, const string &foldername, int &numMessages, 
		 string &folderOpenStatus);

//login to the server

int IMAPLogin(AConPtr &the_connection, const string &host, const int port, const string &username, const string &password, bool usessl);

// get the folders in folderroot (eg of folderroot = "~/mail")

int getIMAPFolders(AConPtr the_connection, list<string>& folders, string folderroot);

int IMAPGetMessagePartByUID(AConPtr the_connection, int UID, string &messagetext, int part);


// get the body of a message (not the header)

int IMAPGetMessageBodyByUID(AConPtr the_connection, int UID, string &messagetext);

// get the full header of a message

int IMAPGetMessageHeaderByUID(AConPtr the_connection, int UID, string &messagetext);

// sets (and unsets) message flags

int IMAPSetFlagByUID(AConPtr the_connection, int UID, const string &flagtoset, string &currentflags);
int IMAPUnSetFlagByUID(AConPtr the_connection, int UID, const string &flagtounset, string &currentflags);


// Expunge doze messages

int IMAPExpunge(AConPtr the_connection, list<int>& deleted);

// Disconnects you from the server

int IMAPLogout(AConPtr the_connection);

// make a folder commands
// to make a directory, the foldername should be followed by a "/"

int IMAPCreateFolder(AConPtr the_connection, const string &foldername, const string &folderroot);
int IMAPDeleteFolder(AConPtr the_connection, const string &foldername, const string &folderroot);
int IMAPRenameFolder(AConPtr the_connection, const string &fromname, const string &toname, const string &folderroot);

//
// copy a message
// to folder in folderroot.
//

int IMAPCopyMessage(AConPtr the_connection, const int uid, const string &tofolder, 
	const string &folderroot);

// convert sequence numbers to UIDs

int IMAPSeq2UID(AConPtr the_connection, const int seq, int &UID);

// runs noop and returns the errorcode of getTilEOLTilExpected (so it will
// tell you when the server says BYE for example...

int IMAPCheckMail(AConPtr the_connection);

// posts the text, text to the folder, folder

int IMAPAppend(AConPtr the_connection, const string &text, const string &folder, const string &folderroot);

#endif




