/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  Only a header is stored with the size to allow
 * for realloc() to retrieve the block size.  Blocks are never coalesced 
 * or reused in this naive implementation. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>

#include "mm.h"
#include "memlib.h"
#include "config.h"             /* defines ALIGNMENT */
#include "slist.h"

/* Some useful macros */
static void* prev_block(struct* block_header) {
    return ((void*) block_header) - header->prev_size;
}
static void* next_block(struct* block_header) {
    return ((void*) block_header) - header->size;
}
/* Round up to next power of 2 */
static size_t round_power(size_t size) {
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return 1 + size;
}

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "travis42+cwinkows",
    /* First member's full name */
    "Travis Porter",
    /* First member's SLO (@cs.vt.edu) email address */
    "travis42",
    /* Second member's full name (leave blank if none) */
    "Calvin Winkowski",
    /* Second member's SLO (@cs.vt.edu) email address (leave blank if none) */
    "cwinkows"
};

/* 
 * If size is a multiple of ALIGNMENT, return size.
 * Else, return next larger multiple of ALIGNMENT:
 * (size/ALIGNMENT + 1) * ALIGNMENT
 * Does so without requiring integer division, assuming
 * ALIGNMENT is a power of 2.
 */
static size_t roundup(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

/* 
 * This C struct captures an allocated header.
 *
 * By casting a memory location to a pointer to a allocated_block_header,
 * we are able to treat a part of memory as if a header had been allocated
 * in it.
 *
 * Note: you should never define instances of 'struct allocated_block_header' -
 *       all accesses will be through pointers.
 */
struct allocated_block_header {
    unsigned char prev_free : 1;
    unsigned char free : 1;
    size_t prev_size;
    size_t size;
    /* 
     * Zero length arrays do not add size to the structure, they simply
     * provide a syntactic form to refer to a char array following the
     * structure.
     * See http://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
     *
     * The 'aligned' attribute forces 'payload' to be aligned at a
     * multiple of alignment, counted from the beginning of the struct
     * See http://gcc.gnu.org/onlinedocs/gcc/Variable-Attributes.html
     */
    char        payload[0] __attribute__((aligned(ALIGNMENT)));
};

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Sanity checks. */
    assert((ALIGNMENT & (ALIGNMENT - 1)) == 0); // power of 2
    assert(sizeof(struct allocated_block_header) == ALIGNMENT);
    assert(offsetof(struct allocated_block_header, size) == 0);
    assert(offsetof(struct allocated_block_header, payload) % ALIGNMENT == 0);
    return 0;
}

static malloc_freelist(size_t size) {
    return NULL;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = roundup(size + sizeof(struct allocated_block_header));
    
    void* reused = malloc_freelist(size_t size);
    if (reused != NULL) {
        return reused;
    }

    struct allocated_block_header * blk = mem_sbrk(newsize);
    if (blk == NULL)
	return NULL;

    blk->size = size;
    return blk->payload;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *oldptr, size_t size)
{
    void *newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Assuming 'oldptr' was a '&payload[0]' in an allocated_block_header,
     * determine its start as 'oldblk'.  Then its size can be accessed
     * more easily.
     */
    struct allocated_block_header *oldblk;
    oldblk = oldptr - offsetof(struct allocated_block_header, payload);

    size_t copySize = oldblk->size;
    if (size < copySize)
      copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

const int NUM_BUCKETS = 5;
slist_node* free_lists[NUM_BUCKETS];

slist_node* get_free_list(size_t size){
}

typedef struct slist_node_struct {
    struct node_struct* next;
    void* data; 
} slist_node;

slist_node* create_node(void* d) {
    slist_node* node;

    node = mem_sbrk(sizeof(node));
    node->next = NULL;
    node->data = d; 

    return node;
}

slist_node* insert_node(slist_node* list, void* data){
    slist_node* node;

    node=create_node(data);
    node->next = list;
    
    return newnode;
}

//doesn't deallocate the node
void remove_node(slist_node* list, slist_node* node){
    
    if(list == NULL || slist_node == NULL)
        return;

    node* cur = list;

    while(cur->next != NULL && cur->next != node)
        cur = cur->next;

    if(cur->next != NULL){
        cur->next = node->next;
    }
}

// vim: ts=8
