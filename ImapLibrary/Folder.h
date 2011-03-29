/**
 * Folder.h
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Description: Folder definitions
 * Updated: 28/03/2011
 * 
 */

#ifndef FOLDER_H
#define FOLDER_H
namespace std {};
using namespace std;

#include <string>
#include <list>
#include "Message.h"

class Folder {
private:
    string folder_name;
    string full_folder_name;
    list<Message> message_list;
    int sequence_start;
    int sequence_stop;
    Server *server_ptr;

public:

    // Constructors
    Folder() {
        Folder("New Folder");
    };

    Folder(string name) {
        folder_name = name;
        sequence_start = 0;
		sequence_stop = 0;
    };


    // Mutators
    void set_Folder_Name(const string &name);
    void set_Sequence_Start(int new_start);
    void set_Sequence_Stop(int new_stop);
    void set_Server_Ptr(Server *);    
    void set_Full_Folder_Name(const string &name);
    
    // Accessors
    string get_Folder_Name();
    int get_Num_Messages();
    int get_Sequence_Start();
    int get_Sequence_Stop();
    Server * get_Server_Ptr();
    Message * get_Message(int);
    string get_Full_Folder_Name();

    // Message-list-related functions
    void add_Message(Message);
    void delete_Message(int);              // Delete based on message-ID??
    int get_Message_Headers();
    int expunge_Deleted_Mail();
    void clear();
    
    // Debugging purposes only
    void display();
};

#endif