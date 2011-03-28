/**
 * imap.cpp
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Description: imap basic functions
 * Caution: Currently only the following functions were tested:
 * IMAPLogin, getIMAPFolders, SelectFolder, GetNewMail, IMAPLogout,
 * GetMessageHeader, GetLineText, GetFlags, tolower 
 * Updated: 23/03/2011
 * 
 */

#include	"imap.h"


//IMAPLogin - logs into the host, returns 0 if successful or 102 if not
int IMAPLogin(AConPtr &the_connection, const string &host, const int port, const string &username, const string &password, bool usessl) {
	string	errcode;
	int err;

	// Connect to port "port" on "host"
	the_connection = MakeConnection(host.c_str(), port, usessl);
	if (!the_connection)
	  return SERVER_NOT_READY_FOR_CONNECTION;
	  
	// Read the response to check if the connection works
	errcode = readTilEOL(the_connection);
	if (errcode.find(gIMAPStarOK)>=errcode.length()) {
		return SERVER_NOT_READY_FOR_CONNECTION;
	}
	
	// Make login command
	string logincommand=gIMAPLoginTag + " " + gIMAPLoginCommand + " \"";
	logincommand += username;
	logincommand += gIMAPQuoteSpaceQuote;
	logincommand += password;
	logincommand += gIMAPQuoteEOL;
	
	// Make noop command
	string noopcommand=gIMAPCheckMailTag + " " + gIMAPCheckMailCommand;
	noopcommand += gIMAPEOL;

	// Send logincommand
	WriteToSocket(the_connection, logincommand.c_str() , logincommand.length());
	
	cout << "C: " << logincommand << endl; // DEBUG
	
	// Make an Ok response in order to check the server response  
	string OKResponse=gIMAPLoginTag + " " + gIMAPOK;

	// Read response if not what we expect, return error code 102
	errcode = readTilEOLTilExpected(the_connection, gIMAPLoginTag, gIMAPLoginExpected, err);
	if (errcode.find(OKResponse)>=errcode.length()) {
		return BAD_LOGIN;
	}
	return SUCCESS;
}

// getIMAPFolders - Get folder listing (in folders). Return SUCCESS or LIST_FAILED on failure.
int getIMAPFolders(AConPtr the_connection, list<string>& folders, string folderroot) {
	string line,foldername;
	int i;

	// form command
	string listcommand = gIMAPListTag + " " + gIMAPListCommand + " " + gIMAPQuoteQuote + " \"" + folderroot + "/" + gIMAPListWildcard + gIMAPQuoteEOL;
	// Send command
	WriteToSocket(the_connection, listcommand.c_str(), listcommand.length());
	
	cout << "C: " << listcommand << endl; // DEBUG

	// Get first response	
	line = readTilEOL(the_connection);

	// while it is giving us list items, process them
	while (line.find(gIMAPListResponse)<line.length()) {
		if (line.find(gIMAPListNoSelect)>=line.length()) {
			i=0;
			int lastslash=line.find(folderroot) + folderroot.length();

			//If folderroot = "" we must look for the names of all folders,
			// which are between " ("folder_name")
			if ((lastslash == 0)&&(folderroot.compare("")==0)) { 
				lastslash = (line.substr(0, line.find_last_of("\"")-1)).find_last_of("\"");
				i = line.find_last_of("/");
				if (i > lastslash)
					lastslash = -1;
			}
		
			i=lastslash;
			if (i != -1) {
				if ((line[i]=='/') || (line[i]== '\"'))
					i++;
				if (line[i]!='.')  {
					foldername="";
					while (line[i] !=0 && line[i] != '\"' && line[i]!='\r') {
						foldername+=line[i];
						i++;
					}
					if (foldername!="")
						folders.push_back(foldername); // add the folder to the list
				}
			}
		}
		line = readTilEOL(the_connection); // read the next line
	}

	// if this isn't what we expect, there might have been a server error. 
	// However, even if we didn't ask for a valid directory it won't complain here.	
	string commandDoneTag = gIMAPListTag + " " + gIMAPOK;

	if (line.find(commandDoneTag)<line.length()) {
		return SUCCESS;
	} else {
		return LIST_FAILED;
	}
}

