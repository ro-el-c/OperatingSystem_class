#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <atomic> // atomic operation 활용시 atomic 헤더 파일 include

#define NUM_THREAD 16

using namespace std;

atomic_flag lock = ATOMIC_FLAG_INIT;
stringstream stream; // 출력 내용 받아뒀다가 한 번에 출력하기 위함 (like 자바의 StringBuffer)

void do_something(int x)
{
    usleep(rand() % 0xFFFF);

    while (lock.test_and_set())
        ; // c++ -> lock 이 객체로 되어있음

    // lock 이 0인 딱 하나의 스레드만 임계 구역에 들어옴

    // x : 스레드 별 일련 번호
    stream << "abcdefghijklmnopqrstuvwxyz #" << x << '\n';
    usleep(rand() % 0xFFFF);
    // test-and-set 동작 제대로 되지 않아, 임계 구역이 깨지면 두 출력에 차이가 있을 것.
    stream << "abcdefghijklmnopqrstuvwxyz #" << x << '\n';

    lock.clear();
}

int main()
{
    vector<thread> threads;

    srand(time(NULL));

    for (int i = 1; i < NUM_THREAD; i++)
        threads.push_back(thread(do_something, i)); // c++ 스레드 호출

    for (auto &th : threads)
        th.join();

    cout << stream.str();

    return 0;
}