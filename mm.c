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
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"
#include "config.h"             /* defines ALIGNMENT */

#define NUM_BUCKETS 6

typedef struct slist_node {
    struct slist_node* next; 
} slist_node_t;

struct block_header {
    /* Size of previous payload */
    size_t prev_size;
    /* Size of payload */
    size_t size;
    unsigned char free : 1;
    char        payload[0] __attribute__((aligned(ALIGNMENT)));
};

slist_node_t free_lists[NUM_BUCKETS];
static struct block_header* last_header;

/* Declarations of functions */
int get_free_list(size_t size);
void print_list(slist_node_t* list);
void remove_from_list(struct block_header* header);
int get_list_size(slist_node_t* list);
/* Some useful macros */
 

static struct block_header* prev_block(struct block_header* header) {
    return (struct block_header*) ((void*) header - header->prev_size - sizeof(struct block_header));
}

static struct block_header* next_block(struct block_header* header) {
    return (struct block_header*) (header->payload + header->size);
}

static struct block_header* header_from_node(slist_node_t* node) {
    return (struct block_header*) ((void*) node - sizeof(struct block_header));
}
/* Round up to next power of 2 */
/*
static size_t round_power(size_t size) {
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return 1 + size;
}
*/

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
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Sanity checks. */
    assert((ALIGNMENT & (ALIGNMENT - 1)) == 0); // power of 2
    assert(sizeof(struct block_header) % ALIGNMENT == 0);
    assert(offsetof(struct block_header, payload) % ALIGNMENT == 0);
    assert(offsetof(struct block_header, payload) == sizeof(struct block_header));
    struct block_header* blk = mem_sbrk(sizeof(struct block_header));
    int i;

    for (i = 0; i < NUM_BUCKETS; i++) {
        free_lists[i].next = NULL;
    }

    last_header = blk;
    blk->prev_size = 0;
    blk->size = 0;
    blk->free = false;
    return 0;
}
/* Checks freelists for an appropriate malloc
    returns a payload if there is an exact match on one of the free lists, else null */
static void* malloc_freelist(size_t size) {
    slist_node_t* cur = &free_lists[get_free_list(size)];
    slist_node_t* prev;
    
    //increment one down the list, since there is always a dummy node
    prev = cur;
    cur = cur->next;

    while(cur != NULL && header_from_node(cur)->size < size ){
        prev = cur;
        cur = cur->next;
    }

    if(cur){
        prev->next = cur->next;
    }

    return cur;

}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void* reused = malloc_freelist(size);
    if (reused != NULL) {
        struct block_header* header = header_from_node((slist_node_t*) reused);
        header->free = false;
        if(header != last_header){
            next_block(header)->prev_size = header->size;
        }
        return reused;
    }

    int newsize = roundup(size + sizeof(struct block_header));

    struct block_header * blk = mem_sbrk(newsize);
    if (blk == NULL)
        return NULL;

    blk->size = newsize - sizeof(struct block_header);
    blk->free = false;
    blk->prev_size = last_header->size;
    last_header = blk;
    return blk->payload;
}
/* set the next headers prev's free unless the current header's the last header*/

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

    struct block_header* header = header_from_node((slist_node_t*) ptr);
    slist_node_t* list = &free_lists[get_free_list(header->size)];
    ((slist_node_t*) ptr)->next = list->next;
    list->next = (slist_node_t*) ptr;
    header->free = true;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *oldptr, size_t size)
{
    struct block_header* header = header_from_node((slist_node_t*) oldptr);
    struct block_header* prev_header = prev_block(header);
    struct block_header* next_header = next_block(header);
    
    if(header->size >= size){
        return oldptr;
    }

    int new_size = prev_header->size + header->size + sizeof(struct block_header);
    /*if(prev_header->free &&  new_size >= size){
        remove_from_list(prev_header);
        prev_header->size = new_size;
        prev_header->free = false;
        assert(header->free == false);

        memcpy(prev_header->payload, oldptr, new_size);
        return prev_header->payload;
    }*/

    
    
    if(header != last_header){
        new_size = next_header->size + header->size + sizeof(struct block_header);
        if(next_header->free && new_size >= size){
            remove_from_list(next_header);
            
            if(next_header == last_header){
                last_header = header;
            }else{
                next_header->prev_size = new_size;
            }
            assert(header->free == false);
            header->size = new_size;
            return oldptr;        
        }
    }else{

        int diff_size = size - header->size;
        mem_sbrk(diff_size);

        header->size = size;
        return oldptr;
    }



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

void remove_from_list(struct block_header* header){


    slist_node_t* cur = &free_lists[get_free_list(header->size)];
    slist_node_t* prev;
    //increment one down the list, since there is always a dummy node
    prev = cur;
    cur = cur->next;

    while(cur != NULL && cur != (slist_node_t*) header->payload){
        prev = cur;
        cur = cur->next;
    }

    if(cur){
        prev->next = cur->next;
    }
    if (cur==NULL) {
        int i;
        for (i = 0; i < NUM_BUCKETS; i++) {
            print_list(&free_lists[i]);
        }
        printf("header: %p, free: %d\n", header->payload, header->free);
    }
    assert(cur!=NULL);
}

int get_free_list(size_t size){
    int i;
    size_t startsize = 8;
    for(i=0; i<NUM_BUCKETS; i++){
        if(size <= startsize)
            return i;
        startsize = startsize << 1;
    }
    return NUM_BUCKETS - 1;
}

int get_list_size(slist_node_t* list){
    int i = 0;
    while(list != NULL){
        list = list->next;
        i++;
    }
    return i;
}
void print_list(slist_node_t* list){
    int i = 0;
    while(list != NULL && i < 10){
        printf("%p - ", list);
        list = list->next;
        i++;
    }
    printf("\n");
}

// vim: ts=8
