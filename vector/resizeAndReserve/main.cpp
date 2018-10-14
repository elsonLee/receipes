#include <cstdio>
#include <vector>

int main (int argc, char** argv)
{
    {
        std::vector<int> v;
        //! size = 100, capacity = 100
        v.resize(100);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());

        // new_size < cur_size, reduce size
        //! size = 99, capacity = 100
        v.resize(99);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());

        // new_size > cur_size, append additional elements
        //! size = 101, capacity = 198
        v.resize(101);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());
    }

    {
        std::vector<int> v;
        printf("====================\n");
        for (int i = 1; i < 50; i++) {
            v.resize(i);
            printf("size: %ld, capacity: %ld\n",
                    v.size(), v.capacity());
        }
        printf("====================\n");
    }

    //! new_size > cap && new_size <= 2*cap, new_cap = 2*cap
    //  new_size > 2*cap, new_cap = new_size
    {
        std::vector<int> v;
        printf("====================\n");
        v.resize(1);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());
        v.resize(49);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());
        v.resize(55);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());
        printf("====================\n");
    }

    {
        std::vector<int> v;
        //! size = 1, capacity = 100
        v.reserve(100);
        v.push_back(1);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());

        //! new_cap < cur_cap, do nothing
        v.reserve(99);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());

        //! new_cap > cur_cap, allocate new mem, capacity = 101
        //  all references are invalid
        v.reserve(101);
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());

        //! capacity = size = 1
        v.shrink_to_fit();
        printf("size: %ld, capacity: %ld\n",
                v.size(), v.capacity());
    }



    return 0;
}
