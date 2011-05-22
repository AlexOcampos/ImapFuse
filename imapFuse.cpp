/**
 * @file imapFuse.cpp
 * @brief This file contain the functions which will be connected to FUSE.
 * @author Alejandro Ocampos Veiga
 * @date 30/03/2011
 */

#define FUSE_USE_VERSION 26

#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <list>
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
Message* search_mail(char* path);
Folder* search_folder(char * name);
Folder* search_subfolder_in_folder(char* name, Folder* f);
Folder* search_subfolder(char * name);
int get_folders_list_from_server(string folder_root, Folder* folder);
string trim_email(string mailstring);
char *trimwhitespace(char *str);

// FUSE methods
/**
 * This function obtain the attributes of a file.
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
	} else if (search_mail((char *) path) != NULL) {
		stbuf->st_mode = S_IFDIR | 0555;	// dr-xr-xr-x
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_size = 0;
		stbuf->st_blksize = 4096;
		stbuf->st_blocks = stbuf->st_size/stbuf->st_blksize+((stbuf->st_size%stbuf->st_blksize)? 1:0);
	}  else
		res = -ENOENT;

	return res;
}

/**
 * This function verify if a file (email) exists and if it has enough permissions
 * @param path The path of the file
 * @param fi Information about open files
 * @return 0 on success. -ENOENT if the file doesn't exist or -EACCES if the file hasn't permissions
 */
static int ImapFuse_open(const char *path, struct fuse_file_info *fi) {
	// Look for the email
	if (search_mail((char *) path) == NULL)
		return -ENOENT;

	// Verify permissions
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
		
	return 0;
}

/**
 * This function read the content of a file (email)
 * @param path The path of the file
 * @param buf The buffer where the content of the email will be stored
 * @param size The size of the email
 * @param offset Position from which we must read
 * @param fi Information about open files
 * @return 0 on success. -ENOENT if the file doesn't exist or -EACCES if the file hasn't permissions
 */
