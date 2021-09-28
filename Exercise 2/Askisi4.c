#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
 
#include "tree.h" 
#include "proc-common.h" 
 
#define SLEEP_PROC_SEC  10 
#define SLEEP_TREE_SEC  3 
 
void fork_procs(struct tree_node *root, int filedesc) 
{ 
 if (root->nr_children==0) { 
  /*leaf*/ 
  int number; 
  change_pname(root->name); 
  number=atoi(root->name); 
   
  if (write(filedesc, &number, sizeof(number)) != 
    sizeof(number)){ 
   perror("Child: write to pipe"); 
   exit(1); 
  } 
 
  sleep(SLEEP_PROC_SEC); 
  exit(2);    /*exit status?? leaf->2*/ 
 } 
 else { 
  /*non-leaf*/ 
  int i, status, pfd[2]; 
  int result, num[2]; 
 
  pid_t pid[root->nr_children]; 
  change_pname(root->name); 
  char op=*(root->name); 
 
  if (pipe(pfd) < 0) { perror("pipe"); exit(1); } 
 
  for (i=0; i<root->nr_children; i++){ 
   pid[i] = fork(); 
   if (pid[i] < 0) { 
perror("fork_procs: fork"); 
exit(1); 
} 
   else if ( pid[i] == 0) { 
    fork_procs(root->children + i,pfd[1]); 
    exit(1); 
   } 
  } 
 
  for (i=0; i<root->nr_children; i++){ 
   if (read(pfd[0], &num[i], sizeof(num[i])) != 
     sizeof(num[i])) {  
 12 
    perror("Parent: read from pipe"); 
    exit(1); 
   } 
  } 
 
  switch(op){ 
   case '+' : 
    result=num[0]+num[1]; 
    break; 
   case '*' : 
    result=num[0]*num[1]; 
    break; 
  } 
 
  if (write(filedesc,&result, sizeof(result)) != 
    sizeof(result)){ 
   perror("Parent: write to pipe"); 
   exit(1); 
  } 
 
  for (i=0; i<root->nr_children; i++){ 
   pid[i] = wait(&status); 
   explain_wait_status(pid[i], status); 
  } 
 
  exit(1);    /*exit status??? non-leaf->1*/ 
 } 
} 
 
int main(int argc, char *argv[]) 
{    
 pid_t pid; 
 int ProcFiledesc[2]; 
 pipe(ProcFiledesc); 
 int status, result; 
 struct tree_node *root; 
 
 if (argc < 2){ 
  fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]); 
  exit(1); 
 } 
 
 root = get_tree_from_file(argv[1]); 
 
 pid = fork(); 
 if (pid < 0) { 
  perror("main: fork"); 
  exit(1); 
 } 
 else if (pid == 0) { 
  fork_procs(root,ProcFiledesc[1]); 
  exit(1); 
 } 
 
if (read(ProcFiledesc[0], &result, sizeof(result)) != 
  sizeof(result)) { 
  perror("main: read from pipe"); 
  exit(1); 
 }
 sleep(SLEEP_TREE_SEC); 
 
 show_pstree(pid); 
 printf("Result: %d\n",result); 
 
 pid = wait(&status); 
 explain_wait_status(pid, status); 
 
 return 0; 
}