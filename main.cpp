#include "hash_map.h"
#include <string>
#include <unordered_map>

int main()
{
    /*map a, b;
    for (size_t i = 0; i != 10; ++i)
        a.insert(std::make_pair(std::to_string(i), i));
    for (size_t i = 0; i != 10; ++i)
        b.insert(std::make_pair(std::to_string(i), i + 1));
    map::iterator a_it = a.begin();
    a.swap(b);
    for (; a_it != b.end(); a_it++) {
        std::cout << (*a_it).second << std::endl;
    }*/

    /*map::iterator x;
    {
        map a;
        x = a.begin();
    }*/

    /*map b;
    b.insert(std::make_pair("1", 1));
    map a;
    map::iterator* i = new map::iterator(a.begin());
    *i = b.begin();
    delete i;
    a.swap(b);*/

    map a;
    for (size_t i = 0; i != 100000; ++i)
        a.insert(std::make_pair(std::to_string(i), i));

    map b = a;

    for (size_t i = 0; i != 100000; ++i)
        a.erase(std::to_string(i));

    for (auto const& e : b)
    {
        std::cout << e.first << " " << e.second << "\n";
    }
}
