#include <stdio.h>
#include <string.h>
/**************************
 * 单个元素,sizeof 为 16
 * ************************
*/

struct node {
    int val;
    int order;
    struct node* next;
}list;
typedef struct node node;

void explode_bomb() {
}

void solve() {
    /* 我们输入的数组*/
    int arr[] = {1,4,5,2,3,6};
	/* part1: 检查所有数字是否 <= 6,并且和前一个不同*/
    {
        int r12 = 0,ebx = 0;
        for(r12 = 0;r12 <= 5;r12 ++ ) {
            if(arr[r12] > 6) 
                explode_bomb();
            for(ebx = r12 + 1;ebx <= 5;ebx ++ ) {
                if(arr[ebx] == arr[r12])
                    explode_bomb();
            }
        }
    }
    /* part2: 让所有数字x变为 7 - x*/
    {   
        long rsi = arr + 6 * 4;
        long rax = arr;
        for(rax = arr;rax != rsi;rax ++ ) { // 等价于 +=4
            *(int*)rax = 7 - *(int*)rax;
        }
    }
    /* part3: 找到arr数组元素在链表中的位置,并将链表中该位置的
    地址存储在栈帧中 */
    {
        int rsi = 0;        // 当前遍历的次数 * 4
        int rcx = 0;       // 遍历输入的元素值 
        long rsp = arr;    // 栈指针,栈帧起点是arr,低地址向高存储
        node rdx = list;  // 全局list指向的地址
        int rax = 0;       // 当前遍历次数
        for(rsi = 0;rsi != 0x18;rsi += 4) {
            long store_find_address = rsp + 0x20 + 2*rsi;  // 存储找到的地址
            int rcx = *((int *)rsp + rsi);
            rdx = list;
            rax = 0;   
            if(rcx <= 1) { // 直接在第一个不需要遍历找了
                rdx = list;
            }
            else { // 需要循环遍历,找到目标值
                rax = 1;
                while(rax != rcx) {
                    rax ++;
                    rdx = *rdx.next;    // 第一位肯定不是,直接跳过
                }
            }
            *((node *)store_find_address) = rdx;
            /* store_find_address 存储链表的地址,而不是val值*/
        }
    }
    /* part4: 利用func3存储的地址,形成一个新的链表!!! */
    {
        long rsp = arr;    // 栈指针,栈帧起点是arr,低地址向高存储
        node rsi = *((node *)(rsp + 0x50)); // 遍历的终点(第一个store_find_address)
        node rbx = *((node *)(rsp + 0x20)); // 遍历的起点
        node rax = *((node *)(rsp + 0x28)); // 总是指向当前rbx的下一位
        
        node rcx = rbx;
        for(;rax != rsi;rax = *rax.next) {
        /* rcx 表示当前位置,rax 下一位置
           循环体做的：rcx.next = rax
        */
            node rdx = rax;   
            rcx.next = &rdx;   // 修改next   
            rcx = *rcx.next;  // 链表跳转到下一位置
        }
    }
    /* part5: 判断func3中存储的地址,按照顺序遍历是不是降序排序
    (从大到小),不是的话 BOMB!!!*/
    {
        int ebp = 5; // 循环次数,比较是否降序只需要五次
        long rsp = arr;    // 栈指针,栈帧起点是arr,低地址向高存储
        node rbx = *((node*)(rsp + 0x20)); // 遍历的起点
        while(ebp -- ) {
            int rax = (*(rbx.next)).val;
            if((rbx).val <= rax)
                explode_bomb();
            rbx = *rbx.next;
        }
    }

    /* 执行完毕！！！*/
}

int strings_not_equal(char *a,char *b) {

}

int read_six_numbers(char *input,int *arr) {
}

void phase_1() {
    long rsi = 0x402400;
    long rdi = 6305664;
    if(strings_not_equal((char*)rdi,(char*)rsi)) {
        explode_bomb();
    }
}

void phase_2() {
    char * input = NULL; // input 在phase_2调用前就读入过了
    int arr[6];
    read_six_numbers(input,arr);    // 从input中读入,并存储在arr中
    
    if(arr[0] != 1) 
        explode_bomb(); 

    long rbp = arr + 24;    // 终点
    long rbx = arr + 4;

    for(;rbx != rbp;rbx += 4) {
        long rax = rbx - 4;
        *((int*)rax) = *((int*)rax) + *((int*)rax);
        if(*((int*)rbx) != *((int*)rax))
            explode_bomb();
    } 
    
}

void phase_3() {
    char *input;
    int a,b;
    int t = sscanf(input,"%d %d",&a,&b);
    if(t <= 1) explode_bomb();
    if(a > 7) 
        explode_bomb();
    int rax = 0;    // 存储对应的密码
    if(a > 7) explode_bomb();

    switch (a)
    {
    case 1: rax = 0xcf;
        break;
    case 2: rax = 0x2c3;
        break;
    case 3: rax = 0x100;
        break;
    case 4: rax = 0x185;
        break;
    case 5: rax = 0xce;
        break;
    case 6: rax = 0x2aa;
        break;
    case 7: rax = 0x147;
        break;
    default:
        rax = 0x137;
        break;
    }
    if(b != rax)
        explode_bomb();
}


int func4(int rdi,int rsi,int rdx) {
// rdi 表示变量a,在运行过程中不会改变
    int rax = rdx - rsi;   // 两者差值
    int rcx = (rax) >> 31; // 获得符号位
    rax = (((rax)>>31) + rax) >> 1;
    rcx = rax + rsi; 
/* 
    以上几行的意思是:
    如果 rdx < rsi,rcx = (rdx + rsi + 1)/2
    如果 rdx >= rsi,rcx = (rdx + rsi)/2
*/


    /* 如果 a != rcx,最后返回的不是0,答案就出错了
       因此,只有a只有为7,答案才是正确的
    */
    if(rcx <= rdi) {
        rax = 0;
        if(rcx < rdi) {
            rsi = rcx + 1;
            rax = func4(rdi,rsi,rdx);
            rax = rax + rax + 1;
        }
    } else {
        rdx = rcx - 1;
        rax = func4(rdi,rsi,rdx);
        rax = rax + rax;
    }
    return rax;
}

void phase_4() {
    int a,b;
    char *input;
    int num = sscanf(input,"%d %d",&a,&b);
    if(num != 2) 
        explode_bomb();
    if(a > 14) 
        explode_bomb();
    int res = func4(a,0,14);
    if(res != 0 || b != 0)
        explode_bomb();
}

void phase_5() {
    char * input;
    long rsp =NULL;
    if(strlen(input) != 6) 
        explode_bomb();
    /* part 1*/
    long rdi = input; // 输入的字符串的地址
    long rbx = rdi;   // rbx 指向输入的字符串
    for(int rax = 0;rax < 0;rax ++ ) {
        //rcx = input[rax]
        char rcx = *((char*)(rbx + rax));
        // 修改这个字符rcx
        int rdx = rcx & 0xf;
        int rdx = *((char*)(rdx + 0x4024b0));
        // 将字符写回到栈帧中
        *((char*)rsp + rax) = rdx;
    }
    /* part2*/
    if(strings_not_equal((char*)(rsp + 0x16),(char*)0x40245e))
        explode_bomb();
}

int main() {
    int n = 10;
    scanf("%d",&n);
    int x = func4(n,0,14);
    printf("%d\n",x);
	return 0;
}
