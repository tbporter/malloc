/* 
 * In our approach we use a segmented list to keep track of all the free blocks.
 * We segmented based off of powers of 2. Our lists are doubly linked, using the payload to store the list_elem*
 * Our header stores the size, prev_size, and if it's free. Using the prev_size we can look at the previous node.
 * When reallocing we coalesc blocks together. Aswell as just checking free blocks for a correct size
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "mm.h"
#include "list.h"
#include "memlib.h"
#include "config.h"             /* defines ALIGNMENT */

#define NUM_BUCKETS 6

struct block_header {
    /* Size of previous payload */
    size_t prev_size;
    /* Size of payload */
    size_t size;
    unsigned char free : 1;
    char        payload[0] __attribute__((aligned(ALIGNMENT)));
};

struct list free_lists[NUM_BUCKETS];
static struct block_header* last_header;

/* Declarations of functions */
int get_free_list(size_t size);


/* Some useful macros */
 
/* return's the previous block header */
static struct block_header* prev_block(struct block_header* header) {
    return (struct block_header*) ((void*) header - header->prev_size - sizeof(struct block_header));
}
/* returns the next block header */
static struct block_header* next_block(struct block_header* header) {
    return (struct block_header*) (header->payload + header->size);
}
/* gets a header from the payload */
static struct block_header* header_from_node(struct list_elem* node) {
    return (struct block_header*) ((void*) node - sizeof(struct block_header));
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

    for (i = 0; i < NUM_BUCKETS; i++)
        list_init(&free_lists[i]);

    last_header = blk;
    blk->prev_size = 0;
    blk->size = 0;
    blk->free = false;
    return 0;
}
/* Checks freelists for an appropriate malloc
    returns a payload if there is an exact match on one of the free lists, else null */
static void* malloc_freelist(size_t size) {
    struct list* l = &free_lists[get_free_list(size)];
    struct list_elem* cur = &l->head;
    struct block_header* cur_header;
    struct block_header* next_header;
    int new_size;
    //increment one down the list, since there is always a dummy node
    cur = list_next(cur);

    struct list_elem* tail = list_end(l);
    int count = 0;
    while (count < 10 && cur != tail && header_from_node(cur)->size < size) {
        cur_header = header_from_node(cur);
        if (cur_header != last_header) {
            next_header = next_block(cur_header);
            new_size = next_header->size + cur_header->size + sizeof(struct block_header);
            if (next_header->free && new_size >= size) {
                cur_header->size = new_size;
                cur_header->free = false;
                if (next_header != last_header) {
                    next_header->prev_size = new_size;
                }
                else
                    last_header = cur_header;
                list_remove((struct list_elem*) next_header->payload);
                list_remove((struct list_elem*) cur_header->payload);
                return cur;
            }
        }
        cur = list_next(cur);
        count++;
    }
    /* exit loops either not finding one or cur = valid node */
    if (count < 10 && cur != tail) {
        cur_header = header_from_node(cur);
        cur_header->free = false;
        list_remove((struct list_elem*) cur);
        return cur;
    }

    return NULL;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void* reused = malloc_freelist(size);
    if (reused != NULL) {
        struct block_header* header = header_from_node((struct list_elem*) reused);
        if(header->free){
            header->free = false;
            if (header != last_header) {
                next_block(header)->prev_size = header->size;
            }
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

    struct block_header* header = header_from_node((struct list_elem*) ptr);
    struct list* l = &free_lists[get_free_list(header->size)];
    
    list_push_front (l, (struct list_elem*) ptr);
    header->free = true;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *oldptr, size_t size)
{
    struct block_header* header = header_from_node((struct list_elem*) oldptr);
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
            list_remove((struct list_elem*) next_header->payload);
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
/*  returns the free_list index based off the size,
 *  starting at block size of 8, goes up by a power of 2
 */
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

/* returns true if it exists
 */
bool exist_in_free(struct block_header* b){
    struct list_elem* blk = (struct list_elem*) b->payload;
    struct list* l =  &free_lists[get_free_list(b->size)];;
    struct list_elem* i;
    for(i = list_front(l); i != list_tail(l); i = list_next(i)){
        if(blk == i)
            return true; //we found 
    }
    //return the check on tail
    return (blk == i);
}

/*  returns true if all free blocks exist in the free lists
 *  If the header sizes are incorrect, it will break, likely segfaulting
 */
bool mm_check(){
    struct block_header* cur = (struct block_header*) mem_heap_lo();
    while(cur != last_header){
        if(cur->free)
            if(!exist_in_free(cur))
                return false;
        cur = next_block(cur);
    }
    //return the check on the last header
    return exist_in_free(cur);
}
// vim: ts=8
