/**
 * basicNetwork.cpp
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Description: Basic IMAP functions
 * Updated: 28/03/2011
 * 
 */

#include	"basicNetwork.h"


string readTilEOL(AConPtr the_connection)
{
  string buffer;
  int i;
  char c;
  i = ReadFromSocket( the_connection, &c, 1 );
  while( i > 0  && c != 10 )
  {
    buffer=buffer+c;
    i = ReadFromSocket( the_connection, &c, 1 );
  }
// mostrar mensaje
    cout << "S: " << buffer << endl;

  return buffer;
}

string readTilEOLTilExpected(AConPtr the_connection, const string &expected1, const string &expected2)
{
  string buffer="";
  int i;
  char c;

 while ((buffer.find(expected1)>=buffer.length())
	&& (buffer.find(expected2)>=buffer.length())  )
 {
   buffer="";
   i = ReadFromSocket( the_connection, &c, 1 );
   while( i > 0  && c != '\n' )
   {
     buffer=buffer+c;
     i = ReadFromSocket( the_connection, &c, 1 );
   }
   // mostrar mensaje
     cout << "S: " << buffer << endl;
   
   
 }
 return buffer;
}

//
// check for errors and stuff... if we get a BYE, stop reading and set
// error to 200. If we get RECENT (we might have new mail) set error to
// 201. (and the other function should probably call the folder.select
// function again, unless that's the function making the request
//


string readTilEOLTilExpected(AConPtr the_connection, const string &expected1, 
	const string &expected2, int &error)
{
string buffer="";
int i;
char c;
error=SUCCESS;

 while (buffer.find(expected1)>=buffer.length()
	&& buffer.find(expected2)>=buffer.length())
 {
   buffer="";
   i = ReadFromSocket( the_connection, &c, 1 );
   while( i > 0  && c != 10 )
   {
	 buffer=buffer+c;
	 i = ReadFromSocket( the_connection, &c, 1 );
   }
   // mostrar mensaje
     cout << "S: " << buffer << endl;
   }
   
   
   if (buffer.find("BYE")<buffer.length())
   {
     error=LOST_CONNECTION_TO_SERVER;
     return "";
   }
   if (buffer.find("RECENT")<buffer.length())
   {
     error=NEW_MAIL;
   }
   
   return buffer;
   
 }

string itoa(const int i)
{
  char ret_val[10];
  
  sprintf(ret_val,"%d",i);
  return ret_val;
  
}













