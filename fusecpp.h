/**
 * @(#) fusecpp.h - allow access to fuse from C++
 *
 * Gerard J. Cerchio www.circlesoft.com
 *
 */

#ifndef AFS_h_h
#define AFS_h_h

#include <string.h>
#include <fuse.h>

namespace fuse_cpp {
	
	typedef int(*readlink)(const char *, char *, size_t);
	typedef int(*getattr)(const char *, struct stat *);
	typedef int(*getdir) (const char *, fuse_dirh_t, fuse_dirfil_t);
	typedef int(*getdir) (const char *, fuse_dirh_t, fuse_dirfil_t);
	typedef int(*mknod) (const char *, mode_t, dev_t);
	typedef int(*mkdir) (const char *, mode_t);
	typedef int(*unlink) (const char *);
	typedef int(*rmdir) (const char *);
	typedef int(*symlink) (const char *, const char *);
	typedef int(*rename) (const char *, const char *);
	typedef int(*link) (const char *, const char *);
	typedef int(*chmod) (const char *, mode_t);
	typedef int(*chown) (const char *, uid_t, gid_t);
	typedef int(*truncate) (const char *, off_t);
	typedef int(*utime) (const char *, struct utimbuf *);
	typedef int(*open) (const char *, struct fuse_file_info *);
	typedef int(*read) (const char *, char *, size_t, off_t, struct fuse_file_info *);
	typedef int(*write) (const char *, const char *, size_t, off_t,struct fuse_file_info *);
	typedef int(*statfs) (const char *, struct statvfs *);
	typedef int(*flush) (const char *, struct fuse_file_info *);
	typedef int(*release) (const char *, struct fuse_file_info *);
	typedef int(*fsync) (const char *, int, struct fuse_file_info *);
	typedef int(*setxattr) (const char *, const char *, const char *, size_t, int);
	typedef int(*getxattr) (const char *, const char *, char *, size_t);
	typedef int(*listxattr) (const char *, char *, size_t);
	typedef int(*removexattr) (const char *, const char *);
	typedef int(*opendir) (const char *, struct fuse_file_info *);
	typedef int(*readdir) (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
	typedef int(*releasedir) (const char *, struct fuse_file_info *);
	typedef int(*fsyncdir) (const char *, int, struct fuse_file_info *);
	typedef void *(*init) (fuse_conn_info*);
	typedef void (*destroy) (void *);
	typedef int(*access) (const char *, int);
	typedef int(*create) (const char *, mode_t, struct fuse_file_info *);
	typedef int(*ftruncate) (const char *, off_t, struct fuse_file_info *);
	typedef int(*fgetattr) (const char *, struct stat *, struct fuse_file_info *);
	
	/**
	 * FuseDispatcher: this is a C++ binding for the fuse system
	 * 
	 *  to use: declare the appropriate routine in a class as static to the above typedefs
	 *  then before calling fuse_main, instantiate the the dispatcher and call the routines
	 *  that you wish to field. Those not called will be handeled by the fuse defaults.
	 */
	class FuseDispatcher
	{
	
		struct fuse_operations theOps;
	
	public:
	
		FuseDispatcher() { memset( &theOps, 0, sizeof(struct fuse_operations) );  }
	
		struct fuse_operations *get_fuseOps() { return &theOps; }
	
		void set_getattr	( getattr 		ptr ) 	{ theOps.getattr	= ptr; }
		void set_readlink	( readlink 		ptr ) 	{ theOps.readlink	= ptr; }
		void set_getdir		( getdir 		ptr )	{ theOps.getdir 	= ptr; }
		void set_mknod 		( mknod 		ptr ) 	{ theOps.mknod		= ptr; }
		void set_mkdir 		( mkdir 		ptr ) 	{ theOps.mkdir		= ptr; }
		void set_unlink		( unlink 		ptr ) 	{ theOps.unlink		= ptr; }
		void set_rmdir		( rmdir 		ptr ) 	{ theOps.rmdir		= ptr; }
		void set_symlink	( symlink 		ptr ) 	{ theOps.symlink	= ptr; }
		void set_rename		( rename 		ptr ) 	{ theOps.rename		= ptr; }
		void set_link		( link 			ptr ) 	{ theOps.link		= ptr; }
		void set_chmod		( chmod 		ptr ) 	{ theOps.chmod		= ptr; }
		void set_chown		( chown 		ptr ) 	{ theOps.chown		= ptr; }
		void set_truncate	( truncate 		ptr ) 	{ theOps.truncate	= ptr; }
		void set_utime		( utime 		ptr ) 	{ theOps.utime		= ptr; }
		void set_open		( open 			ptr ) 	{ theOps.open		= ptr; }
		void set_read		( read 			ptr ) 	{ theOps.read		= ptr; }
		void set_write		( write 		ptr ) 	{ theOps.write		= ptr; }
		void set_statfs		( statfs 		ptr ) 	{ theOps.statfs		= ptr; }
		void set_flush		( flush 		ptr ) 	{ theOps.flush		= ptr; }
		void set_release	( release 		ptr ) 	{ theOps.release	= ptr; }
		void set_fsync		( fsync 		ptr ) 	{ theOps.fsync		= ptr; }
		void set_setxattr	( setxattr 		ptr ) 	{ theOps.setxattr	= ptr; }
		void set_getxattr	( getxattr 		ptr ) 	{ theOps.getxattr	= ptr; }
		void set_listxattr	( listxattr		ptr ) 	{ theOps.listxattr	= ptr; }
		void set_removexattr	( removexattr 		ptr ) 	{ theOps.removexattr	= ptr; }
		void set_opendir	( opendir 		ptr ) 	{ theOps.opendir	= ptr; }
		void set_readdir	( readdir 		ptr ) 	{ theOps.readdir	= ptr; }
		void set_releasedir 	( releasedir 		ptr ) 	{ theOps.releasedir	= ptr; }
		void set_fsyncdir	( fsyncdir 		ptr ) 	{ theOps.fsyncdir	= ptr; }
		void set_init 		( init 			ptr ) 	{ theOps.init		= ptr; }
		void set_destroy	( destroy	 	ptr ) 	{ theOps.destroy	= ptr; }
		void set_access		( access 		ptr ) 	{ theOps.access		= ptr; }
		void set_create		( create 		ptr ) 	{ theOps.create		= ptr; }
		void set_ftruncate	( ftruncate 		ptr ) 	{ theOps.ftruncate	= ptr; }
		void set_fgetattr	( fgetattr 		ptr ) 	{ theOps.fgetattr	= ptr; }
	};

};

#endif
