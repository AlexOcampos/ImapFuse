/**
 * tcpUtilities.cpp
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Althea description: "Adapted in C from Douglas Comer's 
 * 	'Internetworking with TCP/IP' by Jeff Ondich and Lauren Jantz,
 * 	summer 1995.
 * 	Rewritten in C++, Jeff Ondich, January 2000
 * 	These are utility functions that are intended to make very simple
 * 	TCP-based client/server programs relatively easy to write."
 * 
 */

#include	"tcpUtilities.h"


//////////////////////////////////////////////////////
//
//	MakeConnection tries to make a TCP connection
//	to the given host and given port.  The return
//	value is either -1 if the connection failed,
//	or the socket number corresponding to the
//	connection if it succeeded.  The hostName
//	can be either a domain name, like www.moose.com,
//	or a dotted-decimal IP address, like
//	123.123.123.321 .
//
//////////////////////////////////////////////////////

AConPtr MakeConnection( const char *hostName, int port, bool usessl )
{
  AConPtr the_connection = new ACon;
  
  ////////////////////////////////////////////////////
  // Initialize sockaddr_in struct.  Shortly, this
  // struct will receive the address of the remote
  // host.
  ////////////////////////////////////////////////////
  
  struct sockaddr_in		remoteAddress;
  
  bzero( (char *)(&remoteAddress), sizeof(remoteAddress) );
  remoteAddress.sin_family = AF_INET;
  remoteAddress.sin_port = htons( (u_short)port );

  
  ////////////////////////////////////////////////////
  // Get the destination IP address using the host
  // name, which may be something like
  // "blum.mathcs.carleton.edu", or a dotted-decimal
  // IP address string like "137.22.4.10".  The
  // resulting address is then stored in 
  // remoteAddress.sin_addr.
  ////////////////////////////////////////////////////
  
  struct hostent *hostInfo = gethostbyname( hostName );
  
  if( hostInfo )
    bcopy( hostInfo->h_addr, (char *)(&remoteAddress.sin_addr), hostInfo->h_length );
  
  else
    {
      ////////////////////////////////////////////////////
      // The string held in hostName didn't work out as
      // a domain name, so now we'll try it as a
      // dotted-decimal IP address.
      ////////////////////////////////////////////////////
      
      remoteAddress.sin_addr.s_addr = inet_addr( hostName );
      if( remoteAddress.sin_addr.s_addr == INADDR_NONE )
	return( NULL );
    }
  
  
  ////////////////////////////////////////////////////
  //	Ask the OS for a socket to use in connecting
  //	to the remote host.
  ////////////////////////////////////////////////////
  
  int sock = socket( PF_INET, SOCK_STREAM, 0 );
  if( sock < 0 )
    return( NULL );
  
  
  ////////////////////////////////////////////////////
  // Connect the socket to destination address/port.
  ////////////////////////////////////////////////////
  
  if( connect( sock, (struct sockaddr *)&remoteAddress, sizeof(remoteAddress) ) < 0 )
    return( NULL );
  
  
  ////////////////////////////////////////////////////
  // The connection is good.  Return the socket.
  ////////////////////////////////////////////////////

  the_connection->sock=sock;
  the_connection->usessl=usessl;
#ifndef NOSSL
  if (usessl) {
    SSL_load_error_strings();
    SSL_library_init();
    char qrandomstring[15];
    srand(time(0));
    sprintf(qrandomstring,"%d",rand());
    RAND_seed(qrandomstring, strlen(qrandomstring));
    SSL_CTX *the_ctx = SSL_CTX_new(SSLv23_client_method());    
    the_connection->the_ssl=SSL_new(the_ctx);
    SSL_set_fd(the_connection->the_ssl, sock);
    if (SSL_connect(the_connection->the_ssl)<=0) {
      cout << "SSL connection failed" << endl;
      return NULL;
    }
  }
#endif



  
  return( the_connection );
}



//////////////////////////////////////////////////////
//
//	ReadFromSocket is just a fancy way to call 
//	read() over and over until a desired number
//	of characters arrive.  Since we're reading
//	from a network connection, the bytes may
//	arrive in unpredictable numbers, so read()
//	returns not when it has as many bytes as
//	requested, but simply when there are any
//	bytes to return at all.  That way, a well
//	designed program could process whatever bytes
//	it has received while waiting for more
//	(witness, for example, the gradual composition
//	of a web page in a well designed browser).
//
//	ReadFromSocket allows you to wait until
//	all your bytes have arrived.  One could
//	easily write a similar function to wait
//	until a particular byte value or combination
//	of values arrives.
//
//////////////////////////////////////////////////////

int ReadFromSocket( AConPtr the_connection, char *where, int bytesToRead )
{
	int		n, bytesToGo;

	bytesToGo = bytesToRead;
	while( bytesToGo > 0 )
	{
	  if (the_connection->usessl) { 
#ifndef NOSSL
		  n = SSL_read( the_connection->the_ssl, where, bytesToGo );
#endif
	}	else
		  n = read( the_connection->sock, where, bytesToGo );
		if( n < 0 )
			return( n );  // error: caller should handle it
		if( n == 0 )	// EOF
			return( bytesToRead - bytesToGo );
		
		bytesToGo = bytesToGo - n;
		where = where + n;
	}
	return( bytesToRead - bytesToGo );
}

void WriteToSocket( AConPtr the_connection, const char *where, int bytesToWrite )
{


  if (the_connection->usessl){
#ifndef NOSSL
    SSL_write(the_connection->the_ssl, where, bytesToWrite);
#endif
  } else
    write(the_connection->sock, where, bytesToWrite);

}


