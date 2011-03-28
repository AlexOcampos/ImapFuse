/**
 * Message.h
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Description: The all important message class.
 * Updated: 22/03/2011
 * 
 */

#ifndef MESSAGE_H
#define MESSAGE_H
namespace std {};
using namespace std;

#include <string>
#include <list>
#include "connection.h"

class Server;
class Folder;

class Message
{
private:
    string to_address;
    string cc_address;
    string from_address;
    string subject;
    string body_text;
    string date;
    string flags;
    int sequence_number;
    int uid;
    int message_id;
    Folder *folder_ptr;
    int have_body;
    list<string> message_parts;
    
public:

    // Constructors
    Message()
        {
            Message( "", "", "", "" );
        };

    Message( string from, string to, string cc, string sub )
        {
            from_address = from;
            subject = sub;
            to_address = to;
            cc_address = cc;
            sequence_number = 0;  // If it's a new message, we don't care what this is
			uid = 0;		  
            message_id = 0;       // Not sure yet whether this should be set or not
            have_body = 0;
			date = "";
        };


    // Mutators
    void set_From_Address( const string &);
    void set_To_Address( const string &);
    void set_CC_Address( const string &);
    void set_Message_Parts( list<string> &);
    void set_Subject( const string &);
    void set_Body_Text( const string &);
    void set_Sequence_Number( int );
    void set_Message_ID( int );
    void set_Folder_Ptr( Folder * );
    void set_Have_Body( int );
    void set_Date( string );    
    void set_Flags( string );
    void set_UID( int );
    void parse_Date( string );
    
    // Accessors
    string get_From_Address();
    string get_To_Address();
    string get_CC_Address();
    list<string> get_Message_Parts();
    string get_Subject();
    string get_Body_Text();
    int get_Sequence_Number();
    int get_Message_ID();
    int get_UID();
    Folder * get_Folder_Ptr();
    int have_Body_Text();
    string get_Date();
    string get_Flags();
    string get_Header();
    string get_Full_Header(AConPtr &the_connection);
    
    // Network-related functions
    int get_Body_From_Server(AConPtr &the_connection);
    int Delete(AConPtr &the_connection);
    int Undelete(AConPtr &the_connection);
    //int save_FCC();
    
    //  Debugging purposes only
    void display();
    //void send();
};

#endif
