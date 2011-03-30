/**
 * @file connection.h
 * @author Alejandro Ocampos Veiga
 * @brief Establishes the ACon type so that information about the 
 * connection may be passed arround. Adapted from Althea Project by 
 * Alejandro Ocampos for ImapFuse.
 * @date 30/03/2011
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