// Get folder listing
int IMAPExpunge(AConPtr the_connection, list<int>& deleted) {
	string line,foldername,message;
	int i,mess,err;

	// form command
	string expungecommand = gIMAPExpungeTag + " " + gIMAPExpungeCommand + gIMAPEOL;

	// send command
        WriteToSocket(the_connection, expungecommand.c_str(), expungecommand.length());
	// mostrar mensaje
          cout << "C: " << expungecommand << endl;
	// read in response

	line = readTilEOLTilExpected(the_connection,gIMAPExpungeExpected,gIMAPExpungeTag,err);
	if (err==LOST_CONNECTION_TO_SERVER) {return err;}
	// while it is giving us list items, process them
	while (line.find(gIMAPItemMarker)<line.length())
	{
		i=gIMAPExpungeCharsToItem;
		if (line[i]!='.') 
		{
			message="";
			while (line[i] !=' ')
			{
				message+=line[i];
				i++;
			}
			mess = atoi(message.c_str());

			deleted.push_back(mess); // add the folder to the list
		}
	line = readTilEOLTilExpected(the_connection,gIMAPExpungeExpected,gIMAPExpungeTag,err);
	if (err==LOST_CONNECTION_TO_SERVER) {return err;}


	}

	// if this isn't what we expect, there might have been a server error. 
	// However, even if we didn't ask for a valid directory it won't complain here.
	return err;
}

int IMAPGetMessagePartByUID(AConPtr the_connection, int UID, string &messagetext, int part) {
	string line;
	int size=0;
	char c;
	int z=0;
	int err;
	cout << endl << endl << "***************************************" << endl;
	cout << "UID:"<<UID<< "	| PART:" <<part << endl;
	// form command
	//string fetchcommand = gIMAPFetchTag + " " + gIMAPFetchCommand + " " + itoa(UID) + " body[" + itoa(part) + "]"  + gIMAPEOL;
	string fetchcommand = gIMAPFetchTag + " " + gIMAPFetchCommand + " " + itoa(UID) + " body[text]"  + gIMAPEOL;
	// send command
	WriteToSocket(the_connection, fetchcommand.c_str() , fetchcommand.length());
	// mostrar mensaje
	cout << "C: " << fetchcommand << endl;
	// read response
	line = readTilEOLTilExpected(the_connection,gIMAPFetchExpected,gIMAPFetchTag,err);
	if (err==LOST_CONNECTION_TO_SERVER) {return err;}
  if (line.find(gIMAPItemMarker)<line.length()) //check that the message actually exists
    {

      int i=0;
      //go until you pass the flags, if we ever do sub folders, we will need to
      //parse those too.
      while (line[i] != '{' && line[i] != '\"')
	{
	  i++;
	}
      if (line[i] == '\"')
	{
	  messagetext="";
	  line = readTilEOLTilExpected(the_connection, gIMAPFetchTag, gIMAPFetchTag, err);
	  return err;
	}
      // pass over the {
      i++;


      // use temp to store the number of bytes as a string
      string temp;
      while (line[i] != '}')
	{
	  temp += line[i];
	  i++;
	}

      // convert the string to an int
      size=atoi(temp.c_str());
      messagetext="";
      // read size bytes in from the network and put it on message text
      i = ReadFromSocket( the_connection, &c, 1 );
      while( i > 0  && z<size )
	{
	  if (c!='\r')
	    messagetext+=c;
	  /*	  if (gAlthea.get_Verbose()==2) 
		  cout << c;*/
	  i = ReadFromSocket( the_connection, &c, 1 );
	  z++;
	}		
      line = readTilEOLTilExpected(the_connection, gIMAPFetchTag, gIMAPFetchTag, err);
      if (err==LOST_CONNECTION_TO_SERVER) {return err;}


			
    } else {
      // mostrar mensaje
	cout << "in IMAPGetMessagePart line: " << line << endl;
	cout << "does not contain " << gIMAPItemMarker << endl;
      

      return MESSAGE_NOT_THERE; //message doesn't exist
    }

  return err;

}

