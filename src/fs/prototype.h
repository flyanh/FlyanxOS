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

/*================================================================================================*/
/* main.c */
/*================================================================================================*/
_PROTOTYPE( void fs_print_info, (char *info) );


/*================================================================================================*/
/* device.c */
/*================================================================================================*/
_PROTOTYPE( int dev_open, (dev_t device, int proc) );
_PROTOTYPE( int dev_close, (dev_t device, int proc) );
_PROTOTYPE( int dev_io, (int op, dev_t device,int proc_nr,void *buf_addr, off_t pos, int bytes)  );
_PROTOTYPE( int dev_ioctl, (int device, int proc_nr, void *buf) );

/*================================================================================================*/
/* super.c */
/*================================================================================================*/
_PROTOTYPE( int read_super_block, (struct super_block *sp) );
_PROTOTYPE( struct super_block *get_super_block, (dev_t dev) );

/*================================================================================================*/
/* inode.c */
/*================================================================================================*/
_PROTOTYPE( struct inode *get_inode, (dev_t dev, int numb) );
_PROTOTYPE( void readwrite_inode, (struct inode *np, int type) );

/*================================================================================================*/
/* utils.c */
/*================================================================================================*/
_PROTOTYPE( int fs_no_sys, (void) );
_PROTOTYPE( void fs_panic, (const char *msg, int errno) );



