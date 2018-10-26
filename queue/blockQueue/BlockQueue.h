#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockQueue
{
    public:
        BlockQueue () = default;
        ~BlockQueue () = default;

        T take ();

        void put (const T& t);

        size_t size () const {
            std::lock_guard<std::mutex> lk(_mtx);
            return _queue.size();
        }

        bool empty () const {
            std::lock_guard<std::mutex> lk(_mtx);
            return _queue.empty();
        }

    private:
        BlockQueue (const BlockQueue&) = delete;
        BlockQueue& operator= (const BlockQueue&) = delete;

    private:
        std::mutex  _mtx;
        std::condition_variable _cond;
        std::queue<T>  _queue;
};

template <typename T>
T
BlockQueue<T>::take ()
{
    std::unique_lock<std::mutex> lk(_mtx);
    _cond.wait(lk, [this]{ return !_queue.empty(); });
    T tmp(_queue.front());
    _queue.pop();
    return tmp;
}

template <typename T>
void
BlockQueue<T>::put (const T& t)
{
    std::lock_guard<std::mutex> lk(_mtx);
    _queue.push(t);
    _cond.notify_one();
}
