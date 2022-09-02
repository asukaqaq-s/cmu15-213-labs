/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
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

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/********************************************************
* DATA 
*********************************************************/

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define DELAY_TIMES 50 
/********************************************************
* MICRO_FUNCTIONS
*********************************************************/
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* header/footer: 0~2 localbit, 3~31 size */
#define PACK(size, alloc) ((size | (alloc)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* header/footer size */
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Read and write a word at address p*/
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p: least 3 bits */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) ((GET(p)) & (0x1))

/* Given block pter bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
// 注意是 DSIZE, 而不是 WSIZE
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* GIVEN block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


/********************************************************
* MICRO_LIST_FUNCTIONS
* 我们假设bp一直指向 payload 的起始位置
*********************************************************/

/* Move List */
#define NEXT_PTR(bp) ((unsigned int *)((char*)bp + WSIZE))
#define LAST_PTR(bp) ((unsigned int *)((char*)bp))
#define NEXT_NODE(bp) (*(NEXT_PTR(bp)))
#define LAST_NODE(bp) (*(LAST_PTR(bp)))



/********************************************************
* FUNCTIONS
*********************************************************/



/* local heap_top_pointer */
static char * heap_top;
static void * list_head; /* 头结点 */

static void print_error();
static void print_bp(char *bp);
static void print_list();

static void* coalesce(void*);
static void* extend_heap(size_t);
static void* FindFit(size_t);
static void place(char *bp, size_t asize);
inline void DeleteNode(char *);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    
    if((heap_top = mem_sbrk(4 * WSIZE)) == NULL) {
        return -1;
    }
    /* 初始化*/
    PUT(heap_top, PACK(0,0));
    PUT(heap_top + 1 * WSIZE, PACK(DSIZE,1));
    PUT(heap_top + 2 * WSIZE, PACK(DSIZE,1));
    PUT(heap_top + 3 * WSIZE, PACK(0,1));
    heap_top += 2 * WSIZE;
    list_head = NULL;
    /* 扩展heap */
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    print_list();
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    return;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp) {
    return;
    size_t size = GET_SIZE(HDRP(bp));
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(NEXT_NODE(bp), list_head);
    PUT(LAST_NODE(bp), NULL);
    
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    return NULL;
}



/********************************************************
* C_FUNCTIONS
*********************************************************/

/**
 * entend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void place(char *bp, size_t asize) {

}


/**
 * coalesce - merge nearly blks
 */
static void* coalesce(void* bp) {
    char * prev_blk = PREV_BLKP(bp);
    char * next_blk = NEXT_BLKP(bp);
    int prev_allocate = GET_ALLOC(HDRP(prev_blk));
    int next_allocate = GET_ALLOC(HDRP(next_blk));
    size_t prev_size = GET_SIZE(HDRP(prev_blk));
    size_t next_size = GET_SIZE(HDRP(next_blk));
    size_t size = GET_SIZE(HDRP(bp));
    
    if(prev_allocate && next_allocate) {
        return bp;
    } else if(!prev_allocate && next_allocate) {
        size += prev_size;        
        DeleteNode(prev_blk);
        PUT(HDRP(prev_blk), PACK(size, 0));
        PUT(FTRP(prev_blk), PACK(size, 0));
        PUT(LAST_PTR(prev_blk), LAST_NODE(bp));
        PUT(NEXT_PTR(prev_blk), NEXT_NODE(bp));
        bp = prev_blk;

    } else if(prev_allocate && !next_allocate) {
        size += next_size;
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(next_blk), PACK(size, 0));
        DeleteNode(next_blk);
    } else if(!prev_allocate && !next_allocate) {
        size += prev_size + next_size;
        PUT(HDRP(prev_blk), PACK(size, 0));
        PUT(FTRP(next_blk), PACK(size, 0));
        PUT(LAST_PTR(prev_blk), LAST_NODE(bp));
        PUT(NEXT_PTR(prev_blk), NEXT_NODE(bp));
        bp = prev_blk;
        DeleteNode(prev_blk);
        DeleteNode(next_blk);
    }
    return bp;
}

/**
 * extend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void* extend_heap(size_t words) {
    size_t asize;
    char *bp;

    /* 满足8字节对齐 */
    asize = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    /* 满足最小块要求 */
    if(asize <= 4 * WSIZE) {
        asize = 4 * WSIZE;
    }
    if((bp = mem_sbrk(asize)) == NULL) {
        return NULL;
    }

    PUT(HDRP(bp), PACK(asize, 0));
    PUT(FTRP(bp), PACK(asize, 0));
    PUT(LAST_PTR(bp), NULL);
    PUT(NEXT_PTR(bp), list_head);
    list_head = bp;
    return coalesce(bp);
}

static void* FindFit(size_t asize) {

}

inline void DeleteNode(char * bp) {
    void * last_blk = LAST_NODE(bp);
    void * next_blk = NEXT_NODE(bp);
    
    if(last_blk && next_blk) {
        NEXT_NODE(last_blk) = next_blk;
        LAST_NODE(next_blk) = last_blk;
    }
    else if(!last_blk && next_blk) {
        LAST_NODE(next_blk) = last_blk;
    }
    else if(last_blk && !next_blk) {
        NEXT_NODE(last_blk) = next_blk;
    }
    else {
        return;        
    }
    NEXT_NODE(bp) = NULL;
    LAST_NODE(bp) = NULL;
}


/********************************************************
* TEST_FUNCTIONS
*********************************************************/

static void print_error() {
    printf("error!!!\n");
    exit(0);
}

static void print_bp(char *bp) {
    printf("%p %p %p size = %d\n",HDRP(bp),bp,FTRP(bp),GET_SIZE(HDRP(bp)));
}

static void print_list() {
    void *p = list_head;
    int cnt = 0;
    while(p != NULL) {
        printf("[%d] size = %d allocated = %d address = %p\n", ++cnt,
            GET_SIZE(HDRP(p)), GET_ALLOC(FTRP(p)), p);
        p = NEXT_NODE(p);
    }
}