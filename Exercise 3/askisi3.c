struct kgarten_struct {
int vt;
int vc;
int ratio;
pthread_mutex_t mutex;
pthread_cond_t cond;
};
void child_enter(struct thread_info_struct *thr)
{
if (!thr->is_child) {
fprintf(stderr, "Internal error: %s called for a Teacher
thread.\n", __func__);
exit(1);
}
pthread_mutex_lock(&thr->kg->mutex);
while (thr->kg->vc+1 > thr->kg->vt * thr->kg->ratio)
pthread_cond_wait(&thr->kg->cond, &thr->kg->mutex);
++(thr->kg->vc);
pthread_mutex_unlock(&thr->kg->mutex);
fprintf(stderr, "THREAD %d: CHILD ENTER\n", thr->thrid);
}
void child_exit(struct thread_info_struct *thr)
{
if (!thr->is_child) {
fprintf(stderr, "Internal error: %s called for a Teacher 
thread.\n", __func__);
exit(1);
}
fprintf(stderr, "THREAD %d: CHILD EXIT\n", thr->thrid);
pthread_mutex_lock(&thr->kg->mutex);
--(thr->kg->vc);
pthread_cond_broadcast(&thr->kg->cond);
pthread_mutex_unlock(&thr->kg->mutex);
}
void teacher_enter(struct thread_info_struct *thr)
{
if (thr->is_child) {
fprintf(stderr, "Internal error: %s called for a Child
thread.\n", __func__);
exit(1);
}
fprintf(stderr, "THREAD %d: TEACHER ENTER\n", thr->thrid);
pthread_mutex_lock(&thr->kg->mutex);
++(thr->kg->vt);
pthread_cond_broadcast(&thr->kg->cond);
pthread_mutex_unlock(&thr->kg->mutex);
}
void teacher_exit(struct thread_info_struct *thr)
{
if (thr->is_child) {
fprintf(stderr, "Internal error: %s called for a Child 
thread.\n", __func__);
exit(1);
}
pthread_mutex_lock(&thr->kg->mutex);
while (thr->kg->vc > (thr->kg->vt-1) * thr->kg->ratio)
pthread_cond_wait(&thr->kg->cond, &thr->kg->mutex);
--(thr->kg->vt);
pthread_mutex_unlock(&thr->kg->mutex);
fprintf(stderr, "THREAD %d: TEACHER EXIT\n", thr->thrid);
}