// get the message body by the UID (this is a different IMAP command than 
// getting the message by the index, so if we do that it will need to be 
// a different command
int IMAPGetMessageBodyByUID(AConPtr the_connection, int UID, string &messagetext) {
	return IMAPGetMessagePartByUID(the_connection, UID, messagetext, 1);
}


// get the message header by the UID (this is a different IMAP command than 
// getting the message by the index, so if we do that it will need to be 
// a different command
int IMAPGetMessageHeaderByUID(AConPtr the_connection, int UID, string &messagetext) {
	string line;
	int size=0;
	char c;
	int z=0;
	int err;
	string sUID = itoa(UID);

	// form command
	string fetchcommand = gIMAPFetchHeaderTag + " " + gIMAPFetchCommand + " " + itoa(UID) + " " + gIMAPHeaderMIMEPart + gIMAPEOL;
	// send command
    WriteToSocket(the_connection, fetchcommand.c_str() , fetchcommand.length());
	
    cout << "C: " << fetchcommand << endl; // DEBUG
    
	// read response
	line = readTilEOLTilExpected(the_connection,gIMAPFetchHeaderTag,gIMAPFetchExpected,err);
	if (err==LOST_CONNECTION_TO_SERVER) {return err;}
	
	if (line.find(gIMAPItemMarker)<line.length()) { //check that the message actually exists
		int i=0;
		//go until you pass the flags, if we ever do sub folders, we will need to
		//parse those too.
		while (line[i] != '{') {
			i++;
		}
		// pass over the {
		i++;


		// use temp to store the number of bytes as a string
		string temp;
		while (line[i] != '}') {
			temp += line[i];
			i++;
		}

		// convert the string to an int
		size=atoi(temp.c_str());
		messagetext="";
		// read size bytes in from the network and put it on message text
	    i = ReadFromSocket( the_connection, &c, 1 );
       	while( i > 0  && z<size ) {
			messagetext+=c;
			i = ReadFromSocket( the_connection, &c, 1 );
			z++;
        }

		line = readTilEOLTilExpected(the_connection,gIMAPFetchHeaderTag,gIMAPFetchHeaderTag,err);
		if (err==LOST_CONNECTION_TO_SERVER) 
		  return err;
		

		
	} else {
	  // mostrar mensaje
	    cout << "in IMAPGetMessageHeader line: " << line << endl;
	    cout << "does not contain " << gIMAPItemMarker << endl;
	  return MESSAGE_NOT_THERE; //message doesn't exist
	}
	return err;
}

// get the Set flags by the UID (this is a different IMAP command than 
// getting the message by the index, so if we do that it will need to be 
// a different command
int IMAPSetFlagByUID(AConPtr the_connection, int UID, const string &flagtoset, string &currentflags) {
	string line;
	int err;
	string newflagtoset;
	if (flagtoset=="")
		newflagtoset="\\Seen";
	else
		newflagtoset=flagtoset;

	// form command
	string fetchcommand = gIMAPSetFlagTag + " " + gIMAPStoreCommand + " " + itoa(UID) + " " + gIMAPSetFlagCommand + " (" + newflagtoset + ")" + gIMAPEOL; 


	// send command
	WriteToSocket(the_connection, fetchcommand.c_str() , fetchcommand.length());
	
	cout << "C: " << fetchcommand << endl;	// DEBUG
	
	// read response
	line = readTilEOLTilExpected(the_connection, gIMAPFetchExpected, gIMAPSetFlagTag, err);
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}

	if (line.find(gIMAPItemMarker)<line.length()) { //check that the message actually exists
		int i=0;
		//go until you pass the message id
		while (line[i] != '(') {
			i++;
		}
		i++;
		//go until you pass SOMETHING ELSE
		while (line[i] != '(') {
			i++;
		}
		// pass over the {
		i++;
		
		// use temp to store the number of bytes as a string
		string temp;
		while (line[i] != ')') {
			currentflags += line[i];
			i++;
		}

		line = readTilEOLTilExpected(the_connection,gIMAPSetFlagTag,gIMAPSetFlagTag,err);
		if (err==LOST_CONNECTION_TO_SERVER) {
			return err;
		}			
    } else {
		if (line.find(gIMAPOK)<line.length()) // Cyrus doesn't seem to conform to the RFC and doesn't return the new flags
			currentflags += " " + flagtoset;
		else { // we didn't get back either OK or the new flags
		// mostrar mensaje
		cout << "in IMAPSetFlag line: " << line << endl;
		cout << "does not contain " << gIMAPItemMarker << endl;
		return MESSAGE_NOT_THERE; //message doesn't exist
      }
    }
  return err;
}

