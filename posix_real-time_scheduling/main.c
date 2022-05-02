#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <linux/sched.h> // 실시간 관련 프로그래밍할 때 define 사용 가능
#include <sys/types.h>
#include <pthread.h>

#include <math.h>
#include <time.h>
#include <stdlib.h>

#define NUM_THREADS 20     // FIFO, RR 아닌 NORMAL 쓰레드
#define NUM_THREADS_FIFO 1 // FIFO 쓰레드
#define NUM_THREADS_RR 1   // RR 쓰레드

/*
    22개의 쓰레드

    20개의 일반 쓰레드 돌리는 중에
    FIFO, RR이 요청했던 시간(sleep 을 씀)이 정확하게 지켜져 주느냐

    연성이라 아주 정확하진 않음
*/

int flag = 0;         // 22개의 쓰레드가 동시에 출발하도록 (방아쇠 역할)
char str_buf[163984]; // printf -> I/O 자주 -> 생각한 것과 다름
                      // 따라서 버퍼에 적어두고 프로그램 끝나면 출력
int buf_idx = 0;

char junk_buf[16384];

void set_my_sched_policy(int my_policy)
{
    int policy;
    pthread_attr_t attr;

    /* get the default attributes */
    pthread_attr_init(&attr);

    /* set the scheduling policy - real-time */
    if (pthread_attr_setschedpolicy(&attr, my_policy) != 0)
    {
        fprintf(stderr, "Unale to set policy.\n");
    }

    /* get the current scheduling policy */
    if (pthread_attr_getschedpolicy(&attr, &policy) != 0)
    // 내가 요청했던 스케줄링 policy로 제대로 설정됐는지 확인 -> get
    {
        fprintf(stderr, "Unable to get policy.\n");
    }
    else
    {
        if (policy == SCHED_OTHER)
            printf("SCHED_OTHER\n");
        else if (policy == SCHED_RR)
            printf("SCHED_RR\n");
        else if (policy == SCHED_FIFO)
            printf("SCHED_FIFO\n");
    }
}

void *runner(void *); // 쓰레드로 호출할 함수의 원형
void *runner_FIFO(void *);
void *runner_RR(void *);

int main(int argc, char *argv[])
{
    int i, policy;

    pthread_t tid[NUM_THREADS];
    pthread_t tid_FIFO[NUM_THREADS_FIFO];
    pthread_t tid_RR[NUM_THREADS_RR];

    // EXECUTING NORMAL THREADS (SCHED_OTHER)
    /* create SCHED_OTHER threads */
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&tid[i], NULL, runner, NULL);
    // tid[i] : 넘어오는 쓰레드 아이디를 담아둠
    // runner : 일반 쓰레드로 호출할 함수

    // EXECUTING FIFO THREADS
    /* create real-time threads */
    for (int i = 0; i < NUM_THREADS_FIFO; i++)
        pthread_create(&tid_FIFO[i], NULL, runner_FIFO, NULL);
    // runner_FIFO 안에 들어가서 스스로를 FIFO 스케줄 해달라고 요청할 것

    // EXECUTING RR THREADS
    /* create real-time threads */
    for (int i = 0; i < NUM_THREADS_RR; i++)
        pthread_create(&tid_RR[i], NULL, runner_RR, NULL);
    // runner_RR 안에 들어가서 스스로를 RR 스케줄 해달라고 요청할 것

    /*
        쓰레드 create 하고 나면, 바로 실행되기 때문에
        그것들을 holding 하기 위하여
        처음에 flag=0으로 초기화
    */

    // RELEASE THREADS
    flag = 1;

    // DO 'JOIN' ON ALL THREADS
    /* now join on each thread */
    /*
    for(int i=0; i<NUM_THREADS; i++){
        pthread_join(tid[i], NULL);
    }
    */
    for (int i = 0; i < NUM_THREADS_FIFO; i++)
    {
        pthread_join(tid_FIFO[i], NULL);
    }
    for (int i = 0; i < NUM_THREADS_RR; i++)
    {
        pthread_join(tid_RR[i], NULL);
    }

    str_buf[buf_idx] = '\0';
    printf("%s\n", str_buf);

    return 0;
}

/* Each thread will begin control in the fuction */
void *runner(void *param)
// 일반 스케줄링 클래스의 쓰레드로 돌릴 함수
{
    int i, policy;
    pthread_attr_t attr;

    while (!flag) // flag 가 0인 동안 그냥 돌리는데,
        sleep(0); // CPU를 많이 낭비하지 않기 위하여, sleep 사용
    // sleep(0) : yield

    set_my_sched_policy(SCHED_OTHER);
    // 일반 쓰레드 -> 리눅스에서 스케줄링 클래스 : SHCED_OTHER

    for (int i = 0; i <= 100; i++)
    {
        for (int j = 0; j <= 500; j++)
        {
            srand(time(0));
            sprintf(junk_buf, "%f", pow(rand(), 10000));
            // 시간 소모를 위함
        }
        str_buf[buf_idx++] = '.';
        // 일종의 sprintf 역할
    }
}

void *runner_FIFO(void *param)
{
    int i, policy;
    pthread_attr_t attr;

    while (!flag)
        sleep(0);

    set_my_sched_policy(SCHED_FIFO);

    for (int i = 0; i <= 50; i++)
    {
        for (int j = 0; j <= 100; j++)
        {
            srand(time(0));
            sprintf(junk_buf, "%f", pow(rand(), 10000));
        }
        usleep(55000);
        str_buf[buf_idx++] = '!';
        str_buf[buf_idx++] = '\n';
    }
}

void *runner_RR(void *param)
{
    int i, policy;
    pthread_attr_t attr;

    while (!flag)
        sleep(0);

    set_my_sched_policy(SCHED_RR);

    for (int i = 0; i <= 50; i++)
    {
        for (int j = 0; j <= 100; j++)
        {
            srand(time(0));
            sprintf(junk_buf, "%f", pow(rand(), 10000));
        }
        usleep(45000);
        str_buf[buf_idx++] = '@';
        str_buf[buf_idx++] = '\n';
    }
}

/* 실행 결과 예상

- SCHED_OTHER / SCHED_RR / SCHED_FIFO 중 1 출력
- 버퍼에 . / ! / @ 출력
  -> 얼만큼 규칙적으로 찍히는지 볼 것

=> 왜 나선형?
- usleep 함수의 45000, 55000의 10ms 정도의 차이 때문


리눅스 -> 쓰레드에서 원하는 스케줄러로 돌게 하고 싶을 때,
          이런식으로 프로그래밍하면 됨

*/