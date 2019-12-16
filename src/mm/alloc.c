/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/1.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 系统必须对空闲内存保持跟踪，同时进行分配，alloc.c完成这些功能。
 * flyanx0.1暂时只实现最普通的分配，不支持更复杂的调度，所以这个
 * 文件它非常的简洁。
 *
 * 在这里所使用的关键数据结构是内存空洞表，该表维护内存中的内存空洞链表。它将按照内存地址
 * 增加的顺序进行排序。它包含的地址是指从绝对地址0开始的物理内存。在系统初始化期间，
 * ”已分配的内存“将包含中断向量，内核和MM的那部分内存，并将其标记为不可用，并将其从空洞
 * 链表中删除，这很容易做到。
 *
 * 该文件的入口点是：
 *  - alloc_mem     请求一块给定大小的内存。
 *  - free_mem      归还不再需要的内存
 */
#include "mm.h"
#include <flyanx/common.h>
#include <flyanx/callnr.h>
#include <signal.h>     /* 导入它只是因为mmproc.h需要 */
#include "mmproc.h"

#define NR_HOLES    (2 * NR_PROCS)      /* 最多有多少个空洞？ */
#define NIL_HOLE    (struct hole_s *) 0   /* 不存在的内存空洞 */

/* 内存空洞结构定义（本文件内存空洞我们将简称为空洞，其他文件则用全称）
 *
 * 内存空洞的各种信息描述是用块为单位记录的，而不是字节。
 * 其原因非常简单：在16位状态模式下，内存地址用16位整数记录，在块为256字节时可以支持最大16M的内存；
 * 在32位模式下，地址域可以引用高达240字节的地址，即1024G。如果使用字节为单位，那么其数量根本不敢想
 * 象。
 */
struct hole_s {
    struct hole_s *next;	    /* 一个指向表中下一个空洞的指针。 */
    phys_clicks base;		    /* 以块为单位的空洞的基地址 */
    phys_clicks length;		    /* 以块为单位空洞的长度 */
};
typedef struct hole_s Hole;     /* 这样更短 */
PRIVATE Hole hole[NR_HOLES];    /* 内存空洞链表，这个表是单向结构的，所以在任何时候开始找到下一个空洞很容易，
                                 * 但是如果想要找到上一个就必须从头开始搜索直到找到给定的空洞。
                                 */
PRIVATE int merge_count;        /* 合并次数，用于合并空洞 */

PRIVATE Hole *hole_head;        /* 第一个空洞的指针 */
PRIVATE Hole *free_slots;       /* 指向空闲的空洞插槽 */

FORWARD _PROTOTYPE( void remove_slot, (Hole *prev_ptr, Hole *hp) );
FORWARD _PROTOTYPE( void merge_hole, (Hole *hp) );

/*===========================================================================*
 *				mem_init				     *
 *			    内存初始化
 *===========================================================================*/
PUBLIC void mem_init(phys_clicks total, phys_clicks free){
    /* 初始化内存，构造由所有可用内存组成的初始空洞表。
     * 有两个指针，"hole_head"指向系统中所有的空洞的链表；"free_solts"指向
     * 未使用的表条目的链表。最初，前一个链表为每个空闲的物理内存块都有一个条
     * 目，第二个链表将其余的表项，插槽链接在一起。随着系统的运行，内存将可能
     * 变得分散（从最初的一整个可用空洞分解为许多小空洞），这时候需要新的表插
     * 槽来表示它们，这些插槽从"free_slots"链表拿到。
     */
    register Hole *hp;

    /* 将所有空洞放到空闲链表中，即，初始化它们 */
    for(hp = &hole[0]; hp < &hole[NR_HOLES]; hp++) hp->next = hp + 1;
    hole[NR_HOLES - 1].next = NIL_HOLE;
    hole_head = NIL_HOLE;
    free_slots = &hole[0];

    /* 初始化空洞表，这里很简单，因为系统刚启动，所以只有一个空洞，
     * 我们只需要得到这个空洞的基地址和长度就可以了！
     * 基地址 = 等于程序可以安全使用的物理基地址
     * 长度 = 现在可用的空闲空间大小
     * 调用free_mem释放这一个空闲块
     */
    free_mem(PROCS_BASE_CLICK, free);
    /* 完成了，简单吧？ */
}

/*===========================================================================*
 *				alloc_mem				     *
 *			为一个进程分配内存
 *===========================================================================*/
