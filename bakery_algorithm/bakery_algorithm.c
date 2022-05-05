#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define THREAD_COUNT 32 // 방문자 : 업무 보러 오는 사람들

/*
    volatile
    해당 변수에 대한 operation은 optimize 하지 말아라.
    컴파일러 -> 최적화에서 제외, 항상 메모리에 접근하도록
*/

// 각 방문자가 번호기에서 받는 번호 저장
volatile int numbers[THREAD_COUNT] = {0};
// 서로 비교할 때, 상대방이 번호표 뽑고 있을 때, 기다릴 때
volatile int choosing[THREAD_COUNT] = {0};

// 임계 구역 만들기 위함 - 간섭 일어나는지 확인
volatile int counter = 0;

void lock(int);
void unlock(int);
void use_resource(int);
void *thread_body(void *);

int main(int argc, char **argv)
{
    pthread_t threads[THREAD_COUNT];

    srand(time(NULL));

    printf("\nnumber    thread_id   counter\n");
    fflush(stdout);
    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_create(&threads[i], NULL, &thread_body, (void *)((long)i));
    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_join(threads[i], NULL);
    printf("\n");

    return 0;
}

// arg : main for문 안의 i - (void *)((long)i
void *thread_body(void *arg) // 동시에 32개의 thread_body가 실행
{
    long t_id = (long)arg;

    usleep(rand() % 0xFFFFF);

    // random 한 숫자로 대기하기 때문에
    // thread 생성된 시점과 lock 걸린 시점에 차이가 있음
    lock(t_id);

    // Critical Section
    use_resource(t_id);

    unlock(t_id);

    return NULL;
}
void use_resource(int t_id)
{
    // 32개의 스레드들이 함께 실행되기 때문에
    // counter 값들이 제대로 1씩 증가되는지 확인하기 위함
    counter++;
    usleep(rand() % 0xFFFF);
    counter++;
    usleep(rand() % 0xFFFF);
    counter--;
    usleep(rand() % 0xFFFF);
    counter--;
    usleep(rand() % 0xFFFF);
    counter++;
    usleep(rand() % 0xFFFF);

    printf("    %d\t\t%d\t   %d\n", numbers[t_id], t_id, counter);

    // printf -> 버퍼에 들어가 바로 출력 안 될 수 있으므로, flush
    fflush(stdout);
}

/* bakery algorithm 구현 */
void lock(int t_id)
{
    choosing[t_id] = 1; // 1: 번호표 받기

    int max = 0;
    for (int i = 0; i < THREAD_COUNT; i++)
        max = numbers[i] > max ? numbers[i] : max;
    numbers[t_id] = max + 1;

    // 현재 번호표 최댓값보다 +1 한 값이 현재 lock 하러 들어온 프로세스가 받는 번호 == 번호표에 찍힌 번호

    choosing[t_id] = 0; // 0: 번호표 받기 완료

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        while (choosing[i])
            ; // 번호표 받는 중이면 기다림 (choosing[i]=1 => while 조건문 true)

        while (numbers[i] != 0 && (numbers[i] < numbers[t_id] ||
                                   (numbers[i] == numbers[t_id] && i < t_id)))
        {
            /*
                1. numbers[i]!=0
                    unlock 함수 참고 -> numbers[t_id]=0
                    빠져 나가는 함수 기다릴 필요 X

                2. numbers[i] < numbers[t_id] || (numbers[i] == numbers [t_id] && i < t_id)
                    번호 비교
            */
        }
    }
}

void unlock(int t_id)
{
    numbers[t_id] = 0;
}