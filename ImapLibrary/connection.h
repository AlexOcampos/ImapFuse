/**
 * connection.h
 * 
 * Adapted from Althea Project by Alejandro Ocampos for ImapFuse
 * 
 * Description: establishes the ACon type so that information about the
 * connection may be passed arround
 * Updated: 28/03/2011
 * 
 */

#ifndef CONNECTION_H
#define CONNECTION_H
namespace std {};
using namespace std;

#ifndef NOSSL
  #include <openssl/ssl.h>
  #include <openssl/rand.h>
#endif


struct ACon {
  int sock;
  bool usessl;
#ifndef NOSSL
  SSL *the_ssl;
#endif

};


typedef ACon * AConPtr;


#endif
