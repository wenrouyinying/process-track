#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>     /* For getopt() execvp()*/
#include <sys/stat.h>
#include "error.h"
#include <sys/time.h>
#include <glob.h>
#define MAXLINE 8192
enum
     {
      EXIT_CANCELED = 125, /* Internal error prior to exec attempt.  */
      EXIT_CANNOT_INVOKE = 126, /* Program located, but not usable.  */
      EXIT_ENOENT = 127 /* Could not find program to exec.  */
    }; 
struct Proc {
    long uid, pid, ppid, pgid;
//char name[32], cmd[MAXLINE];
    int  print;
    long parent, child, sister;
    unsigned long thcount;
    char mem[32];
 } *P;

int Root_pid;
int Root_index;
int process_num;

long long cut_cpu_time;

void GetProcesses(void) {
    glob_t globbuf; 
    glob("/proc/[0-9]*", GLOB_NOSORT, NULL, &globbuf);  
    printf("test");
    printf("Root pid : %d",Root_pid);
    process_num = (int)globbuf.gl_pathc;
    printf("all process counts : %d",process_num);
    printf("all process counts : %ld",globbuf.gl_pathc);
    P = calloc(globbuf.gl_pathc, sizeof(struct Proc));
    int i,j;
    cut_cpu_time = 0;

    FILE *cpu_stat;
    cpu_stat = fopen("/proc/stat","r");
    long unsigned int cpu_time[10];
    fscanf(cpu_stat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                &cpu_time[0], &cpu_time[1], &cpu_time[2], &cpu_time[3],
                &cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7],
                &cpu_time[8], &cpu_time[9]);

    fclose(cpu_stat);

    for(int i=0; i < 10;i++)
        cut_cpu_time += cpu_time[i];

    printf ("cputime is :%lld\n",cut_cpu_time);
    for (i = j = 0; i < globbuf.gl_pathc; i++) {
        char *pdir,statfile[32];
        FILE *stat;

        pdir = globbuf.gl_pathv[globbuf.gl_pathc -i - 1];
    
 //       printf("process file : %s",pdir);
        snprintf(statfile,sizeof(statfile),"%s%s",pdir,"/stat");
   //     printf("process stat file : %s\n",statfile);
        stat=fopen(statfile,"r");
        if (fscanf(stat,"%ld %*s %*c %ld",&P[j].pid,&P[j].ppid) != 2 ) {
            printf ("fscanf error");
        }
     //   printf("process %ld parent process id is : %ld",P[j].pid,P[j].ppid);

        if (P[j].pid == Root_pid) {
            Root_index = j;
            printf("root process %d index is : %d",Root_pid,Root_index);
        }
        P[j].parent = P[j].child = P[j].sister = -1;
        P[j].mem[0] = '\0';
        fclose(stat);
        j++;
    }
}

void MakeTrees(void){
    int i;

    printf("testing\n");
    printf("get process number is : %d \n ",process_num);
    for (i = 0; i < process_num; i++) {
        printf("i = %d\n",i);
        printf("xxxxxxxxxxxx\n");
        int parent;
        for (parent = process_num - 1; parent >= 0 && P[parent].pid != P[i].ppid; parent--);
        if (parent != i && parent != -1) {
            P[i].parent = parent;
            printf("parent = %ld\n",P[i].parent);
            printf("child = %ld\n",P[i].child);
            if(P[parent].child == -1) {
                P[parent].child = i;
            } else {
                int sister;
    
                int a =  P[parent].child;
    
                printf("test : a is %d\n", a);
                printf("test : sister is %ld\n", P[a].sister);
                for (sister = P[parent].child; P[sister].sister != -1; sister = P[sister].sister){
                    printf("test : sister is %d\n",sister);
                }
                P[sister].sister = i;
            }

            printf("ending  i = %d\n",i);
        }
    } 
}

struct TreeChars {
    char *s2,         /* SS String between header and pid */
      *p,         /* PP dito, when parent of printed childs */
      *pgl,       /* G  Process group leader */
      *npgl,      /* N  No process group leader */
      *barc,      /* C  bar for line with child */
      *bar,       /* B  bar for line without child */
      *barl,      /* L  bar for last child */
      *sg,        /*    Start graphics (alt char set) */
      *eg,        /*    End graphics (alt char set) */
      *init;      /*    Init string sent at the beginning */
};

static struct TreeChars C = {"--",       "-+",       "=",    "-",    "|-",    "|_",    "\\",   "",     "",     ""};