// get the unSet flags by the UID (this is a different IMAP command than 
// getting the message by the index, so if we do that it will need to be 
// a different command
int IMAPUnSetFlagByUID(AConPtr the_connection, int UID, const string &flagtounset, string &currentflags) {
	string line;
	int err;

	// form command
	string fetchcommand = gIMAPSetFlagTag + " " + gIMAPStoreCommand + " " + itoa(UID) + " " + gIMAPUnSetFlagCommand + " (" + flagtounset + ")" + gIMAPEOL;
	// send command
    WriteToSocket(the_connection, fetchcommand.c_str() , fetchcommand.length());
	
    cout << "C: " << fetchcommand << endl; // DEBUG
    
	// read response
	line = readTilEOLTilExpected(the_connection, gIMAPFetchExpected, gIMAPSetFlagTag, err);
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}

	if (line.find(gIMAPItemMarker)<line.length()) { //check that the message actually exists
		int i=0;
		//go until you pass the message id
		while (line[i] != '(') {
			i++;
		}
		i++;
		//go until you pass SOMETHING ELSE
		while (line[i] != '(') {
			i++;
		}
		// pass over the {
		i++;
		
		// use temp to store the number of bytes as a string
		string temp;
		while (line[i] != ')') {
			currentflags += line[i];
			i++;
		}

		line = readTilEOLTilExpected(the_connection,gIMAPSetFlagTag,gIMAPSetFlagTag,err);
		if (err==LOST_CONNECTION_TO_SERVER) {
			return err;
		}		
	} else {
		return MESSAGE_NOT_THERE; //message doesn't exist
	}
	return err;
}

//
// Create a folder
// 
int IMAPCreateFolder(AConPtr the_connection, const string &foldername, const string &folderroot) {
	string line;
	int err;

	// form command
	string createcommand = gIMAPCreateFolderTag + " " + gIMAPCreateFolderCommand + " " + folderroot + gIMAPDirectoryIndicator + foldername + gIMAPEOL;
	
	// send command
	WriteToSocket(the_connection, createcommand.c_str() , createcommand.length());
	
    cout << "C: " << createcommand << endl; // DEBUG
    
	// read response
	line = readTilEOLTilExpected(the_connection, gIMAPCreateFolderExpected, gIMAPCreateFolderTag, err);
	
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}

	if (line.find(gIMAPCreateFolderSuccessful)>=line.length()) {
		return err;		
	} else if (line.find(gIMAPCreateFolderFailed)>=line.length()) { 
		return CREATE_FOLDER_FAILED;
	} else	{
		return CREATE_FOLDER_FAILED;
	}
}

//
// Delete a folder
// 
int IMAPDeleteFolder(AConPtr the_connection, const string &foldername, const string &folderroot) {
	string line;
	int err;
	if (foldername != gIMAPINBOXName) {
		// form command
		string command = gIMAPDeleteFolderTag + " " + gIMAPDeleteFolderCommand + " " + folderroot + gIMAPDirectoryIndicator + foldername + gIMAPEOL;
		
		// send command
		WriteToSocket(the_connection, command.c_str() , command.length());
		
		cout << "C: " << command << endl; // DEBUG
		
		// read response
		line = readTilEOLTilExpected(the_connection,gIMAPDeleteFolderExpected,gIMAPDeleteFolderTag,err);
		if (err==LOST_CONNECTION_TO_SERVER) {
			return err;
		}

		if (line.find(gIMAPDeleteFolderSuccessful)>=line.length()) {
			return err;		
		} else if (line.find(gIMAPDeleteFolderFailedDueToInferior)>=line.length()) { 
			return DELETE_FAILED_DUE_TO_INFERIOR;
		} else if (strstr(line.c_str(),gIMAPDeleteFolderFailed.c_str())!=NULL){
			return DELETE_FAILED;
		} else {
			return DELETE_FAILED;
		}
	} else {
		return NO_DELETE_INBOX;
	}
}

