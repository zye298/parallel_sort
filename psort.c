#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>

int NUM_THREAD;
pthread_t *threads;
int recordPer_Thread;


//https://piazza.com/class/l7kt5onxp5h4hp/post/1312
typedef struct {
  int key;
  char *pointer;
} key_ptr;

typedef struct {
  int i;//index of last created thread, num of thread equals i + 1;
  key_ptr *lower;
  key_ptr *upper;
} sort_input;


//https://piazza.com/class/l7kt5onxp5h4hp/post/1312
void swap(key_ptr *a, key_ptr *b) {
  key_ptr t = *a;
  *a = *b;
  *b = t;
}
//https://piazza.com/class/l7kt5onxp5h4hp/post/1312
key_ptr *part(key_ptr *lower, key_ptr *upper) {

  key_ptr *l = lower;
  for (key_ptr *c = lower; c < upper; c++)
    if (c->key < upper->key)
      swap(c, l++);
  swap(l, upper);
  return l;
}

//https://piazza.com/class/l7kt5onxp5h4hp/post/1312
void qs(key_ptr *lower, key_ptr *upper) {
  if (lower < upper) {
    key_ptr *pivot = part(lower, upper);
    qs(lower, pivot - 1);
    qs(pivot + 1, upper);
  }
}


void *qs_parallel(void *arg){

  sort_input *param = arg;

  int i = param->i;

  key_ptr *lower = param->lower;

  key_ptr *upper = param->upper;

  //make sure upper > lower
  if (lower < upper) {

    key_ptr *pivot = part(lower, upper);

    //restrict num of thread created
    if ( (pivot - lower)>= recordPer_Thread && (i + 1) < NUM_THREAD  ){

      
      sort_input param = {i, lower, pivot-1};

      i = i + 1;

      int creation_out = pthread_create(&threads[i], NULL,qs_parallel,  &param );

      if (creation_out != 0){

        //thread creation failed.
        fprintf(stderr, "An error has occurred\n");
        exit(0);
      }

      pthread_join(threads[i], NULL);
    }else{

      qs(lower, pivot-1);
    }
    
    //restrict num of thread created
    if ( (upper - pivot)>= recordPer_Thread && (i + 1) < NUM_THREAD  ){

      
      sort_input param = {i, pivot+1, upper};

      i = i + 1;

      int creation_out = pthread_create(&threads[i], NULL,qs_parallel,  &param );

      if (creation_out != 0){

        //thread creation failed.
        fprintf(stderr, "An error has occurred\n");
        exit(0);
      }

      pthread_join(threads[i], NULL);
    }else{

      qs(pivot + 1, upper);
    }
  }

  return 0;
}



int main(int argc, char* argv[]){



    NUM_THREAD = get_nprocs();
    if (argc > 3){
      fprintf(stderr, "An error has occurred.\n");

      exit(0);
    }


    //read in file
    int fd, outd;
    char *ptr_ , *ptr_out;
    struct stat stat_;
    size_t size;

    if ( (fd = open(argv[1], O_RDONLY)) == -1 || ( outd = open(argv[2], O_RDWR | O_CREAT, (mode_t)0600) ) == -1 || (fgetc(fopen(argv[1], "r")) == EOF) ){


      fprintf(stderr, "An error has occurred\n");

      exit(0);
    }

    //https://piazza.com/class/l7kt5onxp5h4hp/post/1312
    fstat(fd, &stat_);
    size = stat_.st_size;

    int temp = ftruncate(outd, (int) size);
    temp++;

    ptr_ = mmap(0, size, PROT_READ|PROT_EXEC ,MAP_SHARED , fd, 0);

    ptr_out = mmap(0, size, PROT_WRITE|PROT_EXEC ,MAP_SHARED , outd, 0);


    size = size/100;

    //https://piazza.com/class/l7kt5onxp5h4hp/post/1312
    key_ptr *key_ptr_map = (key_ptr *)malloc(size * sizeof(key_ptr));
    key_ptr *index = key_ptr_map;

    for (char *r = ptr_; r < ptr_ + size * 100; r += 100) {

        index->key = *(int *)r;
        index->pointer = r;
        index++;
    } 

    //do the sorting
    threads = (pthread_t *)malloc(NUM_THREAD * sizeof(pthread_t));
    recordPer_Thread = size / NUM_THREAD;

    int i = 0;

    sort_input param = {i, key_ptr_map, key_ptr_map + size - 1};

    int output = pthread_create(&threads[i], NULL,qs_parallel,  &param );

    if (output != 0){

      //thread creation failed.
      fprintf(stderr, "An error has occurred\n");
      exit(0);
    }

    pthread_join(threads[i], NULL);



    //write output to file
    char *t = ptr_out;
    for (int i = 0; i < size ; i ++) {
      memcpy(t, key_ptr_map[i].pointer,100);
      t += 100;
    }

    close(fd);
    close(outd);

    return 0;
}


