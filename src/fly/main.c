/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include "fly.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>

/*===========================================================================*
 *				fly_main					     *
 *===========================================================================*/
PUBLIC void fly_main(void){
    Message msg;

    receive(ANY, &msg);
}