//
// Rename a folder
// 
int IMAPRenameFolder(AConPtr the_connection, const string &fromname, const string &toname, const string &folderroot) {
	int err;
	string line;
	string command="";
	if (fromname==gIMAPINBOXName) {
		// form command
		command = gIMAPRenameFolderTag + " " + gIMAPRenameFolderCommand + " " + fromname + " " + folderroot + gIMAPDirectoryIndicator + toname + gIMAPEOL;
	} else {
		// form command
		command = gIMAPRenameFolderTag + " " + gIMAPRenameFolderCommand + " " + folderroot + gIMAPDirectoryIndicator + fromname + " " + folderroot + gIMAPDirectoryIndicator + toname + gIMAPEOL;
	}
  
	// send command
	WriteToSocket(the_connection, command.c_str() , command.length());
	
	cout << "C: " << command << endl; // DEBUG
	
	// read response
	line = readTilEOLTilExpected(the_connection,gIMAPRenameFolderExpected,gIMAPRenameFolderTag,err);
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}
  
	if (line.find(gIMAPRenameFolderSuccessful)>=line.length()) {
		return err;		
	} else if (line.find(gIMAPRenameFolderFailed)>=line.length()){
		return RENAME_FAILED;
	} else {
		return RENAME_FAILED;
	}
}

int IMAPLogout(AConPtr the_connection) {
	// form command
	string logoutcommand = gIMAPLogoutTag + " " + gIMAPLogoutCommand + gIMAPEOL;
	// send command
	WriteToSocket(the_connection, logoutcommand.c_str() , strlen(logoutcommand.c_str()));
	
	cout << "C: " << logoutcommand << endl; // DEBUG
	
	return SUCCESS;
}

int IMAPSeq2UID(AConPtr the_connection, const int seq, int &UID) {
	string line;
	string command="";
	int i,err;

	// form command
	command = gIMAPSeq2UIDTag + " " + gIMAPFetchUIDCommand + " " + itoa(seq) + " " + gIMAPUIDMIMEPart + gIMAPEOL; 

	// send command
	WriteToSocket(the_connection, command.c_str() , strlen(command.c_str()));
	
	cout << "C: " << command << endl; // DEBUG
	
	// read response
	line = readTilEOLTilExpected(the_connection,gIMAPFetchExpected,gIMAPSeq2UIDTag,err);
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}
	if (line.find("*")<line.length()) {
		i=2;
		// use temp to store the number of bytes as a string
		while (line[i] != '(') {
			i++;
		}
		i=i+5;
		string temp="";

		while (line[i] != ')') {
			temp += line[i];
			i++;
		}
		UID=atoi(temp.c_str());
		line = readTilEOLTilExpected(the_connection,gIMAPSeq2UIDTag,gIMAPSeq2UIDTag,err);
		if (err==LOST_CONNECTION_TO_SERVER) {
			return err;
		}
		return err;
	} else {
		return MESSAGE_NOT_THERE;
	}
}

int IMAPCopyMessage(AConPtr the_connection, const int uid, const string &tofolder, const string &folderroot) {
	int err;
	string line,command;

	if (tofolder == gIMAPINBOXName) { // form command
		command = gIMAPCopyTag + " " + gIMAPCopyCommand + " " + itoa(uid) + " " + tofolder + gIMAPEOL; 
	} else { // not INBOX - form command
		command = gIMAPCopyTag + " " + gIMAPCopyCommand + " " + itoa(uid) + " " + folderroot + gIMAPDirectoryIndicator + tofolder + gIMAPEOL; 
	}

	// send command
	WriteToSocket(the_connection, command.c_str() , strlen(command.c_str()));

	cout << "C: " << command << endl; // DEBUG
	
	// read response
	line = readTilEOLTilExpected(the_connection,gIMAPCopyExpected,gIMAPCopyTag,err);
	
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}
	if (strstr(line.c_str(),gIMAPCopySuccessful.c_str())!=NULL) {
		return err;		
	} else if (strstr(line.c_str(),gIMAPCopyFailedDueToNoFolder.c_str())!=NULL) { 
		return COPY_FAILED_DUE_TO_NO_FOLDER;
	} else if (strstr(line.c_str(),gIMAPCopyFailed.c_str())!=NULL){
		return COPY_FAILED;
	} else {
		return COPY_FAILED;
	}
}