void PrintTree(int idx, const char *head){
    char nhead[MAXLINE], out[4 * MAXLINE], thread[16] = {'\0'};
    int child;

    char procdir[16],smapfile[32];
    printf("pid is %ld \n",P[idx].pid);
    int pid = P[idx].pid;
    snprintf(procdir,sizeof(procdir),"/proc/%d",pid);
    printf("porcdir is %s \n",procdir);
    //sprintf(procdir,"%s%ld","/proc/",P[idx].pid);
    struct stat st;
    int process_mem = 0,clean_mem = 0,dirty_mem = 0;
    
    if (stat(procdir, &st) != 0) { 
        return;
    } else {
        FILE *smap;
        int line = 1;
        char buffer[10240];
        snprintf(smapfile,sizeof(smapfile),"%s/smaps",procdir);
        smap = fopen(smapfile,"r");
        char clean[10],dirty[10];
        while (fgets(buffer,sizeof(buffer),smap)) {
            if ((line - 9)%23 == 0) {
                strncpy(clean, buffer + 14, 10);
                clean_mem += atoi(clean);
            } else if ((line - 10)%23 == 0) {
                strncpy(dirty, buffer + 14, 10);
                dirty_mem += atoi(dirty);
            }

            line++;
        }

        fclose(smap);
    }

    process_mem = clean_mem + dirty_mem;

    if (process_mem < 1024 ) {
        snprintf(P[idx].mem,sizeof(P[idx].mem),"%d KB",process_mem);
    } else if (process_mem < 1024*1024) {
        snprintf(P[idx].mem,sizeof(P[idx].mem),"%d MB",process_mem/1024);
    } else {
        snprintf(P[idx].mem,sizeof(P[idx].mem),"%d GB",process_mem/1024/1024);
    }

    snprintf(out, sizeof(out),
        "%s%s%s%s%s%s %05ld %s %s%s",
        C.sg,
        head,
        head[0] == '\0' ? "" : P[idx].sister != -1 ? C.barc : C.barl,
        P[idx].child != -1       ? C.p   : C.s2,
        C.npgl,
        C.eg,
        P[idx].pid,P[idx].mem,"","");
    int Columns = 4*MAXLINE;
    out[Columns-1] = '\0';
    puts(out);

    snprintf(nhead, sizeof(nhead), "%s%s ", head,
             head[0] == '\0' ? "" : P[idx].sister != -1 ? C.bar : " ");

    for (child = P[idx].child; child != -1; child = P[child].sister) {
        PrintTree(child, nhead);
    } 

}


int main (int argc, char **argv) {
    int opt;
    printf("argc = %d\n", argc);
    char *command_line [100];
    char *c;
    int saved_errno;
    char *token;
    char *ss = C.s2;
    printf ("CCCCCCC is %s\n",ss);
    while ((opt = getopt(argc, argv, "o:c:")) != EOF) 
        switch(opt) {
            case 'o':
                printf("optint o: output file name is %s\n",optarg);
                break;
            case 'c':
                printf("run %s\n",optarg);
                c = optarg;
                break;
            default:
                printf("not supported option %s\n",optarg);
                break;
        }

    token = strtok(c, " ");
    int i = 0;
    while (token != NULL) {
        command_line[i++] = token;
        token = strtok(NULL, " ");
    }
    command_line[i++] = NULL;
    pid_t pid = fork();
    if (pid < 0) {
        printf("Failed to fork\n");
    } else if (pid == 0) {
        printf ("child PID IS : %d\n",getpid());
        execvp(command_line[0],command_line);
    } else if (pid > 0) {

        Root_pid = pid;
        printf ("child PID IS : %d\n",Root_pid);
        printf("go to parent process\n");
        while (1) {
            printf("run getprocess\n");
            GetProcesses();
            printf("run makeTree\n");
            int i;
            for (i=0;i < process_num;i++) {
                printf ("process %ld,parent %ld,child %ld,sister %ld\n",P[i].pid,P[i].parent,P[i].child,P[i].sister);
            }
            MakeTrees();
            for (i=0;i < process_num;i++) {
                printf ("process %ld,parent %ld,child %ld,sister %ld\n",P[i].pid,P[i].parent,P[i].child,P[i].sister);
            }
            printf("Root index is %d\n",Root_index);
            PrintTree(Root_index,"");
            free(P);
            sleep(10);
        }

    } else {
        printf("Failed to fork\n");
        exit(1);
    }

}


