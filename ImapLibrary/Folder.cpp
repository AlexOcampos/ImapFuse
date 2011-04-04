/**
 * @file Folder.h
 * @author Alejandro Ocampos Veiga
 * @brief Folder class functions. Adapted from Althea Project by Alejandro Ocampos for ImapFuse.
 * @date 23/03/2011
 */

#include <iostream>
#include <string>
#include <list>
#include "Folder.h"
#include "imap.h"
#include "errors.h"

void Folder::set_Folder_Name(const string &name) {
    folder_name = name;
}

void Folder::set_Sequence_Start(int new_start) {
    sequence_start = new_start;
}

void Folder::set_Sequence_Stop(int new_stop) {
    sequence_stop = new_stop;
}

void Folder::set_Server_Ptr( Server * s_ptr )
{
    server_ptr = s_ptr;
}

void Folder::set_Full_Folder_Name(const string &name) {
    full_folder_name = name;
}


string Folder::get_Folder_Name() {
    return(folder_name);
}

int Folder::get_Num_Messages() {
    return(message_list.size());
}

int Folder::get_Num_subFolders() {
	return(subfolder_list->size());
}

int Folder::get_Sequence_Start() {
    return(sequence_start);
}

int Folder::get_Sequence_Stop() {
    return(sequence_stop);
}

Server * Folder::get_Server_Ptr() {
    return server_ptr;
}

Folder * Folder::get_subFolder( int n ) {
    list<Folder>::iterator it;
    int count = 1;
    
    it = subfolder_list->begin();
	if (n < 0)
		return NULL;	// The subfolder does not exist
    while( it != subfolder_list->end() && count < n ) {
        count++;
        it++;
    }
    if(count <= get_Num_subFolders())
        return(&( *it ));	// The subfolder does not exist
    else
        return(NULL);
}

Message * Folder::get_Message( int n ) {
    list<Message>::iterator it;
    int count = 1;
    
    it = message_list.begin();
	if (n < 0)
		return NULL;	// The message does not exist
    while( it != message_list.end() && count < n ) {
        count++;
        it++;
    }
    if(count <= get_Num_Messages())
        return(&( *it ));	// The message does not exist
    else
        return(NULL);
}

string Folder::get_Full_Folder_Name() {
    return(full_folder_name);
}

void Folder::add_subFolder(Folder F) {
	subfolder_list->push_back( F );
}

void Folder::add_Message(Message M) {
    message_list.push_back( M );
    if( message_list.size() == 1 )
        sequence_start = M.get_Sequence_Number();
    sequence_stop = M.get_Sequence_Number();
}

void Folder::delete_Message(int ID) {
    list<Message>::iterator it;
    it = message_list.begin();
    while(it != message_list.end() && (*it).get_Message_ID() != ID) {
        it++;
    }
    if((*it).get_Message_ID() == ID)
        message_list.erase(it);
}

int Folder::get_Message_Headers() {
    //return( GetMessageHeaders( get_Server_Ptr()->get_Connection(), *this ) );
    return 0;
}

/*int Folder::expunge_Deleted_Mail( )
{
    list<int> deleted_list;

    int ret_val= IMAPExpunge( get_Server_Ptr()->get_Connection(), deleted_list );
    

    for (list<int>::iterator it = deleted_list.begin();
	 it != deleted_list.end();
	 it++) {
      list<Message>::iterator mit;
      mit = message_list.begin();
      while( mit != message_list.end() && (*mit).get_Message_ID() != (*it) )
      {
	mit++;
      }

      mit=message_list.erase(mit );
      int counter=0;
      for (int x = 1; x<get_Sequence_Stop(); x++) {
	Message *M= get_Message(x);
		if (M!=NULL) {
		  counter++;
		  M->set_Message_ID(counter);
		} 
      }
      set_Sequence_Stop( counter );
    }
    return ret_val;
}
    
*/    

void Folder::clear() {
    message_list.clear();
}


//  Debugging purposes only
void Folder::display() {
    list<Message>::iterator it;
    
    cout << endl << "   +" << folder_name << endl
        << "     " << message_list.size() << " messages" << endl
        << "     from " << sequence_start << " to " << sequence_stop << endl << "     |" << endl
        << "     Message Info" << endl;

    it = message_list.begin();
    while(it != message_list.end()) {
        (*it).display();
        it++;
    }  
}
