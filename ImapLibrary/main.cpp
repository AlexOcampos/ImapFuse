/**
 * main.cpp
 * Created by: Alejandro Ocampos
 * Updated: 28/03/2011
 * Description: This is an example of an imap session. Firstly, it logins
 * to server. Secondly, it gets a folder list from the server. After, it
 * selects a folder (the first in previously list), and it gets a email
 * list from that folder. Finally, it gets the header and the body of a
 * email, and logout.
 * 
 * This is an alpha version. There are a lot of mistakes and debug 
 * messages.
 */
#include    <iostream>
#include    <stdlib.h>
#include    <errno.h>
#include    <unistd.h>
#include    <string.h>
#include    <string>
#include    <sys/types.h>
#include 	<list>
#include	"imap.h"
#include	"Folder.h"


ACon *connection;
string imap_server;
int port_n;
string user;
string pass;
bool usessl;
list<Folder> folder_list;	// List of folders
string folder_root;			// Name of de folder root

void add_Folder(Folder &F) {
    F.set_Full_Folder_Name( folder_root + "/" + F.get_Folder_Name() );
    cout << "Folder_name= " << F.get_Folder_Name() << endl;				// DEBUG
    cout << "fullFolder_name= " << F.get_Folder_Name() << endl;			// DEBUG
    folder_list.push_back( F );    
    return;
}

void create_folder_list( list<string> folder_names) {
	Folder *F;
	list<string>::iterator it = folder_names.begin();
	while( it != folder_names.end() )
	{
		if( *it != "\r" ) { // Because of the way the names were parsed, there's a \r in the empty name
			F = new Folder( *it );
			add_Folder( *F );
		}
		it++;
	}
	return;
}

int main( int argc, char *argv[] ) {
	cout << "*** ImapLibrary Test ***" << endl;
	cout << "Connect to imap.gmail.com at port 993..." << endl;
	imap_server 	= "imap.gmail.com";					// Nombre servidor IMAP
	port_n			= 993;								// 143 || 993 con encriptación SSL
	cout << "Write your username: "; cin >> user;		// Usuario
	cout << "Write your pass: "; cin >> pass;			// Password
	usessl			= true;
	int i;
	
	/**
	 * Connect with server and login
	 */
	cout << endl << "********** IMAPLogin **********" << endl;			// DEBUG
	i = IMAPLogin(connection, imap_server, port_n, user, pass, usessl);
	if (i == BAD_LOGIN) {
		printf("Bad login: %d\n", i);
		return -1;
	}
	if (i == SERVER_NOT_READY_FOR_CONNECTION) {
		printf("Server not ready for connection: %d\n", i);
		return -1;
	}
	
	/**
	 * Get folder list from server
	 * - we need to get a list of folder names
	 */
	cout << endl << "********** getIMAPFolders **********" << endl;		// DEBUG
    list<string> folder_names;
    folder_root = ""; // Nombre del buzón de correo que queremos que liste
    int return_code;
    
    return_code = getIMAPFolders(connection, folder_names, folder_root );
    
	cout << endl << "-------------------------------------" << endl;	// DEBUG
	cout << "-----------List of Folders-----------" << endl;			// DEBUG
	cout << "-------------------------------------" << endl;			// DEBUG
	cout << folder_root << "/"<< endl;									// DEBUG
    if( return_code == SUCCESS ) { //  Successful 
        create_folder_list(folder_names);
    }
    
    // DEBUG - Show folder_list
    list<Folder>::iterator itera = folder_list.begin();
    /*while(itera != folder_list.end()) {
		cout << "folder: " << (*itera).get_Folder_Name() << endl;
		cout << "     -> full_name= " << (*itera).get_Full_Folder_Name() << endl;
		cout << " -> seq_start= " << (*itera).get_Sequence_Start() << endl;
		cout << " -> seq_stop = " << (*itera).get_Sequence_Stop() << endl;
		cout << " -> num_mess = " << (*itera).get_Num_Messages() << endl;
		itera++;
	}*/

	/**
	 * Select a folder
	 * - In order to get the emails of a folder, we must select it before.
	 */
	string folderOpenStatus;
	int numMessages;
	itera = folder_list.begin();
	cout << endl << "********** SelectFolders **********" << endl;		// DEBUG
	cout << "Folder name: " << (*itera).get_Folder_Name() << endl;		// DEBUG
	cout << "Full folder name: " << (*itera).get_Full_Folder_Name() << endl;// DEBUG
	i = SelectFolder(connection, (*itera).get_Folder_Name(), numMessages, folderOpenStatus);
	if (i == UNKNOWN_MAILBOX)
		printf("Unknown mailbox - %d\n",i);
	cout << "numMessages = " << numMessages << " |folderOpenStatus = " << folderOpenStatus << endl;// DEBUG

	/**
	 * Get new emails
	 * - We must select a folder before or the function return us a failure
	 */
	printf("\n\n********************************************\n");
	cout << "Folder name: " << (*itera).get_Folder_Name() << endl;
	cout << "Full folder name: " << (*itera).get_Folder_Name() << endl;
	i = GetNewMail(connection, (*itera));
	if (i>BEG_FAIL || i==LIST_FAILED || i== FOLDER_NOT_EXIST) {
		printf("GetNewMail Error - %d\n",i);	
		return -1;
	}
	
	int Message_id = 1;		// Message identifier
	Message *email = (*itera).get_Message(Message_id);		// Para obtener cuerpo de un email a partir de su uid,
															// primero necesitamos coger uno de la lista (orden por id)
	if (email != NULL) {
		cout << "From: " << email->get_From_Address() << endl;			// DEBUG
		cout << "Have body? " << email->have_Body_Text() << endl;		// DEBUG
		cout << "Date: " << email->get_Date() << endl;					// DEBUG
		cout << "Subject: " << email->get_Subject() << endl;			// DEBUG
		cout << "UID: " << email->get_UID() << endl;					// DEBUG
		cout << "-------------------------------" << endl;				// DEBUG
		i=email->get_Body_From_Server(connection);
		cout << "getbodyfromserver=" << i << endl;						// DEBUG
		cout << "Have body? " << email->have_Body_Text() << endl;		// DEBUG
		if (email->have_Body_Text() == 1)
			cout << "BODY: " << email->get_Body_Text();					// DEBUG
	} else
		cout << "The message does not exist." << endl;
	
	/**
	 * Logout
	 * - Finalize the session
	 */
	i = IMAPLogout(connection);
	if (i != SUCCESS) {
		cout << "An error ocurred" << endl;
		return -1;
	}
    
	return 0;
}

