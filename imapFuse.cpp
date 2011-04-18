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
Folder* search_subfolder_in_folder(char* name, Folder* f);
Folder* search_subfolder(char * name);
int get_folders_list_from_server(string folder_root, Folder* folder);

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
	} else if (strcmp(path, hello_path) == 0) {	// DEBUG
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else if (search_folder(((char *)path)+1) != NULL) {	// If it is a dir in folder root
		// I do path + 1, because the path is like /folder_name (ignore / in the search)
		stbuf->st_mode = S_IFDIR | 0555;	// dr-xr-xr-x
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_size = 0;
		stbuf->st_blksize = 4096;
		stbuf->st_blocks = stbuf->st_size/stbuf->st_blksize+((stbuf->st_size%stbuf->st_blksize)? 1:0);
	} else if (search_subfolder((char *) path) != NULL) {	// If it is a subfolder
		stbuf->st_mode = S_IFDIR | 0555;	// dr-xr-xr-x
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_size = 0;
		stbuf->st_blksize = 4096;
		stbuf->st_blocks = stbuf->st_size/stbuf->st_blksize+((stbuf->st_size%stbuf->st_blksize)? 1:0);
	} else
		res = -ENOENT;

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

	if (strcmp(path, "/") != 0) {// If it isn't the folder root
		Folder* f;
		char* folder1;
		char* temppath = (char*) malloc(strlen(path)+1);
		
		strncpy(temppath,path,strlen(path)+1);	// get a copy of path, because strtok modifies the string
		folder1 = strtok(temppath, "/");	// get the first folder name of the path
		
		if (get_folders_list_from_server("", NULL) != SUCCESS) // get the folder_list
			return -ENOENT;
		f = search_folder(folder1); // I do path + 1, because the path is like /folder_name (ignore / in the search)
		
		if (f != NULL) {	// If folder exists
			// The rigor entries
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			//cout << "." << endl << ".." << endl;
			
			// Obtain subfolders (if exists)
			if (get_folders_list_from_server(path+1, f) != SUCCESS) // path+1 because the path is like /name
				return -ENOENT;
				
			f = search_subfolder((char*)path); // get the actual folder
			
			int num_subfolders = f->get_Num_subFolders();	// num of subfolders
			if (num_subfolders > 0) {
				int i;
				Folder* tempfolder;
				for (i=1;i<=num_subfolders;i++) {	// Insert in filler all the subfolder names
					tempfolder = f->get_subFolder(i);
					filler(buf, tempfolder->get_Folder_Name().c_str(), NULL, 0);
					//cout << tempfolder->get_Folder_Name() << endl;
				}
			}
			
			// Get emails (if exists)
			/*string folderOpenStatus;
			int numMessages;
			int i;
			char* path_folder = ((char*) f->get_Full_Folder_Name().c_str())+1;
			if (SelectFolder(connection, path_folder, numMessages, folderOpenStatus) != UNKNOWN_MAILBOX) {
				i = GetNewMail(connection, *f);
				if (i>BEG_FAIL || i==LIST_FAILED || i== FOLDER_NOT_EXIST)
					return -ENOENT;
					
				// Show all the messages
				Message *email;
				//for (i=1;i<=numMessages;i++) {
				//	email = f->get_Message(i);
				//	filler(buf, (char*) email->get_From_Address().c_str(), NULL, 0);
				//}
				
				char* num = (char*) malloc(20);
				sprintf(num, "%d", numMessages);
				filler(buf, num, NULL, 0); // DEBUG
				free(num);
			} else
				filler(buf, "UNKNOW MAILBOX ", NULL, 0); // DEBUG
			*/
			
			free(temppath);
			return 0;
		} 
		
		return -ENOENT;
		
	} else {	// We are in folder root
		if (get_folders_list_from_server("", NULL) == SUCCESS) {	// Obtain list of folders from folder root
			// The rigor entries
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			//cout << "." << endl << ".." << endl;
			
			// Obtain all the folders
			list<Folder>::iterator itera = folder_list.begin();
			while(itera != folder_list.end()) {
				filler(buf, ((*itera).get_Folder_Name()).c_str(), NULL, 0);
				//cout << (*itera).get_Folder_Name() << endl;
				itera++;
			}
		} else {	// An error has been ocurred
			return -ENOENT;
		}
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
 * Add folder to folder list
 * @param F folder object
 * @param folder_root name of folder root
 * @param folder parent folder
 */ 
void add_Folder_in_folder(Folder F, string folder_root, Folder* folder) {
    F.set_Full_Folder_Name( folder_root + "/" + F.get_Folder_Name() );
    folder->add_subFolder(F); 
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
 * Convert a list of folder names in a list of Folder objects
 * @param folder_names A list<strings> with the folder names
 * @param folder_root name of folder root
 */
void create_folder_list_in_folder( list<string> folder_names, string folder_root, Folder* folder) {
	Folder *F;
	list<string>::iterator it = folder_names.begin();
	while( it != folder_names.end() )
	{
		if( *it != "\r" ) { // Because of the way the names were parsed, there's a \r in the empty name
			F = new Folder( *it );
			add_Folder_in_folder(*F, folder_root, folder);
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
Folder* search_folder(char* name) {
	list<Folder>::iterator itera = folder_list.begin();
	while(itera != folder_list.end()) {
		if (strcmp(name, ((*itera).get_Folder_Name()).c_str()) == 0) {
			return &(*itera);
		}
		itera++;
	}
	
	return NULL;
}

/**
 * Search for a folder in other folder. It needs the complete path to look for it.
 * @param name Name of folder.
 * @return The folder object.
 */
Folder* search_subfolder(char* name) {
	Folder* f = NULL;
	char* pathtemp = (char*) malloc(strlen(name)+1);
	char* pch;
	
	strncpy(pathtemp, name, strlen(name)+1);	// make a copy of name
	pch = strtok(pathtemp,"/");
	if (pch == NULL) {
		free(pathtemp);
		return NULL;
	}
	
	// Obtain parent folder
	f = search_folder(pch);
	if (f == NULL) {
		free(pathtemp);
		return NULL;
	}
	
	// Obtain subfolder
	while (pch != NULL) {
		pch = strtok(NULL, "/");
		if (f == NULL) {
			free(pathtemp);
			return NULL;
		}
		if (pch == NULL) {
			break;
		}
		f = search_subfolder_in_folder(pch, f);
	}
	
	free(pathtemp);
	return f;
}

/**
 * Search for a subfolder in a specified folder.
 * @param name Name of folder.
 * @param f folder object.
 * @return The folder object.
 */
Folder* search_subfolder_in_folder(char* name, Folder* f) {
	int i;
	Folder* folder_temp;
	string folder_name;
	for (i=1; i<=f->get_Num_subFolders(); i++) {	// Search in list of subfolders of the current folder
		folder_temp = f->get_subFolder(i);
		folder_name = folder_temp->get_Folder_Name();
		if (strcmp((folder_name.c_str()), name) == 0)
			return folder_temp;
	}
	return NULL;
}

// Imap Methods
/**
 * Obtain a list of folders. If it is a subfolder, the function gets the folder list of each folder in the path. If that folder list already exists, refresh it.
 * @param folder_root Name of folder root
 * @param folder NULL if it is the folder root ("/") or a Folder* if it's a subfolder
 * @return SUCCESS or LIST_FAILED
 */ 
int get_folders_list_from_server(string path, Folder* folder) {
	int return_code;
	list<string> folder_names;
	
	if (folder == NULL) { // Create folder list in folder root
		folder_names.clear();
		return_code = getIMAPFolders(connection, folder_names, path);
		if (return_code == SUCCESS) {
			folder_list.clear();// empty folder_list
			create_folder_list(folder_names, "");// obtain a new list
		}
	} else { // create folder list in a specified folder
		char* pathtemp = (char*) malloc(strlen((char*)path.c_str()) + 1);
		string pch;
		string str;
		string path2;
		int posBar = 0;
		bool salir = false;
		
		str = path;
		
		posBar = str.find_first_of("/");
		if (posBar == -1) {
			pch = str;
			str = "";
		} else {
			pch = str.substr(0,posBar);
			str.erase (0,posBar+1);		
		}
		path2 = pch;
		
		// Obtain de folder root list
		folder_names.clear();
		return_code = getIMAPFolders(connection, folder_names, "");	// get the folder names in folder root
		if (return_code != SUCCESS)
			return return_code;
		
		folder_list.clear(); // empty folder_list
		create_folder_list(folder_names, "");// obtain a new list
		
		Folder* f = search_folder((char*)pch.c_str()); // gets the first folder in the folder hierarchy
		
		while (!salir) {
			folder_names.clear();
			return_code = getIMAPFolders(connection, folder_names, path2); // get subfolder names list
			f->clear_subfolders();	// empty subfolder_list
			create_folder_list_in_folder(folder_names, path2, f); // get a new list	
			
			if (posBar == -1)
				salir = true;
				
			// Replace strtok
			posBar = str.find_first_of("/");
			if (posBar == -1) {
				pch = str;
				str = "";
			} else {
				pch = str.substr(0,posBar);
				str.erase (0,posBar+1);
			}
			
			path2.append("/");
			path2.append(pch);
			
			f = search_subfolder((char*)path2.c_str()); // Obtain the folder object with the name stored in pch.
			// (we pass the absolute path stored in "path2")
		}
		
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
	
	/*Folder *folder = new Folder("[Gmail]");
	get_folders_list_from_server("[Gmail]", folder);
	cout << "FOLDER LIST SUBFOLDER: " << folder->get_Num_subFolders() << endl;*/
	
	/*get_folders_list_from_server("", NULL);
	Folder* f;
	f = search_folder((char *)"[Gmail]");
	if (f == NULL) 
		cout << "es nulo" << endl;
	get_folders_list_from_server("[Gmail]", f);
	if (search_subfolder((char *)"/[Gmail]/Borradores") == NULL)
		cout << "null" << endl;
	else
		cout << "no null" << endl;*/
	
	/*char* folder1;
	char* path = (char*) "/3x3/inscripciones/2011";
	char* temppath = (char*) malloc(strlen(path)+1);
	strncpy(temppath,path,strlen(path)+1);
	cout << path << endl;
	cout << temppath << endl;
	folder1 = strtok(temppath, "/");
	cout << "f1: " << folder1<<endl;
	while (folder1 != NULL) {
		folder1 = strtok(NULL, "/\n");
		if (folder1 == NULL)
			continue;
		cout << "f1: " << folder1 << endl;
	}
	free(temppath);*/
	
	/*get_folders_list_from_server("", NULL);
	Folder* f = search_folder((char*)"3x3");
	get_folders_list_from_server("/3x3/Inscripciones", f);*/
	
	/*struct stat a;
	
	cout << "READDIR SOBRE /" << endl;
	ImapFuse_readdir("/", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/", &a);
	
	cout << "READDIR SOBRE /3x3" << endl;
	ImapFuse_readdir("/3x3", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/3x3", &a);
	
	cout << "READDIR SOBRE /Inscripciones" << endl;
	ImapFuse_readdir("/3x3/Inscripciones", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/3x3/Inscripciones", &a);
	
	cout << "READDIR SOBRE /3x3" << endl;
	ImapFuse_readdir("/3x3", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/3x3", &a);
	
	cout << "READDIR SOBRE /Documentos" << endl;
	ImapFuse_readdir("/3x3/Documentos", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/3x3/Documentos", &a);
	
	cout << "READDIR SOBRE /3x3" << endl;
	ImapFuse_readdir("/3x3", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/3x3", &a);
	
	cout << "READDIR SOBRE /" << endl;
	ImapFuse_readdir("/", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/", &a);
	
	cout << "READDIR SOBRE /[Gmail]" << endl;
	ImapFuse_readdir("/[Gmail]", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/[Gmail]", &a);
	
	cout << "READDIR SOBRE /[Gmail]/Borradores" << endl;
	ImapFuse_readdir("/[Gmail]/Borradores", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/[Gmail]/Borradores", &a);
	
	cout << "READDIR SOBRE /[Gmail]" << endl;
	ImapFuse_readdir("/[Gmail]", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/[Gmail]", &a);
	
	cout << "READDIR SOBRE /Enviados" << endl;
	ImapFuse_readdir("/[Gmail]/Enviados", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/[Gmail]/Enviados", &a);
	
	cout << "READDIR SOBRE /[Gmail]" << endl;
	ImapFuse_readdir("/[Gmail]", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/[Gmail]", &a);
	
	cout << "READDIR SOBRE /[Gmail]/Todos" << endl;
	ImapFuse_readdir("/[Gmail]/Todos", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/[Gmail]/Todos", &a);*/
	
	return fuse_main(argc, argv, (dispatcher->get_fuseOps()), NULL);
}
