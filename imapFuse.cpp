/**
 * @file imapFuse.cpp
 * @brief This file contain the functions which will be connected to FUSE.
 * @author Alejandro Ocampos Veiga
 * @date 30/03/2011
 */

#define FUSE_USE_VERSION 26

#include    <iostream>
#include    <stdlib.h>
#include    <errno.h>
#include    <unistd.h>
#include    <sys/types.h>
#include 	<list>

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fusecpp.h"
#include "ImapLibrary/imap.h"
#include "ImapLibrary/Folder.h"

using namespace fuse_cpp;

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static ACon *connection;
static string imap_server;
static int port_n;
static string user;
static string pass;
static bool usessl;
static list<Folder> folder_list;	// List of folders in folder root

// Methods header
Folder* search_folder(char * name);

// FUSE methods
/**
 * This metod obtain the attributes of a file.
 * @param path The path of the file
 * @param stbuf The place where the stat structure with his attributes will be save
 * @return 0 on success
 */
static int ImapFuse_getattr(const char *path, struct stat *stbuf) {
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, hello_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else { // Si es una carpeta
		//res = -ENOENT;
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_size = 0;
		stbuf->st_blksize = 4096;
		stbuf->st_blocks = stbuf->st_size/stbuf->st_blksize+((stbuf->st_size%stbuf->st_blksize)? 1:0);
	}

	return res;
}

static int ImapFuse_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int ImapFuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;
}

/**
 * This method obtains all the archives in path.
 * @param path The path of the file
 * @param buf The structure which will be filling with folder names
 * @param filler The filling function
 * @param offset The offset between directory entries
 * @param fi The info of fuse file
 */
static int ImapFuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0) {// Si no estamos en el directorio raiz del email
		// Mirar si está en la lista de folders
		Folder* f;
		f = search_folder((char *)path);
		if (f != NULL) {
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			filler(buf, "holahola", NULL, 0); // DEBUG
			return 0;
		}
		
		return -ENOENT;
	} else {
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		list<Folder>::iterator itera = folder_list.begin();
		while(itera != folder_list.end()) {
			filler(buf, ((*itera).get_Folder_Name()).c_str(), NULL, 0);
			itera++;
		}
		filler(buf, hello_path + 1, NULL, 0);
	}
	return 0;
}

// Useful Methods
/**
 * Add folder to folder list
 * @param F A folder object
 * @param folder_root name of folder root
 */ 
void add_Folder(Folder &F, string folder_root) {
    F.set_Full_Folder_Name( folder_root + "/" + F.get_Folder_Name() );
    folder_list.push_back( F );    
    return;
}

/**
 * Convert a list of folder names in a list of Folder objects
 * @param folder_names A list<strings> with the folder names
 * @param folder_root name of folder root
 */
void create_folder_list( list<string> folder_names, string folder_root) {
	Folder *F;
	list<string>::iterator it = folder_names.begin();
	while( it != folder_names.end() )
	{
		if( *it != "\r" ) { // Because of the way the names were parsed, there's a \r in the empty name
			F = new Folder( *it );
			add_Folder(*F, folder_root);
		}
		it++;
	}
	return;
}

/**
 * Search for a folder in folder_list.
 * @param name Name of folder.
 * @return The folder object.
 */
Folder* search_folder(char * name) {
	list<Folder>::iterator itera = folder_list.begin();
	while(itera != folder_list.end()) {
		if (strcmp(name+1, ((*itera).get_Folder_Name()).c_str()) == 0) {
			return &(*itera);
		}
		itera++;
	}
	
	return NULL;
}

// Imap Methods
/**
 * Obtain a list of folders
 * @param folder_root name of folder root
 * @return SUCCESS or LIST_FAILED
 */ 
int get_folders_list_from_server(string folder_root) {
	int return_code;
	list<string> folder_names;
	return_code = getIMAPFolders(connection, folder_names, folder_root );
	if( return_code == SUCCESS ) {
        create_folder_list(folder_names, folder_root);
    }
	return return_code;
}

/**
 * Connect with server and login
 * @return 0 on success or -1 on failure
 */
int login(void) {
	imap_server 	= "imap.gmail.com";					// Nombre servidor IMAP
	port_n			= 993;								// 143 || 993 con encriptación SSL
	cout << "Write your username: "; cin >> user;		// Usuario
	cout << "Write your pass: "; cin >> pass;			// Password
	usessl			= true;
	
	int i = IMAPLogin(connection, imap_server, port_n, user, pass, usessl);
	if (i == BAD_LOGIN) {
		printf("Bad login: %d\n", i);
		return -1;
	}
	if (i == SERVER_NOT_READY_FOR_CONNECTION) {
		printf("Server not ready for connection: %d\n", i);
		return -1;
	}
	return 0;
}

/**
 * Main
 */
int main(int argc, char *argv[]) {
	FuseDispatcher *dispatcher;
	dispatcher = new FuseDispatcher();
	
	dispatcher->set_getattr	(&ImapFuse_getattr);
	dispatcher->set_readdir	(&ImapFuse_readdir);
	dispatcher->set_open	(&ImapFuse_open);
	dispatcher->set_read	(&ImapFuse_read);
	
	login();
	get_folders_list_from_server("");
	
	return fuse_main(argc, argv, (dispatcher->get_fuseOps()), NULL);
}
