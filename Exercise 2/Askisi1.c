#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
 
#include "proc-common.h" 
 
#define SLEEP_PROC_SEC  10 
#define SLEEP_TREE_SEC  3 
 
void fork_procs(void) 
{ 
 pid_t pid, pid2, pid3; 
 int status; 
 
 change_pname("A"); 
 printf("A: Starting...\n"); 
 
 pid = fork(); 
 if (pid < 0) { perror("fork_procs: fork"); exit(1);} 
 else if (pid == 0) { 
  change_pname("B"); 
  printf("B: Starting...\n"); 
 
  pid2 = fork(); 
  if (pid2 < 0) { perror("fork_procs: fork"); exit(1);} 
  else if (pid2 == 0) { 
   change_pname("D"); 
   printf("D: Sleeping...\n"); 
   sleep(SLEEP_PROC_SEC); 
 
   printf("D: Exiting...\n"); 
   exit(17); 
  } 
   
  printf("B: Waiting...\n"); 
    
  pid = wait(&status); 
  explain_wait_status(pid, status); 
 
  printf("B: Exiting...\n"); 
  exit(19);    
 } 
 
 pid3 = fork(); 
 if (pid3 < 0) { perror("fork_procs: fork"); exit(1);} 
 else if (pid3 == 0) { 
  change_pname("C"); 
  printf("C: Sleeping...\n"); 
  sleep(SLEEP_PROC_SEC); 
 
  printf("C: Exiting...\n"); 
  exit(13); 
 } 
 printf("A: Waiting...\n"); 
 
 pid = wait(&status); 
 explain_wait_status(pid, status); 
 
 pid3 = wait(&status); 
 explain_wait_status(pid3, status); 
 
 printf("A: Exiting...\n"); 
 exit(16); 
}
int main(void) 
{ 
 pid_t pid; 
 int status; 
 
 pid = fork(); 
 if (pid < 0) { 
  perror("main: fork"); 
  exit(1); 
 } 
 if (pid == 0) { 
  fork_procs(); 
  exit(1); 
 } 
 
 sleep(SLEEP_TREE_SEC); 
 
 show_pstree(pid); 
 
 pid = wait(&status); 
 explain_wait_status(pid, status); 
 
 return 0; 
}