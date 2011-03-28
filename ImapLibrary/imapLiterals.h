/**
 * imapLiterals.h
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Description: string literals used in imap.cpp
 * Updated: 22/03/2011
 * 
 */

#ifndef IMAPLITERALS_H
#define IMAPLITERALS_H
namespace std {};
using namespace std;

#include	<string>


const string gIMAPStarOK = "* OK";
const string gIMAPOK = "OK";


const string gIMAPLoginTag = "a001";
const string gIMAPLoginCommand = "login";
const string gIMAPLoginExpected = "LOGIN";

const string gIMAPEOL = "\r\n";

const string gIMAPQuoteEOL = "\"\r\n";
const string gIMAPQuoteSpaceQuote = "\" \"";
const string gIMAPQuoteQuote = "\"\"";


//list sending things
const string gIMAPListTag = "a002";
const string gIMAPListCommand = "list";
const string gIMAPListWildcard = "*";
const string gIMAPListNoSelect = "\\NoSelect";

//list receiving things
const string gIMAPListResponse = "* LIST";

//expunge sending things
const string gIMAPExpungeTag = "a666";
const string gIMAPExpungeCommand = "expunge";

//expunge receiving things

const string gIMAPExpungeExpected = "EXPUNGE";
const string gIMAPItemMarker = "*";
const int    gIMAPExpungeCharsToItem=2;

// fetch sending things

const string gIMAPFetchTag = "a003";
const string gIMAPFetchCommand = "uid fetch";
const string gIMAPBodyMIMEPart = "body[1]";

// fetch receiving things
const string gIMAPFetchExpected = "FETCH";
const string gIMAPUIDFetchExpected = "UID FETCH";

// fetch header sending things

const string gIMAPFetchHeaderTag = "a004";
const string gIMAPHeaderMIMEPart = "body[header]";

// set flag sending things

const string gIMAPSetFlagTag = "a005";
const string gIMAPStoreCommand = "uid store";
const string gIMAPSetFlagCommand = "+flags";
const string gIMAPUnSetFlagCommand = "-flags";

// Create folder stuff

const string gIMAPCreateFolderTag = "a006";
const string gIMAPCreateFolderCommand = "create";
const string gIMAPDirectoryIndicator = "/"; 
const string gIMAPCreateFolderExpected = "CREATE";
const string gIMAPCreateFolderSuccessful = "OK CREATE";
const string gIMAPCreateFolderFailed = "NO";

// Delete folder stuff

const string gIMAPDeleteFolderTag = "a007";
const string gIMAPDeleteFolderCommand = "delete";
const string gIMAPDeleteFolderExpected = "DELETE";
const string gIMAPDeleteFolderSuccessful = "OK DELETE";
const string gIMAPDeleteFolderFailed = "NO";
const string gIMAPDeleteFolderFailedDueToInferior = "inferior";


// Rename folder stuff
const string gIMAPRenameFolderTag = "a008";
const string gIMAPRenameFolderCommand = "rename";
const string gIMAPRenameFolderExpected = "RENAME";
const string gIMAPRenameFolderSuccessful = "OK RENAME";
const string gIMAPRenameFolderFailed = "NO";

// Logout stuff

const string gIMAPLogoutTag = "a099";
const string gIMAPLogoutCommand = "logout";

// Seq2UID Stuff

const string gIMAPSeq2UIDTag = "a009";
const string gIMAPFetchUIDCommand = "fetch";
const string gIMAPUIDMIMEPart = "UID";

// Copy stuff

const string gIMAPCopyTag = "a010";
const string gIMAPCopyCommand = "uid copy";
const string gIMAPCopyExpected = "COPY";
const string gIMAPCopySuccessful = "OK UID RENAME";
const string gIMAPCopyFailed = "NO";
const string gIMAPCopyFailedDueToNoFolder = "TRYCREATE";

const string gIMAPCheckMailTag = "a011";
const string gIMAPCheckMailCommand = "noop";
const string gIMAPCheckMailExpected = "NOOP";


const string gIMAPINBOXName = "INBOX";

const string gIMAPAppendTag = "a012";
const string gIMAPAppendCommand = "APPEND";
const string gIMAPAppendSuccessful = "OK APPEND";
const string gIMAPAppendNoSuchFolder = "TRYCREATE";
const string gIMAPAppendFailed = "NO";\
const string gIMAPAppendExpected = "a012";


#endif
