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

struct block_header {
    unsigned char prev_free : 1;
    unsigned char free : 1;
    /* Size of previous payload */
    size_t prev_size;
    /* Size of payload */
    size_t size;
    char        payload[0] __attribute__((aligned(ALIGNMENT)));
};

/* Some useful macros */
static void* prev_block(struct* block_header) {
    return (void*) block_header - header->prev_size - sizeof(struct block_header);
}
static void* next_block(struct* block_header) {
    return ((void*) block_header) + header->size + sizeof(struct block_header);
}
static struct block_header* header_from_node(slist_node_t node) {
    return (struct block_header*) ((void*) node - sizeof(struct block_header));
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

const int NUM_BUCKETS = 6;
slist_node* free_lists[NUM_BUCKETS];


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


static struct block_header* last_header;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Sanity checks. */
    assert((ALIGNMENT & (ALIGNMENT - 1)) == 0); // power of 2
    assert(sizeof(struct block_header) == ALIGNMENT);
    assert(offsetof(struct block_header, size) == 0);
    assert(offsetof(struct block_header, payload) % ALIGNMENT == 0);
    struct block_header* blk = mem_sbrk(sizeof(struct block_header));
    blk->prev_size = 0;
    blk->prev_free = false;
    blk->size = 0;
    blk->free = false;
    return 0;
}
/* Checks freelists for an appropriate malloc
    returns a payload if there is an exact match on one of the free lists, else null */
static void* malloc_freelist(size_t size) {
    slist_node* list = free_lists[get_free_list(size)];
    if(list != NULL){
        while(list->next != NULL && header_from_node(list->next)->size != size ){
            list = list->next;
        }

        if(list->next){
            list->next = list->next->next;
        }

        return list->next;
    }
    else{
        return NULL;
    }

}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void* reused = malloc_freelist(size_t size);
    if (reused != NULL)
        return reused;

    int newsize = roundup(size + sizeof(struct block_header));

    struct block_header * blk = mem_sbrk(newsize);
    if (blk == NULL)
	return NULL;

    blk->size = size;
    blk->free = true;
    blk->prev_size = last_header->size;
    blk->prev_free = last_header->free;
    last_header = blk;
    return blk->payload;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    slist_node* list = free_lists[get_free_list(header_from_node((slist_node_t*) ptr)->size)];
    ((slist_node*) ptr)->next = list;
    list = ptr;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *oldptr, size_t size)
{
    void *newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Assuming 'oldptr' was a '&payload[0]' in an block_header,
     * determine its start as 'oldblk'.  Then its size can be accessed
     * more easily.
     */
    struct block_header *oldblk;
    oldblk = oldptr - offsetof(struct block_header, payload);

    size_t copySize = oldblk->size;
    if (size < copySize)
      copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


int get_free_list(size_t size){
    int i;
    size_t startsize = 8;
    for(i=0; i<NUM_BUCKETS; i++){
        if(size <= startsize)
            return i;
        startsize = startsize << 1;
    }
}

typedef struct slist_node_t {
    struct node_struct* next; 
} slist_node;
// vim: ts=8