PUBLIC phys_clicks alloc_mem(
        phys_clicks clicks      /* 请求的内存块数量 */
){
    /* 本例程非常简单，它只是按照最简单的首次适配算法查找可用的空洞。
     * 如果找到的块比申请的大，就取出来返回，然后把剩下留在空洞表上
     * 并减掉取走的长度。如果整个空洞都需要，则调用remove_slot将其
     * 从空洞表中删除这个空洞。
     */
    register Hole *hp, *prev_ptr;
    phys_clicks old_base;

    hp = hole_head;
    while (hp != NIL_HOLE){
        if(hp->length >= clicks){
            /* OK，我们从表中发现了一个足够大的空洞，直接用就可以。 */
            old_base = hp->base;    /* 记住这个空洞的基地址 */
            hp->base += clicks;     /* 被用掉了 */
            hp->length -= clicks;   /* 长度也要更新 */

            /* 如果这个空洞已经用完，删除它 */
            if(hp->length == 0) remove_slot(prev_ptr, hp);

            /* 返回获取成功的内存起始地址 */
            return old_base;
        }
        prev_ptr = hp;
        hp = hp->next;
    }
    return NO_MEM;
}

/*===========================================================================*
 *				free_mem				     *
 *				释放内存
 *===========================================================================*/
PUBLIC void free_mem(
        phys_clicks base,   /* 释放的内存基地址 */
        phys_clicks clicks  /* 释放多少块？ */
){
    /* 我们将这个返回的空洞按照地址从小到达的顺序找到它应该所在的位置，然后
     * 将它插入，然后调用merge_hole()去合并空洞，merge_hole()会检查新释
     * 放的内存能够否跟后面的空洞合并，如果能就合并空洞并更新空洞表。
     */

    register Hole *hp, *new_hole, *prev_hole;

    if(clicks == 0) return;     /* 释放0等于不释放 */
    new_hole = free_slots;

    if(new_hole == NIL_HOLE) mm_panic("Hole table full", NO_NUM); /* 空洞表已经满了，这极少发生... */
    /* 将要释放的块填写到new_hole中 */
    new_hole->base = base;
    new_hole->length = clicks;
    free_slots = new_hole->next;    /* 更新空闲插槽 */
    hp = hole_head;

    /* 如果释放的块的地址在地址上小于当前的最低空洞，或者当前根本
     * 没有任何可用的空洞，则将此空洞放在空洞链表的前面。
     */
    if(hp == NIL_HOLE || base <= hp->base){
        new_hole->next = hp;
        hole_head = new_hole;
        merge_hole(new_hole);
        return;
    }

    /* 要返回的空洞不在空洞链表前面，我们寻找到它应该在的位置 */
    while (hp != NIL_HOLE && base > hp->base){
        prev_hole = hp;
        hp = hp->next;
    }

    /* 找到它应该所处的位置了！我们在这里插入这个空洞 */
    new_hole->next = prev_hole->next;
    prev_hole->next = new_hole;
    merge_hole(prev_hole);      /* 合并后的队列应为：prev_hole->new_hole->hp */
}

/*===========================================================================*
 *				merge_hole				     *
 *			合并一个新的空洞到空洞表中
 *===========================================================================*/
PRIVATE void merge_hole(Hole *hp){
    /* 检查是否有连续的空洞并合并它们，释放一块内存时，可能会发生
     * 有连续的空洞，并且碰巧在释放的空洞的两端。
     */

    register Hole *next_hole;
    merge_count++;

    /* 合并次数达到五次，可以停止了 */
    if(merge_count == 5){
        merge_count = 0;
        return;
    }

    next_hole = hp->next;
    /* 如果hp是最后一个空洞，则它不能被合并 */
    if(next_hole == NIL_HOLE) return;

    /* 如果这个空洞刚好紧挨着下一个空洞，那么将后一个空洞
     * 和并到这个空洞中，然后删除后一个空洞。
     */
    if(hp->base + hp->length == next_hole->base){
        hp->length += next_hole->length;
        remove_slot(hp, next_hole);
    } else {
        /* 不过它们不紧挨着，我们继续检查下一个能否合并，递归调用自己，
         * 一直到这个函数被调用5次或碰到了最后一个空洞。
         */
        hp = next_hole;
        merge_hole(hp);
    }
}

/*===========================================================================*
 *				remove_slot				     *
 *				删除空洞插槽
 *===========================================================================*/
PRIVATE void remove_slot(
        Hole *prev_ptr,     /* 指向要删除空洞的上一个空洞指针 */
        Hole *hp            /* 将被删除的空洞 */
){
    /* 从空洞表中删除一个已经用完的空洞
     * 它很简单，删除单链表中的一个元素是基本的数据结构知识 ^ - ^
     */

    if(hp == hole_head){
        /* 如果要删除的空洞是空洞表头，那么表头指向它的下一个，它就没了 */
        hole_head = hp->next;
    } else {
        /* 如果要删除的在中间，它的上一个的下一个指向它的下一个，它就没了 */
        prev_ptr->next = hp->next;
    }

    /* 在这，空闲插槽指向被删掉的这个空洞，下一次新的空洞放在这 */
    hp->next = free_slots;
    free_slots = hp;
}


