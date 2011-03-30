/**
 * @file error.h
 * @author Alejandro Ocampos Veiga
 * @brief sSet an array of errornumbers and what they mean. Adapted from
 * Althea Project by Alejandro Ocampos for ImapFuse.
 * @date 30/03/2011
 */

#ifndef ERRORS_H
#define ERRORS_H
namespace std {};
using namespace std;

// define the number of errors we are going to keep 
// track of in this array

enum errors {

  BEG_SUCCESS,

  // general
  SUCCESS,

  // fun server news
  NEW_MAIL,
  SUCCESSFUL_SERVER_READ,


  //IMAP Stuff
  END_SUCCESS,

  BEG_FAIL,


  // errors sending mail
  TO_NOT_SPECIFIED,
  SENDER_NOT_SPECIFIED,

  // server problems
  LOST_FOLDER_LOCK,
  LOST_CONNECTION_TO_SERVER,
  COMMAND_TO_SERVER_FAILED,
  COMMAND_OR_ARGUMENTS_INVALID,

  //Login related
  SERVER_NOT_READY_FOR_CONNECTION,
  BAD_LOGIN,


  //LIST folders
  LIST_FAILED,

  // get message, get header, set flag, seq2uid
  MESSAGE_NOT_THERE,
   
  // create folder
  CREATE_FOLDER_FAILED,

  // delete folder
  DELETE_FAILED_DUE_TO_INFERIOR, //there are files/directories in the folder
  DELETE_FAILED,
  NO_DELETE_INBOX, //you shouldn't delete the INBOX
  
  // rename
  RENAME_FAILED,
   
  // copy
  COPY_FAILED_DUE_TO_NO_FOLDER,
  COPY_FAILED,

  // append
  APPEND_FAILED_DUE_TO_NO_FOLDER,
  APPEND_FAILED,

  END_FAIL,
  
  // otros
  UNKNOWN_MAILBOX,
  FOLDER_NOT_EXIST
};

#endif
