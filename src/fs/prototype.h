/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

/* 结构声明 */
struct super_block;
struct inode;
struct file_desc;

/*================================================================================================*/
/* main.c */
/*================================================================================================*/
_PROTOTYPE( void fs_print_info, (char *info) );

/*================================================================================================*/
/* open.c */
/*================================================================================================*/
_PROTOTYPE( int do_creat, (void) );
_PROTOTYPE( int do_open, (void) );
_PROTOTYPE( int do_close, (void) );
_PROTOTYPE( int do_mkdir, (void) );
_PROTOTYPE( int do_mknod, (void) );
_PROTOTYPE( int do_lseek, (void) );

/*================================================================================================*/
/* file.c */
/*================================================================================================*/
_PROTOTYPE( int get_fd, (int start, mode_t mode, int *fd, struct file_desc **fd_slot) );
_PROTOTYPE( struct file_desc *get_file, (int fd) );

/*================================================================================================*/
/* path.c */
/*================================================================================================*/
_PROTOTYPE( struct inode *track_path, (char *path) );
_PROTOTYPE( struct inode *last_dir, (char *path, char last_name[NAME_MAX]) );
_PROTOTYPE( struct inode *step_dir, (struct inode *dir, char name[NAME_MAX]) );

/*================================================================================================*/
/* device.c */
/*================================================================================================*/
_PROTOTYPE( int dev_open, (dev_t device, int proc, int flags) );
_PROTOTYPE( int dev_close, (dev_t device, int proc) );
_PROTOTYPE( int dev_io, (int op, dev_t device,int proc_nr,void *buf_addr, off_t pos, int bytes)  );
_PROTOTYPE( int dev_ioctl, (int device, int proc_nr, void *buf) );

/*================================================================================================*/
/* super.c */
/*================================================================================================*/
_PROTOTYPE( int read_super_block, (struct super_block *sp) );
_PROTOTYPE( struct super_block *get_super_block, (dev_t dev) );
_PROTOTYPE( ino_t alloc_imap_bit, (dev_t dev) );
_PROTOTYPE( int alloc_smap_bit, (dev_t dev, int nr_sects) );

/*================================================================================================*/
/* inode.c */
/*================================================================================================*/
_PROTOTYPE( struct inode *get_inode, (dev_t dev, int numb) );
_PROTOTYPE( void put_inode, (struct inode *inp) );
_PROTOTYPE( void readwrite_inode, (struct inode *np, int type) );
_PROTOTYPE( struct inode *new_inode, (dev_t dev, ino_t inode_nr, int start_sect) );
_PROTOTYPE( void sync_inode, (struct inode *ip) );

/*================================================================================================*/
/* utils.c */
/*================================================================================================*/
_PROTOTYPE( time_t clock_time, (void) );
_PROTOTYPE( int get_pathname, (char *path, int len, int flag) );
_PROTOTYPE( int fs_no_sys, (void) );
_PROTOTYPE( void fs_panic, (const char *msg, int errno) );



