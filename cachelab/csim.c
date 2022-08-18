#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define false 0
#define true 1
typedef int bool;

/***************************************************************
 * data
 ***************************************************************/

int cache_b = 0;  /* b */
int cache_s = 0;  /* s */
int cache_E = 0;

/***************************************************************
 * Cache_line
 ***************************************************************/
/*作为Cache组中的一行*/
struct Cache_line {
    int valid;    // 有效位
    long tag;     // 标记位
    long time;    // 时间戳 
    long Address; // 起始地址
};

typedef struct Cache_line Cache_line;

/**********************
* Function name: Get_line_tag
* Description:  通过访问的address,找出对应的tag
***********************/
long Get_line_tag(long Address) {
    long tag = Address >> (cache_b + cache_s);
    return tag;
}

/**********************
* Function name: Cache_line_AdrInit
* Description:  通过访问的address,完成对于Cache_line的初始化
***********************/

Cache_line Cache_line_AdrInit(long Address) {
    Cache_line line;
    line.Address = Address;
    line.valid = true;
    line.time = 0;
    line.tag = Get_line_tag(Address);
    return line;
}

/**********************
* Function name: Get_line_Init
* Description:  完成初始化,全部置为0
***********************/
Cache_line Cache_line_Init() {
    Cache_line line;
    line.Address = 0;
    line.valid = 0;
    line.tag = 0;
    line.time = 0;
    return line;
}

/***************************************************************
 * cache
 ***************************************************************/

/* 作为一个Cache模拟器存在*/

struct Cache {
    int hit_count;
    int miss_count;
    int eviction_count;
    Cache_line** array_; /* Cache数组 */
};

typedef struct Cache Cache;

Cache Cache_init() {
    Cache cache;
    cache.hit_count = 0;
    cache.eviction_count = 0;
    cache.miss_count = 0;
    int cache_set_number = 1 << cache_s;
    cache.array_ = (Cache_line**)malloc(sizeof(Cache_line*) * cache_set_number);
    for(int i = 0;i < cache_set_number;i ++) {
        cache.array_[i] = (Cache_line *)malloc(sizeof(Cache_line) * cache_E);
        for(int j = 0;j < cache_E;j ++) {
            cache.array_[i][j] = Cache_line_Init();
        }
    }
    return cache;
}

void Cache_read(Cache *cache,long address,int time) {
    int set_num = address >> cache_b & ((1 << cache_s) - 1); // 组号
    int aim_tag = address >> (cache_b + cache_s);
    int is_found = false;
    for(int i = 0;i < cache_E;i ++) {
        if(!cache->array_[set_num][i].valid)
            continue;
        if(cache->array_[set_num][i].tag == aim_tag) {
            cache->array_[set_num][i].time = time; // 时间记得更新+
            cache->hit_count++;
            is_found = true;
            break;
        }
    }
    // 如果找到了直接return,没找到需要置换
    if(is_found) 
        return;
    // 先从空的开始寻找
    for(int i = 0;i < cache_E;i ++) {
        if(!cache->array_[set_num][i].valid) {
            cache->array_[set_num][i] = Cache_line_AdrInit(address);
            cache->array_[set_num][i].time = time;
            is_found = true;
            break;
        }
    }
    cache->miss_count++;
    if(is_found) return;
    // 还是找不到,找到一个时间戳最小的
    int minn = 1e9;
    int v = -1;
    for(int i = 0;i < cache_E;i ++) {
        if(cache->array_[set_num][i].time < minn) {
            minn = cache->array_[set_num][i].time;
            v = i;
        }
    }
    cache->array_[set_num][v] = Cache_line_AdrInit(address);
    cache->array_[set_num][v].time = time;
    cache->eviction_count++;
}

void Free_cache(Cache* cache) {
    int set_nums = 1 << cache_s;
    for(int i = 0;i < set_nums;i ++)
        free(cache->array_[i]);
    free(cache->array_);
}
/***************************************************************
 * function 
 ***************************************************************/

/*************
 * read_argument: 读入给定的参数
 *************/
FILE* read_argument(int argc,char **argv) {
    bool is_input_file = false;
    char *input_file_id = NULL;
    /* 1.读入参数和给定的变量
       2.读入文件名
    */
    for(int i = 1;i < argc;i ++) {
        if(!strcmp(argv[i],"-s")) {
            i++;
            cache_s = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],"-E")) {
            i++;
            cache_E = atoi(argv[i]); 
            // 由于有多个数字,直接atoi
        }
        else if(!strcmp(argv[i],"-b")) {
            i++;
            cache_b = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],"-t")) {
            is_input_file = true;
            input_file_id = argv[++i];
        }
    }
    /* 读取trace文件,返回对应的指针*/
    FILE* input_file = fopen(input_file_id,"r");
    
    /* 如果没打开指针或者变量没给齐,直接退出*/
    if(!is_input_file || !cache_b || !cache_E 
    || !cache_s || input_file == NULL) {
        puts("you should check on input argument!!!");
        return NULL;
    }
    return input_file;
}


void solve(FILE* file) {
    Cache cache = Cache_init();
    
    char op = 0;
    long address = 0,len = 0;
    int time = 0;
    while(fscanf(file," %c %lx,%ld",&op,&address,&len) > 0) {
        time++;
        if(op == 'I') continue;
        if(op == 'M') {
            Cache_read(&cache,address,time);
            cache.hit_count++;
        }
        else {
            Cache_read(&cache,address,time);
        }
    }
    printSummary(cache.hit_count, cache.miss_count,
    cache.eviction_count);
    Free_cache(&cache);
}

int main(int argc,char **argv)
{
    FILE * input_file = read_argument(argc,argv);
    if(input_file == NULL) {
        return 0;
    }
    
    solve(input_file);

    fclose(input_file);
    return 0;
}
