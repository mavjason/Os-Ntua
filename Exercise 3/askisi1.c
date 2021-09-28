pthread_mutex_t count_lock; 
 
void *increase_fn(void *arg) { 
 int i; 
 volatile int *ip = arg; 
  
 fprintf(stderr, "About to increase variable %d times\n", N); 
 for (i = 0; i < N; i++) { 
  if (USE_ATOMIC_OPS) { 
   __sync_add_and_fetch(ip, 1); 
  } else { 
   pthread_mutex_lock(&count_lock); 
   ++(*ip); 
   pthread_mutex_unlock(&count_lock); 
  } 
 } 
 fprintf(stderr, "Done increasing variable.\n"); 
 
 return NULL; 
} 
 
void *decrease_fn(void *arg) { 
 int i; 
 volatile int *ip = arg; 
 
 fprintf(stderr, "About to decrease variable %d times\n", N); 
 for (i = 0; i < N; i++) { 
  if (USE_ATOMIC_OPS) { 
   __sync_sub_and_fetch(ip, 1); 
  } else { 
   pthread_mutex_lock(&count_lock); 
   --(*ip); 
   pthread_mutex_unlock(&count_lock); 
  } 
 } 
 fprintf(stderr, "Done decreasing variable.\n"); 
  
 return NULL; 
}