int IMAPCheckMail(AConPtr the_connection) {
	int err;
	string command = gIMAPCheckMailTag + " " + gIMAPCheckMailCommand + gIMAPEOL;
	
	// send command
	WriteToSocket(the_connection, command.c_str() , strlen(command.c_str()));
	
	cout << "C: " << command << endl; // DEBUG
	
	string line = readTilEOLTilExpected(the_connection,gIMAPCheckMailTag,gIMAPCheckMailExpected,err);
	return err;
}

// append posts the message to the folder
int IMAPAppend(AConPtr the_connection, const string &text, const string &folder, const string &folderroot) {
	string line;
	int err;
	
	// form command
	string command = gIMAPAppendTag + " " + gIMAPAppendCommand + " " + folderroot +    "/" + folder + " (\\Seen) {"; 
	command = command + itoa(strlen(text.c_str())) + "}" + gIMAPEOL;
	
	// send command
	WriteToSocket(the_connection, command.c_str() , strlen(command.c_str()));

	cout << "C: " << command << endl; // DEBUG
		  
	// read response
	WriteToSocket(the_connection, text.c_str(), strlen(text.c_str()));
	WriteToSocket(the_connection, gIMAPEOL.c_str(), strlen(gIMAPEOL.c_str()));

	line = readTilEOLTilExpected(the_connection,gIMAPAppendExpected,gIMAPAppendTag,err);
	if (err==LOST_CONNECTION_TO_SERVER) {
		return err;
	}

	if (strstr(gIMAPAppendSuccessful.c_str(),line.c_str())==0) {
	  return err;
	}

	if (strstr(gIMAPAppendNoSuchFolder.c_str(),line.c_str())==0) {
	  return APPEND_FAILED_DUE_TO_NO_FOLDER;
	}

	if (strstr(gIMAPAppendFailed.c_str(),line.c_str())==0) {
	  return APPEND_FAILED;
	}
	
	return err;
}

// SelectFolder - Select a folder and return the number of messages in the folder (numMessages) and the status
// of the folder (folderOpenStatus). Return SUCCESS or UNKNOWN_MAILBOX on failure.
int SelectFolder(AConPtr the_connection, const string &foldername, int &numMessages, string &folderOpenStatus)  {
	string command;
	string response;
	string tmpString;
	int startEXISTS;
	int begStatus;
	int endStatus;
	int lengthStatus;
	int errnum=SUCCESS;

	// make command
	command = "a003 SELECT \"" + foldername + "\"\r\n";

	// send command to server
	WriteToSocket(the_connection, command.c_str(), strlen(command.c_str()));

	cout << "C: " << command << endl; // DEBUG

	// read response
	do {
		response = readTilEOL(the_connection);
		if (((int)response.find("NONEXISTENT")) != -1) {
			folderOpenStatus = "Open Failed";
			numMessages = -1;
			return (errnum=UNKNOWN_MAILBOX);
		}
			
		// right now we only really care about how many messages are in the mailbox 
		if (strstr(response.c_str()," EXISTS"))  {
			// we know that the message is in the form
			// * <SPACE> int <SPACE> EXISTS
			// -- must find out where that int ends, which is posIndex for the start of exists - 1
			startEXISTS = response.find_first_of(" EXISTS");

			// find the int substring and convert it to type int
			tmpString = response.substr(2,startEXISTS-3);
			numMessages = atoi(tmpString.c_str());
		}
	}  while ((!strstr(response.c_str(),"a003 OK ")) && (errnum<END_SUCCESS)); 

	// the final response told us our read/write status for this folder
	// or if the folder open was unsuccessful, some info we don't want.  check it first
	if (errnum<END_SUCCESS)  {
		begStatus = response.find("[")+1;
		endStatus = response.find("]");
		lengthStatus = endStatus-begStatus;
		folderOpenStatus = response.substr(begStatus,lengthStatus);
	} else  {
		folderOpenStatus = "Open Failed";
	}

	return(errnum);
}

