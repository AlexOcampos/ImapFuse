/**
 * @file Message.cpp
 * @author Alejandro Ocampos Veiga
 * @brief Functions of Message class. Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * @date 30/03/2011
 */

#include <iostream>
#include <string>
#include <pthread.h>
#include "Message.h"
#include "imap.h"
#include "errors.h"


void Message::set_From_Address( const string &new_address )
{
    from_address = new_address;
}

void Message::set_To_Address( const string &new_address )
{
    to_address = new_address;
}

void Message::set_CC_Address( const string &new_address )
{
    cc_address = new_address;
}

void Message::set_Subject( const string &new_subject )
{
    subject = new_subject;
}

void Message::set_Body_Text( const string &new_body )
{
    body_text = new_body;
}

void Message::set_Sequence_Number( int new_sequence )
{
    sequence_number = new_sequence;
}

void Message::set_UID( int new_UID )
{
    uid = new_UID;
}

void Message::set_Message_Parts( list<string> & msgParts )
{
  message_parts=msgParts;
}


void Message::set_Message_ID( int new_id )
{
    message_id = new_id;
}

void Message::set_Have_Body( int i )
{
    have_body = i;
}

void Message::set_Date( string s )
{
   date = s;
}

void Message::set_Flags( string s )
{
    flags = s;
}

void Message::parse_Date( string s )
{
    string day;
    string month;
    string year;
    string time;
    
    // Get rid of extra garbage
    s = s.substr( s.find_first_of( ',' ) +2);
    s = s.substr( 0, s.find_last_of( ':' ));

    // Grab each field
    day = s.substr(0, s.find_first_of(' '));
    s = s.substr( s.find_first_of(' ')+1);

    month = s.substr(0, s.find_first_of(' '));
    s = s.substr( s.find_first_of(' ')+1);

    year = s.substr(0, s.find_first_of(' '));
    s = s.substr( s.find_first_of(' ')+1);

    time = s;   

    // Some dates are just 3, not 03
    if( day.size() == 1 )
        day = "0"+day;

    if( month == "Jan" )
        month = "01";
    else if( month == "Feb" )
        month = "02";
    else if( month == "Mar" )
        month = "03";
    else if( month == "Apr" )
        month = "04";
    else if( month == "May" )
        month = "05";
    else if( month == "Jun" )
        month = "06";
    else if( month == "Jul" )
        month = "07";
    else if( month == "Aug" )
        month = "08";
    else if( month == "Sep" )
        month = "09";
    else if( month == "Oct" )
        month = "10";
    else if( month == "Nov" )
        month = "11";
    else if( month == "Dec" )
        month = "12";

    set_Date( month + "/" + day + "/" + year + " " + time ); 

}

string Message::get_From_Address()
{
    return( from_address );
}

string Message::get_To_Address()
{
    return( to_address );
}

string Message::get_CC_Address()
{
    return( cc_address );
}

string Message::get_Subject()
{
    return( subject );
}

string Message::get_Body_Text()
{
    return( body_text );
}

int Message::get_Sequence_Number()
{
    return( sequence_number );
}

int Message::get_UID()
{
    return( uid );
}

int Message::get_Message_ID()
{
    return( message_id );
}

list<string> Message::get_Message_Parts()
{

  return(message_parts);

}

int Message::get_Body_From_Server(AConPtr &the_connection)
{
    int return_code;
    string body;
    return_code = IMAPGetMessageBodyByUID(the_connection , uid, body );
    set_Body_Text( body );
    set_Have_Body( 1 );
	
    if (return_code <BEG_FAIL)
      return_code = IMAPSetFlagByUID(the_connection , uid, "", flags );


    return( return_code );
}

/*int Message::save_FCC()
{
  string fccfolder = get_Server_Ptr()->get_FCC_Folder();
  if ((fccfolder != "NONE") && (fccfolder != "none") && (fccfolder != "None")) 
    {
      
      int return_code;
      
      string message = get_Header() + "\r\n\r\n" + get_Body_Text();
      
      return_code = IMAPAppend(get_Server_Ptr()->get_Connection(), message, fccfolder, get_Server_Ptr()->get_Folder_Root());
      
      return( return_code );
    }
  return SUCCESS;
}*/

string Message::get_Header()
{
    string tmp;
    tmp = "From: " + get_From_Address() + "\n";
    tmp += "To: " + get_To_Address() + "\n";
    if (get_CC_Address()!="")
      tmp += "CC: " + get_CC_Address() + "\n";
    tmp += "Date: " + get_Date() + "\n";
    tmp += "Subject: " + get_Subject() + "\n\n";

    return( tmp );
}

string Message::get_Full_Header(AConPtr &the_connection)
{
    string tmp;
    IMAPGetMessageHeaderByUID(the_connection, uid, tmp);
    return( tmp );
}

int Message::Delete(AConPtr &the_connection)
{
    int return_code;
    string currentflags;
    return_code = IMAPSetFlagByUID( the_connection, uid, "\\Deleted",currentflags );
    set_Flags( currentflags );
    
    return( return_code );
}

int Message::Undelete(AConPtr &the_connection)
{
    int return_code;
    string currentflags;
    return_code = IMAPUnSetFlagByUID( the_connection, uid,"\\Deleted",currentflags );
    set_Flags( currentflags );

    return( return_code );
}

int Message::have_Body_Text()
{
    return( have_body );
}
        
string Message::get_Date()
{
    return( date );
}

string Message::get_Flags()
{
    return( flags );
}

//  Debugging purposes only
void Message::display()
{
    cout << "        Message # " << sequence_number << endl
        <<"         From   : " << from_address << endl
        << "         To     : " << to_address << endl
	<< "	Date: " << date << endl
        << "         Subject: " << subject << endl << endl;
}

/*void Message::send()
{
  pthread_t thread;
  int rc;
  
  rc = pthread_create(&thread, NULL, SendMail, (void *)this);

  //  SendMail( this );
}*/