static int ImapFuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	size_t len;
	(void) fi;

	Message* email;
	string text;
	 
	// Look for the email
	email = search_mail((char *) path);
	if (email == NULL)
		return -ENOENT;

	// Get body of the email
	email->get_Body_From_Server(connection);
	if (email->have_Body_Text() != 1) // not body
		return -ENOENT;

	// Print email
	text = email->get_Body_Text();
	
	len = text.length();
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, ((char*)text.c_str()) + offset, size);
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
		
		if (get_folders_list_from_server("", NULL) != SUCCESS) { // get the folder_list
			free(temppath);
			return -ENOENT;
		}
		f = search_folder(folder1);
		
		if (f != NULL) {	// If folder exists
			// The rigor entries
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			
			// Obtain subfolders (if exists)
			if (get_folders_list_from_server(path+1, f) != SUCCESS) { // path+1 because the path is like /name
			
				// path could be a email. In this case, I have to cut the path to the email, 
				// check it and check if it's really a email
				string temp_path = path;
				int lastbar = temp_path.find_last_of("/");
				if (lastbar+1 == temp_path.length()) {	// if the last character of path is a '/'
					temp_path = temp_path.substr(0,temp_path.length() -1);
					lastbar = temp_path.find_last_of("/");
				}
				if (temp_path[0] == '/')	// if the first character of path is a '/'
					temp_path = temp_path.substr(1,lastbar-1);	
				else
					temp_path = temp_path.substr(0,lastbar);
					
				// Check if is a correct path and a correct email. In affirmative case fill with "header" and "body"
				if (get_folders_list_from_server(temp_path, f) != SUCCESS) {
					free(temppath);
					return -ENOENT; // The path doesn't exist
				} else {
					if (search_mail((char*)path) != NULL) {
						filler(buf, "header", NULL, 0);
						filler(buf, "body", NULL, 0);
					} else {
						free(temppath);
						return -ENOENT; // It isn't an email
					}
				}
				
			} else {
				f = search_subfolder((char*)path); // get the actual folder
				
				int num_subfolders = f->get_Num_subFolders();	// num of subfolders
				if (num_subfolders > 0) {
					int i;
					Folder* tempfolder;
					for (i=1;i<=num_subfolders;i++) {	// Insert in filler all the subfolder names
						tempfolder = f->get_subFolder(i);
						filler(buf, tempfolder->get_Folder_Name().c_str(), NULL, 0);
					}
				}
				
				// Get emails (if exists)
				string folderOpenStatus;
				int numMessages;
				int i;
				char* path_folder = ((char*) f->get_Full_Folder_Name().c_str())+1;
				if (SelectFolder(connection, path_folder, numMessages, folderOpenStatus) != UNKNOWN_MAILBOX) {
					i = GetNewMail(connection, *f);
					if (i>BEG_FAIL || i==LIST_FAILED || i== FOLDER_NOT_EXIST)
						return -ENOENT;
							
					// Show all the messages
					Message *email;
					string email_from;
					string email_subject;
					string email_uid;
					string email_file;
					for (i=1;i<=numMessages;i++) {
						email = f->get_Message(i);

						// Get from address
						email_from = email->get_From_Address();
						email_from = trim_email(email_from);

						// Get subject
						email_subject = email->get_Subject();
						email_subject = trimwhitespace((char *)email_subject.c_str());

						// Get uid
						char* temp = (char*) malloc(4);
						sprintf(temp, "%d", email->get_UID());
						email_uid = temp;

						email_file = "";
						email_file.append(email_uid);
						email_file.append("-");
						email_file.append(email_subject);
						email_file.append(" - ");
						email_file.append(email_from);
						
						
						filler(buf, (char*) email_file.c_str(), NULL, 0);
						free(temp);
					}
				} else
					cout << "UNKNOW MAILBOX" << endl;
			}
			
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
 * Copy the valid options for fuse_main in dest.
 * @param argc the size of argv[]
 * @param argv a vector which contains all the mount options
 * @param dest a vector which the valid options will be copied
 * @return number of options copied
 */
int copy_options(int argc, char* argv[], char* dest[]) {
	int i=0, j=0;
	
	while (i < argc) {
		if (strcmp(argv[i], "-u")==0 || strcmp(argv[i], "-w")==0) {
			i+=2;
			continue;
		}
		dest[j] = argv[i];
		i++;
		j++;
	}
	return j;
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
 * Gets the mount option specified in option.
 * @param argc the size of argv[]
 * @param argv a vector which contains all the mount options
 * @param option the option which is looking for
 * @param result the result of the searching
 */
void mount_options(int argc, char *argv[], char* option, char** result){
	*result = NULL;
	for (int i = 0; i<argc; i++) {
		if (strcmp(argv[i], option) == 0) {
			*result = argv[i+1];
			break;
		}
	}

	return;
}

/**
 * Search for a email in a folder
 * @param path The path of the email.
 * @return The email object.
 */
Message* search_mail(char * path) {
	Message* email;
	Folder* f;

	string temp_path;
	int lastbar;
	int numMessages;
	int i;
	string folderOpenStatus;
	string email_name;
	string email_path;

	char* temp = (char*) malloc(4);
	string email_uid;
	string email_from;
	string email_subject;
	string email_file;

	temp_path = path;
	lastbar = temp_path.find_last_of("/");
	if (lastbar+1 == temp_path.length()) {	// if the last character of path is a '/'
		temp_path = temp_path.substr(0,temp_path.length() -1);
		lastbar = temp_path.find_last_of("/");
	}
	// Copy the name of the email
	email_name = temp_path.substr(lastbar+1); 

	// Copy the path to the email
	if (temp_path[0] == '/')	// if the first character of path is a '/'
		email_path = temp_path.substr(1,lastbar-1);	
	else
		email_path = temp_path.substr(0,lastbar);

	// Now, we have the email path (in email_path) an the email name (in email_name)
	// Look for the last folder in path
	f = search_subfolder((char*) email_path.c_str());
	if (f == NULL)
		return NULL;

	if (SelectFolder(connection, (char*) email_path.c_str(), numMessages, folderOpenStatus) == UNKNOWN_MAILBOX) {
		free(temp);
		return NULL;
	}
	i = GetNewMail(connection, *f);
	if (i>BEG_FAIL || i==LIST_FAILED || i== FOLDER_NOT_EXIST) {
		free(temp);
		return NULL;
	}
	
	// Look for email in the folder
	for (i=1;i<=numMessages;i++) {
		email = f->get_Message(i);

		// Get from address
		email_from = email->get_From_Address();
		email_from = trim_email(email_from);

		// Get subject
		email_subject = email->get_Subject();
		email_subject = trimwhitespace((char*) email_subject.c_str());

		// Get uid
		sprintf(temp, "%d", email->get_UID());
		email_uid = temp;

		email_file = "";
		email_file.append(email_uid);
		email_file.append("-");
		email_file.append(email_subject);
		email_file.append(" - ");
		email_file.append(email_from);
		
		if (email_name.compare(email_file) == 0) {
			free(temp);
			return email;
		}
	}
	free(temp);
	return NULL;
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

/**
 * Only trim the email of a string. The email must be among < and >.
 * @param mailstring The string with the email.
 * @return Only the email.
 */
string trim_email(string mailstring) {
	string email;
	size_t posini, posfin;

	posini = mailstring.find_last_of("<");
	posfin = mailstring.find_last_of(">");

	if ((posfin <= posini) || (posini == string::npos) || (posfin == string::npos))
		email = mailstring;
	else
		email = mailstring.substr(posini+1, (posfin-posini)-1);
		
	if (email.find("@") == string::npos)
		email = mailstring;
		
	return email;
}

/**
 * Trim white spaces
 * @param str The string
 * @return The string without whitespaces.
 */
char *trimwhitespace(char *str) {
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
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
	int numMessages;
	string folderOpenStatus;
	
	if (folder == NULL) { // Create folder list in folder root
		folder_names.clear();
		if ((return_code = SelectFolder(connection, "", numMessages, folderOpenStatus)) != UNKNOWN_MAILBOX) {
			return return_code;
		}
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
			if (f == NULL) {
				salir = true;
				return_code = -1;
			}	
		}
		
	}
	return return_code;
}

/**
 * Connect with server and login
 * @return 0 on success or -1 on failure
 */
int login(char* user, char* pass) {
	imap_server 	= "imap.gmail.com";					// Nombre servidor IMAP
	port_n			= 993;								// 143 || 993 con encriptación SSL
	//~ cout << "Write your username: "; cin >> user;		// Usuario
	//~ cout << "Write your pass: "; cin >> pass;			// Password
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
	char* username;
	char* password;
	int nOptions;
	char* dest[50];
	
	dispatcher = new FuseDispatcher();
	
	dispatcher->set_getattr	(&ImapFuse_getattr);
	dispatcher->set_readdir	(&ImapFuse_readdir);
	dispatcher->set_open	(&ImapFuse_open);
	dispatcher->set_read	(&ImapFuse_read);
	
	// Mount options
	mount_options(argc, argv, (char*)"-u", &username);	// Get the username
	mount_options(argc, argv, (char*)"-w", &password);	// Get the password
	if (username == NULL || password == NULL)  {
		cout << endl << "** -u and -w options are required!" << endl;
		return -1;
	}
		
	// Copy mount options
	nOptions = copy_options(argc, argv, dest);
	
	
	if (login(username, password) == -1) {
		cout << endl << "** Login failed!" << endl;
		return -1;
	}
	
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
	
	cout << "READDIR SOBRE /GC" << endl;
	ImapFuse_readdir("/GC", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr("/GC/Defensa GC - alexocamposveiga@gmail.com", &a);*/
	
	/*cout << "READDIR SOBRE /3x3" << endl;
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

	/*struct stat a;
	ImapFuse_readdir("/Personal", NULL, NULL, NULL, NULL) ;
	ImapFuse_getattr((char*)"/Personal/asdf - talia.brana@gmail.com (43).html", &a);*/
	//search_mail((char*)"/Personal/asdf - talia.brana@gmail.com (43).html") ;
	
	return fuse_main(nOptions, dest, (dispatcher->get_fuseOps()), NULL);
}