/**
 *	GetNewMail
 * 1. Figure out how many messages are in the folder. (The folder must be selected previously with "SelectFolder()") 
 * 2. Create a list in "folder" with all the messages in it.
 * Return: SUCCESS, FOLDER_NOT_EXIST, LIST_FAILED or other errors (>BEG_FAIL)
 */
int GetNewMail(AConPtr the_connection, Folder &folder)  {
	Message message;
	string command;
	string response;
	string tmpString;
	string to,from,subject,date,cc;
	int uid;
	string flags;
	int startEXISTS;
	int highNumMessages;
	int errnum=SUCCESS;
	int msgCounter;

	// Figure out how many messages are in the folder
	command = "a023 STATUS " + folder.get_Folder_Name() + " (MESSAGES)\r\n";
	WriteToSocket(the_connection,command.c_str(),strlen(command.c_str()));
  
	cout << "C: " << command << endl; // DEBUG

	// read response and put in highNumMessages the number of messages in the folder
	do {
		response = readTilEOL(the_connection);
		if (errnum>BEG_FAIL) {
			return(errnum);
		}
		if (response.find("NONEXISTENT") != string::npos) {
			return (FOLDER_NOT_EXIST);
		}
		if (strstr(response.c_str(),"(MESSAGES"))  {
		  // we know that the message is in the form (MESSAGES N)
		  // -- must find out where that int ends, which is posIndex for the start of 
		  // exists - 1
		  startEXISTS = response.find("(MESSAGES");
		  // find the int substring and convert it to type int
		  tmpString = response.substr(startEXISTS+10,response.find_last_of(")")-(startEXISTS+10));
		  highNumMessages = atoi(tmpString.c_str());
		}
	}  while ((!strstr(tolower(response).c_str(),"a023 ok")) && (errnum<END_SUCCESS)); 
	
	cout << "highNumMessages = " << highNumMessages << endl; // DEBUG

/* ESTO ES NECESARIO???????
	// loop through and start picking up message headers, adding a new message instance to
	// our folder with each iteration compose our command
	if (folder.get_Sequence_Stop()+1 <=highNumMessages) {
      errnum = SelectFolder(the_connection,folder.get_Full_Folder_Name(),numMessages,folderOpenStatus);
      
      int nextuid;
      if (folder.get_Sequence_Stop()!=0)
	nextuid=folder.get_Message(folder.get_Sequence_Stop())->get_UID()+1;
      else
	nextuid=1;*/
	
	int nextuid = 1;
	command = "A3 FETCH " + itoa(nextuid) + ":" + itoa(highNumMessages) + " (UID BODY[HEADER])\r\n";
	
    // get all of our header information from the Fetch BODY[header] command           
	cout << "C: " << command << endl;
	
	// send command to server
	WriteToSocket(the_connection,command.c_str(),strlen(command.c_str()));

	for (msgCounter=1; msgCounter<=highNumMessages; msgCounter++)  {
		// get the headers
		list<string> msgparts;
		string flags;
		errnum = GetMessageHeader(the_connection,msgCounter,to,cc,from,subject,uid,date,flags,msgparts);

		if (errnum>BEG_FAIL || errnum==LIST_FAILED)
			return(errnum);

		// insert the stuff into out message
		message.set_From_Address(from);
		message.set_Subject(subject);
		message.set_To_Address(to);
		message.set_CC_Address(cc);
		message.parse_Date( date );
		message.set_Flags( flags );
		message.set_Have_Body( 0 );

	 
		// this is not the uid here.
		// it is the relative msg number/sequence id
		message.set_Message_ID(msgCounter);
		message.set_UID(uid);
		message.set_Sequence_Number(msgCounter);	 
		message.set_Flags(flags);  
		message.set_Message_Parts(msgparts);	 
		 
		// add the message to our folder
		folder.add_Message(message);
	}
     
  return(errnum);
}

