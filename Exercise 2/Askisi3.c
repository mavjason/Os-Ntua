#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
 
#include "tree.h" 
#include "proc-common.h" 
 
void fork_procs(struct tree_node *root) 
{ 
 if (root->nr_children==0) { 
  /*leaf*/ 
  change_pname(root->name); 
  printf("%s: Starting...\n", root->name); 
  printf("%s: SigStopping...\n", root->name); 
  raise(SIGSTOP); 
 
  exit(2);   /*exit status: leaf->2*/ 
 } 
 else { 
  /*non-leaf*/ 
  pid_t pid[root->nr_children]; 
  int i; 
 
  change_pname(root->name); 
  printf("%s: Starting...\n", root->name); 
 
  for (i=0; i<root->nr_children; i++) { 
   pid[i] = fork(); 
   if (pid[i] < 0) { 
perror("fork_procs: fork"); 
exit(1); 
} 
   else if (pid[i] == 0) { 
    fork_procs(root->children + i); 
    exit(1); 
   } 
   wait_for_ready_children(1); 
  } 
 
  printf("%s: SigStopping...\n", root->name); 
  raise(SIGSTOP); 
   
  for (i=0; i<root->nr_children; i++) 
kill(pid[i], SIGCONT); 
 
  int status; 
  for (i=0; i<root->nr_children; i++) { 
   pid[i] = wait(&status); 
   explain_wait_status(pid[i], status); 
  } 
 
  exit(1);   /*exit status: non-leaf->1*/ 
   
 9 
 } 
} 
 
int main(int argc, char *argv[]) 
{ 
 pid_t pid; 
 int status; 
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
 if (pid == 0) { 
  fork_procs(root); 
  exit(1); 
 } 
 
 wait_for_ready_children(1); 
 
 show_pstree(pid); 
 
 kill(pid, SIGCONT); 
 
 pid = wait(&status); 
 explain_wait_status(pid, status); 
 
 return 0; 
}