#include <iostream>
#include <thread>
#include <cassert>

#include "BlockQueue.h"

constexpr long answer ()
{
    return (1 + 50000) * (50000 / 2);
}

int main ()
{
    BlockQueue<int> q;
    long sum = 0;

    std::thread producer(
            [&q]{
                for (int i = 0; i <= 50000; i++) {
                    q.put(i);
                }
                q.put(-1);
            });

    std::thread consumer(
            [&q, &sum]{
                int val = 0;
                while ((val = q.take()) != -1) {
                    sum += val;
                }
            });

    producer.join();
    consumer.join();

    std::cout << "sum: " << sum << std::endl;
    assert(sum == answer());

    return 0;
}
