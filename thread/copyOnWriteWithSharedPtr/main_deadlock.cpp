#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <set>

/* this code has two issues:
 *
 * 1. Inventory::printAll will lock Inventory::_mutex first, then lock Request::_mutex,
 *    but ~Request will lock Request::_mutex first, then lock Inventory::_mutex, so if
 *    printAll and ~Request are called in the same time, it will lead to deadlock.
 *
 * 2. ~Request has data race issue cuz Inventory has its raw pointer.
 *
 */

class Request;
class Inventory
{
    public:
        void add (Request* req) __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            _requests.insert(req);
        }

        void remove (Request* req) __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            _requests.erase(req);
        }

        void printAll () const;

    private:
        mutable std::mutex _mutex;
        std::set<Request*> _requests;
};

Inventory g_inventory;

class Request
{
    public:
        void process () __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            g_inventory.add(this);
        }

        ~Request () __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            sleep(1);
            g_inventory.remove(this);
        }

        void print () __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            printf("print req\n");
        }

    private:
        mutable std::mutex _mutex;
};

void
Inventory::printAll () const
{
    std::lock_guard<std::mutex> lk(_mutex);
    sleep(1);
    for (auto it = _requests.begin(); it != _requests.end();
            it++)
    {
        (*it)->print();
    }
}

int main ()
{
    std::thread t([]{
                        Request* req = ::new Request;
                        req->process();
                        ::delete req;
                    });

    usleep(500*1000);
    g_inventory.printAll();
    t.join();

    return 0;
}
