#include <iostream>
#include <atomic>
#include <thread>
#include <sstream>
#include <vector>
#include <unistd.h>

#define NUM_THREAD 16

using namespace std;

stringstream stream;
atomic<int> lock;

void do_something(int x)
{
    int old = 0, one = 1;
    bool ex = false;

    usleep(rand() % 0xFFFF);

    while ((!(ex = lock.compare_exchange_strong(old, one)) || old))
        ; // c++ -> compare-and-swap => compare_exchange
    // old vs new - 동일 -> lock 값 one으로 덮어씀
    //            - 다름 -> 현재 lock 값 return

    // stream << "lock " << lock << " old " << old << " one " << one << " ex " << ex << '\n;

    stream << "abcdefghijklmnopqrstuvwxyz #" << x << '\n';
    usleep(rand() % 0xFFFF);
    stream << "abcdefghijklmnopqrstuvwxyz #" << x << '\n';

    lock = 0;
}

int main() // test-and-set과 main문은 동일
{
    vector<thread> threads;

    lock = 0;

    for (int i = 1; i <= NUM_THREAD; i++)
        threads.push_back(thread(do_something, i));

    for (auto &th : threads)
        th.join();

    cout << stream.str();

    return 0;
}