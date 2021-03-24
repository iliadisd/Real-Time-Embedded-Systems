/*
 *	Original File	: pc.c
 *
 *  Revised File : prod-cons_8811.c
 *
 *	Title	: Demo Producer/Consumer.
 *
 *	Short	: A solution to the producer consumer problem using
 *		pthreads.
 *
 *	Author	: Andrae Muys
 *
 *	Date	: 18 September 1997
 *
 *	Revised by : Dimosthenis Iliadis-Apostolidis
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

//#define P 1
//#define Q 64
#define QUEUESIZE 64
#define LOOP 100000
#define numfunc 5
#define numargs 10
#define pi 3.1415926535

int P, Q;
void *producer (void *arg);
void *consumer (void *arg);
void *f1(void *arg);
void *f2(void *arg);
void *f3(void *arg);
void *f4(void *arg);
void *f5(void *arg);
void * (*functions[numfunc]) (void * ) = {f1, f2, f3, f4, f5};
int arguments[numargs];

typedef struct {
  void * (*work)(void *);
  void * arg;
} workFunction ;

typedef struct {
  workFunction  buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

struct timeval arrival[QUEUESIZE];
int doneP = 0;
double sum = 0;
queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, workFunction in);
void queueDel (queue *q, workFunction *out);

void *producer (void *q){
	//FILE *output=fopen("output.txt", "w");
	queue *fifo;
  int i;
  fifo = (queue *)q;
  int randfunc, randarg;
  for (i = 0; i < LOOP; i++) {
    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
			//fprintf (output, "producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
    workFunction func ;
    randfunc = rand()%numfunc;
    randarg = rand()%numargs;
    func.work = functions[randfunc];
    func.arg = &arguments[randarg];
    gettimeofday(&(arrival[(fifo->tail)]),NULL);
    queueAdd (fifo, func);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
  }
  doneP++;
  if(doneP == P) {
    pthread_cond_broadcast(fifo->notEmpty);
  }
  return (NULL);
}

void *consumer (void *q)
{
  //FILE *output=fopen("output.txt", "w");
  queue *fifo;
  int i, d;
  fifo = (queue *)q;
  double time_elapsed;
  while(1) {
    pthread_mutex_lock (fifo->mut);
    while(fifo->empty==1 && doneP!=P) {
		    //fprintf (output, "consumer: queue EMPTY.\n");
        pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }
    if(doneP==P && fifo->empty==1) {
      pthread_mutex_unlock (fifo->mut);
      pthread_cond_broadcast(fifo->notEmpty);
      break;
    }
    workFunction func ;
    struct timeval send ;
    gettimeofday(&send,NULL);
    time_elapsed = (double)(send.tv_sec -(arrival[fifo->head]).tv_sec) * 1e6 + (send.tv_usec-(arrival[fifo->head]).tv_usec) ;
    queueDel (fifo, &func);
    // printf("%lf\n",time_elapsed);
    sum += time_elapsed;
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notFull);
    func.work(func.arg);
  }
  return (NULL);
}

queue *queueInit (void)
{
  queue *q;
  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);
  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q,workFunction in)
{
  q->buf[q->tail] = in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;
  return;
}

void queueDel (queue *q, workFunction *out)
{
  *out = q->buf[q->head];
  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;
  return;
}

int main (int argc , char *argv[])
{
	do {
			 printf("Enter the number of producer threads :" );
			 scanf("%d", &P);
	 } while (P <= 0);
	 do {
				printf("Enter the number of consumer threads :" );
				scanf("%d", &Q);
		} while (Q <= 0);

  srand(time(NULL));
  for (int i = 0; i < numargs; i++){
    arguments[i] = rand() % 100 + 1;
  }
  queue *fifo;
  pthread_t prod[P], cons[Q];
  fifo = queueInit ();
	//FILE *output=fopen("output.txt", "w");
  if (fifo ==  NULL) {
    //fprintf (stderr, "main: Queue Initialization failed.\n");
    exit (1);
  }
  for(int i=0; i<Q; i++){
    pthread_create(&cons[i], NULL, consumer, fifo);
  }
  for(int i=0; i<P ; i++){
    pthread_create (&prod[i], NULL, producer, fifo);
  }
  for(int i=0; i<P ; i++){
    pthread_join(prod[i], NULL);
  }
  for(int i=0; i<Q ; i++){
    pthread_join(cons[i], NULL);
  }
  double mean = ((double)sum/(P*LOOP));
	//fprintf(output, "\nExecution ended. Mean Time: %f usec\n", mean);
	printf("Execution ended. Mean Time: %f usec\n", mean);
  queueDelete(fifo);
  //fclose(output);
  return 0;
}

void * f1(void * arg){
  int n = *(int *)arg;
  //printf("This is the 1st function. \nI'm calculating the sum of all numbers 'till %d.\n", x);
  int sum = 0;
  for (int i=1; i<n; i++){
    sum += i;
  }
  //printf("The sum 'till %d is %d.\n\n", n, sum);
}

void * f2 ( void * arg){
  int n = *(int *)arg;
  //printf("This is the 2nd function. \nI'm calculating the factorial of a random integer between 1 and 50.\n");
	int i = 1;
	unsigned long long factorial = 1;
  if (n > 50){
    n = rand() % 50 + 1;
  }
	for (i = 1; i < n + 1; i++){
		factorial *= i;
	}
	//printf("The factorial of number %d is %llu.\n\n", n, factorial);
}

void * f3(void *arg){
  int r = *(int *)arg;
  //printf("This is the 3rd function. \nI'm calculating random circle areas with radius between 1 and 100.\n");
	double area = pi*r*r;
	//printf("The area of a circle with radius %d is %f.\n\n", r,area);
}

void * f4(void *arg){
  int x = *(int *)arg;
  //printf("This is the 4th function. \nI'm calculating random integer's square root.\n");
  double root = sqrt(x);
  //printf("The square root of %d is %f.\n\n", x, root);
}

void * f5(void *arg){
  int x = *(int *)arg;
  //printf("This is the 5th function. \nI'm calculating the ln of an integer between 1 and 100.\n");
	double ln = log(x);
	//printf("ln(%d) = %f.\n\n", x, ln);
}
