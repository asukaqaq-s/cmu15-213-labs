/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */
#define JOBS 4
/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        
        case 'h':             /* print help message */
                usage();
	    
            break;

        case 'v':             /* emit additional diagnostic info */
                verbose = 1;
	    
            break;
    
        case 'p':             /* don't print a prompt */
                emit_prompt = 0;  /* handy for automatic testing */
	    
            break;
	
    
        default:
            usage();
	    }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        /* Read command line from stdin to cmdline */
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{ /* 分为 
   1. 内置命令: 调用build-in command
   2. 可执行文件: 调用子进程执行, 如果是前台进程,在前台执行并等待(因此shell前台只会有一个作业)
         如果是后台进程, 在后台执行无需等待
   3. 每个进程都应该有自己的pgid,防止被一锅端*/
    char *argv[MAXARGS]; /* Arguent list*/
    char buf[MAXLINE];  /* holds modified command line*/
    int is_bg;
    pid_t pid;
    sigset_t mask_all, mask_one, prev_one; /*mask_all 所有block位
    mask_one 单个block位  prev_one 单个unblock位*/

    strcpy(buf, cmdline);
    is_bg = parseline(cmdline,argv); /* 是否是后台进程 */
    if (argv[0] == NULL)
        return;
    
    sigfillset(&mask_all);   /* 阻塞全部信号 */
    sigemptyset(&mask_one);  /* 初始化为0 */
    sigaddset(&mask_one, SIGCHLD); /* 阻塞SIGCHLD信号*/
    

    if (!builtin_cmd(argv)) { /* 没有对应的内置命令 */
        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
        if ((pid = fork()) == 0) { /* 子进程返回0*/
            sigprocmask(SIG_SETMASK, &prev_one, NULL); /* unblock*/
            if(execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        else { /* 父进程返回pid */
            sigprocmask(SIG_BLOCK, &mask_all, NULL); /* 阻塞全部信号*/
            setpgid(pid, pid);   /* 将pgid设置为自己的pid */
            int status = (is_bg) ? BG : FG;
            addjob(jobs, pid, status, cmdline);
            sigprocmask(SIG_SETMASK, &prev_one, NULL); /* 返回成未设置之前 */
        }

        /* Parent waits for foreground job to terminate*/
        if(!is_bg) {
        /* 等待所有的前台进程*/
            waitfg(pid);
        } else { 
            int jid = pid2jid(pid);
            printf("[%d] (%d) %s", jid, pid, cmdline);
            // deletejob(jobs, pid);
        }
    }

    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 解析命令行,并且将命令行的元素拆分出来放在argv里面。
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces, ‘ ’  */
	    buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\''); /* 找到最后一个'的下标,作为结尾 */
    }
    else {
	    delim = strchr(buf, ' '); /* 第一个空格 */
    }

    while (delim) { /* 直到遇到最后一个 '\0',才会退出循环*/
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	    return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	    argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    if(!strcmp(argv[0], "quit"))
        exit(0);
    if(!strcmp(argv[0], "fg")) {
        do_bgfg(argv);
        return 1;
    }
    if(!strcmp(argv[0], "bg")) {
        do_bgfg(argv);
        return 1;
    }
    if(!strcmp(argv[0], "jobs")) {
        listjobs(jobs);
        return 1;
    }
    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    char *argu;
    int jid;
    argu = argv[1];
    
    if (argu != NULL && *argu == '%')
        argu++;
    else if(*argu == 0) {
        printf("%s command requires PID or %%jobid argument\n",argv[0]);
        return;
    }
    jid = atoi(argu);
    struct job_t* job = getjobjid(jobs, jid);
    if(job == NULL) {
        printf("%s: No such job\n",argv[1]);
        return;
    }

    /* 进行对应的bg、fg转换*/
    if(!strcmp(argv[0], "bg")) {
        kill(job->pid, SIGSTOP); /* 立即停止 */
        job->state = BG;
        kill(job->pid, SIGCONT);
        printf("[%d] (%d) %s", job->jid, job->pid, job->cmdline);
    } else if(!strcmp(argv[0], "fg")) {
        if(job->state == ST) { /* 如果最后是stop状态 */
            kill(job->pid, SIGCONT);/* 发射sigcont信号让他苏醒 */
        }
        job->state = FG;
        waitfg(job->pid);
    }
    
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    int status;
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    pid_t res;

    while((res = waitpid(pid, &status, WUNTRACED)) < 0) {
        sleep(1);
    }
    if(res >= 0) {
    /* 如果等待pid的结果<0,表示没有进程终止,出错*/
        // printf("wait========= %d\n",pid);
        if(WIFSTOPPED(status)) { /* 如果是被挂起的*/
            // 被挂起的不需要干活...
        } else {
            sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
            deletejob(jobs, pid);
            sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }
    }
    else {
        unix_error("waitfg: waitpid error");
    }
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    pid_t pid;
    
    sigfillset(&mask_all);
    while((pid = waitpid(-1,NULL,WNOHANG)) > 0) { /* 立即终止,不会继续等待*/
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        deletejob(jobs, pid);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
    if(errno != ECHILD) {
       //  unix_error("waitpid error");
        // printf("%d\n",errno);
    }
    errno = olderrno;
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    pid_t pid = fgpid(jobs);
    if(pid != 0) {
        struct job_t * job = getjobpid(jobs, pid);
        kill(-pid, SIGKILL); /* 直接杀死,负数表示整个组 */
        printf("Job [%d] (%d) terminated by signal %d\n",
                job->jid, job->pid, sig);
        clearjob(job);
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    pid_t pid = fgpid(jobs);
    // printf("pid = %d pgid = %d\n",pid,getpgid(pid));
    if(pid != 0) {
        struct job_t * job = getjobpid(jobs, pid);
        
        job->state = ST;
        kill(-pid, SIGSTOP); /* 暂停*/
        printf("Job [%d] (%d) stopped by signal %d\n", 
            job->jid, job->pid, sig);
    }
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	    clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].state == FG)
	        return jobs[i].pid;
    }
    return 0;
}


/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	    return NULL;
    for (i = 0; i < MAXJOBS; i++) {
	    // printf("%d %d %d\n",jobs[i].pid, jobs[i].jid, jobs[i].state);
        if (jobs[i].pid == pid)
	        return &jobs[i];
    }
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	    return NULL;
    for (i = 0; i < MAXJOBS; i++) 
	    if (jobs[i].jid == jid)
	        return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
                return jobs[i].jid;
            }
    }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