int GetMessageHeader (AConPtr the_connection, int msgNumber, string &to, string &cc, string &from, string &subject, int &uid,
		      string & date,string & flags,list<string> & msgparts)  {
	// defs
	string msgStringCounter;
	string command;
	string header;
	string bodystructure;
	string response;
	string tmpHeader;
	string tmpLength;
	int length;      
	int end;
	int beg;
	int headerCounter;
	int i;
	int errnum=SUCCESS;
	char c;

	response="";
	while (response.find("{")>=response.length()) {
		response = readTilEOL(the_connection);   
		if (((int)response.find("BAD FETCH")) != -1)
			return (LIST_FAILED);  
		if (errnum>BEG_FAIL)
		  return(errnum);
	}
	beg = response.find("UID")+4;
	end = response.find(" ",beg);
	uid=atoi(response.substr(beg,end-beg).c_str());

	// the first line coming back will have the length of the rest of our info
	beg = response.find("{") + 1;
	end = response.find("}");
	tmpLength = response.substr(beg,end-beg);
	length = atoi(tmpLength.c_str());

	// start reading in the rest of the header
	header = "";
	i = 1;
	//cout << "getMessageHeader: " <<endl;
	for (headerCounter=0; headerCounter<=length && i>0; headerCounter++)  {
		i = ReadFromSocket(the_connection,&c,1);
		header += c;
	}


  // get the date
  GetLineText(header,"Date:",date);

  // get sender (from)
  GetLineText(header,"From:",from);

  // get subject
  GetLineText(header,"Subject:",subject);
      
  // get who it is to
  GetLineText(header,"To:",to);

  // get who it is to
  GetLineText(header,"Cc:",cc);

  //Adding for case insensitivity
  if (cc=="")
    GetLineText(header,"CC:",cc);
  if (cc=="")
    GetLineText(header,"cc:",cc);



  flags=GetFlags(response, msgNumber, errnum, msgparts);

  return(errnum);
}

string tolower(string s) {
  string lowerstring="";
  for (unsigned int x=0; x<s.length(); x++) {
    lowerstring+=tolower(s[x]);
  }
  return(lowerstring);  
}

void GetLineText(const string &haystack, const string &keyword, string &value)  {
  // defs
  unsigned int afterKeyword;
  value = "";
  
  // figure it out
  if (strstr(haystack.c_str(),keyword.c_str())!=NULL)  {
    afterKeyword = haystack.find(keyword) + keyword.length();
    for (; !(haystack[afterKeyword]=='\n' && haystack.substr(afterKeyword-3,3).find(",")>haystack.substr(afterKeyword-3,3).length()) && afterKeyword<=haystack.length(); 
	 afterKeyword++)
      if (haystack[afterKeyword]!='\n' && haystack[afterKeyword]!='\r')
	value += haystack[afterKeyword];
  }
  else
    value = "";
  
}

string GetFlags(string response, int msgNumber, int &errnum, list<string> &parts)  {
  // defs
  // string response;
  string flags;
  string command;
  string msgStringCounter;

  // remove FLAGS parens
  EraseBefore(response,"FLAGS"); 
  EraseBefore(response,"("); 
  flags = response.substr(0,response.find(")"));
  EraseBefore(response,"("); 
  string bodystructure;
  if (response[0]!='(')
    bodystructure="("+response;
  else 
    bodystructure=response;

  parts.clear();

  string part="";
  int parencount=0;
  int numpts=0;


  for (unsigned int x=0; x<bodystructure.length()-1; x++) {
    if (bodystructure[x]=='(')
      parencount++;
    if (bodystructure[x]==')') 
      parencount--;
    part+=bodystructure[x];
     
    if (parencount==0) {       
      numpts++;
      parts.push_back(part);
      part="";
      // if there isn't another part after this -- the x<bla will shortcut if this would go over the end of the string
      if (x<bodystructure.length()-1 && bodystructure[x+1]!='(') 
	x=bodystructure.length(); // we are done
    }
  }
   
  return(flags);
}

void EraseBefore(string &haystack, const string &needle)  {
  haystack.erase(0,haystack.find(needle)+1);
}
