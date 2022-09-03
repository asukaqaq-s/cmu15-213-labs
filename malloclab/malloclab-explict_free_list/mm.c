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
static void* ExtendHeap(size_t);
static void* FindFit(size_t);
static void place(char *bp, size_t asize);
inline void DeleteNode(char *);
inline void AddNode(char *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    
    if((heap_top = mem_sbrk(4 * WSIZE)) == NULL) {
        return -1;
    }
    /* 初始化*/
    PUT(heap_top, PACK(0,0));
    PUT(heap_top + 1 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_top + 2 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_top + 3 * WSIZE, PACK(0, 1));
    heap_top += 2 * WSIZE;
    list_head = NULL;
    /* 扩展heap */
    if(ExtendHeap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    // 1. 首先align size
    // 2. 然后 find_fit 请求申请一个块
    // 3. 如果找不到一个可以使用的块,需要扩展堆
    
    size_t asize;
    size_t extend_size;
    char *bp;
    
    // 1. align size
    if(size == 0)
        return NULL;
    if(size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = DSIZE * ((size + DSIZE + DSIZE - 1)/DSIZE);
    }
    // 2. find_fit
    if((bp = FindFit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    extend_size = MAX(CHUNKSIZE, asize);
    if((bp = ExtendHeap(extend_size/WSIZE)) == NULL) {
        return NULL;    
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    AddNode(bp);
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *old_bp,*new_bp;
    size_t new_block_size; /* after free */
    size_t old_block_size; /* before free */
    size_t asize; /* align size */

    old_bp = ptr;
    old_block_size = GET_SIZE(HDRP(ptr));

    // 1. align size
    
    if(size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1))/DSIZE);
    }

    /* 在判断 block_size >= asize 之前,可能周围有其他空闲块
       因此需要先合并周围空闲块,由于free不会返回一个合并后的指针,所以只调用 coalesce */
    
    // 2. coalesce

    new_bp = coalesce(old_bp);
    new_block_size = GET_SIZE(HDRP(new_bp));
    
    if(new_bp != old_bp) { /* 发生了合并 */
        memcpy(new_bp, old_bp, old_block_size - DSIZE); /* 拷贝原来的内容到new_bp上 */
    }

    // 3. 后两种情况    
    if(new_block_size >= asize) {
        place(new_bp, asize);
        return new_bp;
    }
    
    // 4. 第一种情况
    new_bp = mm_malloc(asize);
    memcpy(new_bp, old_bp, old_block_size - DSIZE); /* 拷贝原来的内容到new_bp上 */
    mm_free(old_bp);
    return new_bp;
}



/********************************************************
* C_FUNCTIONS
*********************************************************/

/**
 * entend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void place(char *bp, size_t asize) {
    size_t blk_size = GET_SIZE((HDRP(bp)));
    size_t bsize = blk_size - asize;
    char *next_p;
    DeleteNode(bp);
    if(bsize < 2 * DSIZE) {
        PUT(HDRP(bp), PACK(blk_size, 1));
        PUT(FTRP(bp), PACK(blk_size, 1));
    } else {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        next_p = NEXT_BLKP(bp);
        PUT(HDRP(next_p), PACK(bsize, 0));
        PUT(FTRP(next_p), PACK(bsize, 0));
        AddNode(next_p);
    }
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
    
    DeleteNode(bp); // 删除bp
    if(prev_allocate && next_allocate) {
        return bp;
    } else if(!prev_allocate && next_allocate) {
        size += prev_size;
        DeleteNode(prev_blk);
        PUT(HDRP(prev_blk), PACK(size, 0));
        PUT(FTRP(prev_blk), PACK(size, 0));
        bp = prev_blk;
    } else if(prev_allocate && !next_allocate) {
        size += next_size;
        DeleteNode(next_blk);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } else if(!prev_allocate && !next_allocate) {
        size += prev_size + next_size;
        DeleteNode(prev_blk);
        DeleteNode(next_blk);
        PUT(HDRP(prev_blk), PACK(size, 0));
        PUT(FTRP(prev_blk), PACK(size, 0));
        bp = prev_blk;
    }
    AddNode(bp);
    return bp;
}

/**
 * extend_heap - extend the heap size blks
 * @return 0 if failed,else return new_blk 's start address
 */
static void* ExtendHeap(size_t words) {
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
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new end blk */
    PUT(LAST_PTR(bp), NULL);
    AddNode(bp);
    return coalesce(bp);
}

static void* FindFit(size_t asize) {
    void *p = list_head;
    size_t size;
    size_t allocate;
    
    while(p != NULL) {
        size = GET_SIZE(HDRP(p));
        allocate = GET_ALLOC(HDRP(p));
        if(!allocate && size >= asize) {
            return p;
        }
        p = NEXT_NODE(p);
    }
    return NULL;
}

inline void DeleteNode(char * bp) {
    void * last_blk = LAST_NODE(bp);
    void * next_blk = NEXT_NODE(bp);
    if(list_head == bp) {
        list_head = NEXT_NODE(bp);
    }
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

inline void AddNode(char *bp) {
    if(list_head == NULL) {
        list_head = bp;
        PUT(NEXT_PTR(bp), NULL);
        PUT(LAST_PTR(bp), NULL);
        return;
    }
    PUT(LAST_PTR(bp), NULL);
    PUT(NEXT_PTR(bp), list_head); /* 下一个结点是head */
    PUT(LAST_PTR(list_head), bp); /* 原head前一个结点是新head */
    list_head = bp;
}

/********************************************************
* TEST_FUNCTIONS
*********************************************************/

static void print_error() {
    printf("error!!!\n");
    exit(0);
}

static void print_bp(char *bp) {

    printf("size = %d allocate = %d address = %p\
    last_blk = %p next_blk = %p\n",
        GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), bp, LAST_NODE(bp), NEXT_NODE(bp));
}

static void print_list() {
    void *p = list_head;
    int cnt = 0;
    printf("print_list start: -------\n");
    while(p != NULL) {
        // printf("[%d] size = %d allocated = %d address = %p\n", ++cnt,
        //     GET_SIZE(HDRP(p)), GET_ALLOC(FTRP(p)), p);
        printf("[%d] ",++cnt);
        print_bp(p);
        p = NEXT_NODE(p);
    }
    printf("print_list end: ---------\n");
}