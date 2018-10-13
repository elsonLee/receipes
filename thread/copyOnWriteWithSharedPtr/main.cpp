#include <stdio.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <thread>
#include <mutex>
#include <set>
#include <vector>

/* Methods to resolve the issues:
 *
 * 1. move Request::print() out of the critical section in printAll() by
 *    using copy-on-write with shared_ptr, 
 *    Request::print is a read operation, and all read operations are allowed
 *    simultaneously if and only if no write operation happens. so we can do
 *    write operation on a copy of the instance, Request::print can be called
 *    without mutex locked.
 *
 * 2. use shared_ptr to delay the lifetime of Request, resolve data race in
 *    ~Request()
 *    weak_ptr can be put into std::vector, but cannot be put into std::set,
 *    the internal pointer can be modified to nullptr. so we use shared_ptr instead
 *
 */

class Request;
template <typename T>
using List = std::set<T>;
template <typename T>
using ListPtr = std::shared_ptr<List<T>>;

class Inventory
{
    public:

        Inventory () :
            _requests(::new List<std::shared_ptr<Request>>())
        {}

        void add (std::shared_ptr<Request> req) __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            if (_requests.use_count() != 1) {
                // _requests is read by other threads, so copy it and write on the duplication
                _requests.reset(::new List<std::shared_ptr<Request>>(*_requests));
            }
            assert(_requests.use_count() == 1);
            _requests->insert(req);
        }

        void remove (std::shared_ptr<Request> req) __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            if (_requests.use_count() != 1) {
                // _requests is read by other threads, so copy it and write on the duplication
                _requests.reset(::new List<std::shared_ptr<Request>>(*_requests));
            }
            assert(_requests.use_count() == 1);
            _requests->erase(req);
        }

        void printAll () const;

    private:
        mutable std::mutex _mutex;
        ListPtr<std::shared_ptr<Request>> _requests;
};

Inventory g_inventory;

class Request : public std::enable_shared_from_this<Request>
{
    public:
        void process () __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            g_inventory.add(shared_from_this());
        }

        ~Request () __attribute__((noinline))
        {
            // the logic in ::cancel() cannot put here,
            // don't put shared_from_this in destructor
        }

        void cancel () __attribute__((noinline))
        {
            std::lock_guard<std::mutex> lk(_mutex);
            sleep(1);
            g_inventory.remove(shared_from_this());
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
    std::shared_ptr<std::set<std::shared_ptr<Request>>>  tmp;
    {
        std::lock_guard<std::mutex> lk(_mutex);
        sleep(1);
        tmp = _requests;
    }
    for (auto it = tmp->begin(); it != tmp->end(); it++)
    {
        (*it)->print();
    }
}

int main ()
{
    std::thread t([]{
                        std::shared_ptr<Request> req = std::make_shared<Request>();
                        req->process();
                        req->cancel();
                    });

    usleep(500*1000);
    g_inventory.printAll();
    t.join();

    return 0;
}
