// 수업

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUF 2
#define MAX_LOOP 32

int buffer[MAX_BUF];
int counter = 0;
int in = -1, out = -1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_has_space = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_has_data = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    int i, j;
    unsigned long data;

    for (i = 0; i < MAX_LOOP; i++)
    {
        srand(time(NULL));
        for (j = 0; j < rand() % 0xFFFFFFFF; j++)
            ;

        data = (unsigned long)j;

        pthread_mutex_lock(&mutex);

        if (counter == MAX_BUF)
        {
            printf("prod. full!\n");
            pthread_cond_wait(&buffer_has_space, &mutex);
        }

        in++;
        in %= MAX_BUF;
        buffer[in] = (int)data;
        counter++;

        printf("in: %d, counter: %d, data: %ld\n", in, counter, data);

        pthread_cond_signal(&buffer_has_data);

        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg)
{
    int i, j;
    unsigned long data;

    for (i = 0; i < MAX_LOOP; i++)
    {
        pthread_mutex_lock(&mutex);

        if (counter == 0)
        {
            printf("prod. empty!\n");
            pthread_cond_wait(&buffer_has_data, &mutex);
        }

        out++;
        out %= MAX_BUF;
        data = (unsigned long)buffer[out];
        counter--;

        printf("out: %d, counter: %d, data: %ld\n", out, counter, data);

        pthread_cond_signal(&buffer_has_space);

        pthread_mutex_unlock(&mutex);

        srand(time(NULL));
        for (j = 0; j < rand() % 0xFFFFFFFF; j++)
            ;
    }
}

int main(void)
{
    int i;
    pthread_t threads[2];
    pthread_create(&threads[0], NULL, &producer, NULL);
    pthread_create(&threads[1], NULL, &consumer, NULL);
    for (i = 0; i < 2; i++)
        pthread_join(threads[i], NULL);
    return 0;
}