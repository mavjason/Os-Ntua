#include <errno.h> 
#include <pthread.h> 
#include <stdint.h> 
#include <semaphore.h> 
 
#define perror_pthread(ret, msg) \ 
 do { errno = ret; perror(msg); } while (0)
 int n; 
sem_t *sem; 
 
void *compute_and_output_mandel_line(void *arg) 
{ 
 int i, j, color_val[x_chars]; 
        j = *((int *) arg); 
 
 for (i = j; i<y_chars; i+=n) { 
  compute_mandel_line(i, color_val); 
 
  sem_wait(&sem[j]); 
  output_mandel_line(1, color_val); 
  sem_post(&sem[(j+1) % n]); 
 } 
 
 return NULL; 
} 
 
int main(int argc, char *argv[]) 
{ 
 if (argc!=2) { 
  perror("one argument expected"); 
  exit(1); 
 } 
 
 n = atoi(argv[1]); 
  
 int i, ret, i2[n]; 
 pthread_t t[n]; 
 sem=malloc(sizeof(sem_t)*n); 
 
 sem_init(&sem[0], 0, 1); 
 for (i=1; i<n; i++) sem_init(&sem[i], 0, 0); 
 
 xstep = (xmax - xmin) / x_chars; 
 ystep = (ymax - ymin) / y_chars; 
 
 for (i=0; i<n; i++) { 
  i2[i]=i; 
  ret = pthread_create(&t[i], NULL,  
    compute_and_output_mandel_line, i2+i); 
  if (ret) { 
   perror_pthread(ret, "pthread_create"); 
   exit(1); 
  } 
 } 
 
 for (i=0; i<n; i++) { 
  ret = pthread_join(t[i], NULL); 
  if (ret) perror_pthread(ret, "pthread_join"); 
 } 
 
 for (i=0; i<n; i++) sem_destroy(&sem[i]); 
 
 reset_xterm_color(1); 
 return 0; 
}