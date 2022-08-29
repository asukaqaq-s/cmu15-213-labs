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
* C_FUNCTIONS
*********************************************************/

/* local heap_top_pointer */
char * heap_top;
char * heap_start;


static void print_error() {
    printf("error!!!\n");
    exit(0);
}

static void print_bp(char *bp) {
    printf("%p %p %p size = %d\n",HDRP(bp),bp,FTRP(bp),GET_SIZE(HDRP(bp)));
}

static void print_list() {
    return;
    char *now;
    size_t size;
    int allocated;
    now = heap_top;
    // return;
    printf("start: ");
    // printf("size = %d ",size);
    int cnt = 0;
    while(1) {
        size = GET_SIZE(HDRP(now));
        allocated = GET_ALLOC(HDRP(now));
       //  printf("size = %d alloc = %d ", size, allocated);
        if(size == 0) {
            printf("end = %p maxheap = %p cnt = %d",now,heap_top + (20*(1<<20)), cnt);
            break;
        }
        cnt ++;
        now = NEXT_BLKP(now);
    }
    printf("\n");
}

/**
 * coalesce - merge nearly blks
 */
static void* coalesce(void* bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if(prev_alloc && next_alloc) {
        return bp;
    } 
    else if(prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    
    return bp;
}

/**
 * extend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void* extend_heap(size_t words) {
    char *bp;
    size_t size;
    /* allocate how much blks */
    size = (words % 2) ? (words + 1) * WSIZE:words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }
    
    /* Initialize new free blk */
    PUT(HDRP(bp), PACK(size, 0)); /* new blk header */
    PUT(FTRP(bp), PACK(size, 0)); /* new blk footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new end blk */
    
    return coalesce(bp);
}

/**
 * entend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void place(char *bp, size_t asize) {
    size_t blk_size = GET_SIZE(HDRP(bp));
    size_t bsize = blk_size - asize;
    char * next_p;
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    if(asize == blk_size) /* 大小恰好相等,直接退出 */
        return;
    next_p = NEXT_BLKP(bp);
    PUT(HDRP(next_p), PACK(bsize, 0));
    PUT(FTRP(next_p), PACK(bsize, 0));
}

/**
 * entend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void* find_blk(size_t asize) {
    char *now;
    size_t size;
    int is_allocated;
    now = heap_top;

    while(1) {
        size = GET_SIZE(HDRP(now));
        is_allocated = GET_ALLOC(HDRP(now));
        if(size == 0) 
            break;
        if(size >= asize && !is_allocated) {
            return now;
        }
        now = NEXT_BLKP(now);
    }
    return NULL;
}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    // init four blks, include paddings、header node、end node
    if ((heap_top = mem_sbrk(4 * WSIZE)) == (void *)-1) {
        printf("init failed !\n");
        return -1;
    }    
    PUT(heap_top, PACK(0, 0)); // padding blk
    PUT(heap_top + (1 * WSIZE), PACK(DSIZE, 1)); // pro blk's header
    PUT(heap_top + (2 * WSIZE), PACK(DSIZE, 1)); // pro blk's footer
    PUT(heap_top + (3 * WSIZE), PACK(0, 1)); // end blk
    heap_top += 2 * WSIZE;
    
    heap_start = heap_top;

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        printf("init:entend failed\n");
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    size_t asize;
    char *bp;
    size_t entend_size;
    if(size == 0)
        return NULL;
    if(size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        // 要找一个对齐并且是原来 size + 8, 因为一个块浪费两个 word 给header、footer
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1))/(DSIZE));
    }
    // printf("size = %d, asize = %d\n",size,asize);
    if((bp = find_blk(asize)) != NULL) { /* success find */
        place(bp, asize);
        // printf("malloc: find_blk size = %d\n               ",asize);
        // print_list();
        return bp;
    }
    // 分配一个新的块
    entend_size = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(entend_size/WSIZE)) == NULL) {
        printf("malloc:extend failed\n");
        return NULL;
    }
    
    place(bp, asize);
    // printf("malloc: extend size = %d\n               ",asize);
    // print_list();
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
    // printf("free: size = %d\n               ",size);
    // print_list();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr; /* 原来块的原指针 */
    void *newptr;       /* 原来块的新指针 */
    void *newblk_ptr;   /* 新块的新指针 */
    size_t copySize;
    size_t old_size;
    size_t new_size;
    size_t words = size/WSIZE;
    size_t asize;
    
    if(ptr == NULL) {
        newblk_ptr = mm_malloc(size); 
        return newblk_ptr;  
    }
    else if(size == 0) {
        mm_free(oldptr);
        return NULL;
    }
    old_size = GET_SIZE(HDRP(ptr));
    // 1.看看原来的ptr能不能用
    newptr = coalesce(ptr); /* 合并原来的块 */
    asize = size <=DSIZE ? 2*DSIZE : DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); 
    
    if(newptr != oldptr) { /* 如果前面有空闲块,payload前移 */
        memcpy(newptr, oldptr, old_size - DSIZE);
    }
    new_size = GET_SIZE(HDRP(newptr));

    if(asize <= new_size) { /* 能够满足直接退出 */
        place(newptr, asize);
        return newptr;
    }
    /* 为了确保free正常,设置为allocated */
    PUT(HDRP(newptr), PACK(new_size, 1));
    PUT(FTRP(newptr), PACK(new_size, 1));
    
    // 2.如果不能用,需要malloc+free
    newblk_ptr = mm_malloc(size); /* malloc 里面会让size变成size + 2 */
    if (newblk_ptr == NULL)
      return NULL;

    copySize = GET_SIZE(HDRP(ptr)) - DSIZE;
    if (size <= copySize)
      copySize = size;

    memcpy(newblk_ptr, newptr, copySize);
    mm_free(newptr);
    return newblk_ptr;
}














