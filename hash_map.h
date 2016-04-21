//
// Created by dns on 16.04.2016.
//

#ifndef HASH_MAP_HASH_MAP_H
#define HASH_MAP_HASH_MAP_H
#include "Entry.h"
#include <vector>
#include <assert.h>
#include <set>
#include <iostream>
#define STANDARD_CAPACITY 10000

struct map {

    struct iterator
    {
        iterator& operator++();
        iterator operator++(int);
        iterator& operator--();
        iterator operator--(int);
        iterator& operator=(iterator const&);
        void swap(iterator &);
        friend bool operator==(iterator const&, iterator const&);
        friend bool operator!=(iterator const&, iterator const&);

        value_type& operator*();

        friend struct map;
        iterator();
        ~iterator();
        iterator(iterator const&);
        iterator(iterator const&, std::string);
        map *m;

    private:
        iterator(std::shared_ptr<Entry> const&, map* const&, std::string);
        std::shared_ptr<Entry> v;
        std::string info;
    };

    map();
    map(int capacity);
    ~map();
    map(map const&);
    map& operator=(map const&);

    bool empty() const;
    size_t size() const;
    iterator insert(value_type const&);
    size_t erase(key_type const&);
    iterator erase(iterator const&);
    iterator find(key_type const&);

    void swap(map&);
    void clear();

    iterator begin();
    iterator end();

private:
    std::pair<size_t, std::shared_ptr<Entry>> eraser(key_type const&);
    friend void swapper(map&, map&);
    int first_bucket = -1, last_bucket = -1;
    std::vector<std::shared_ptr<Entry>> table;
    std::vector<iterator*> iterators; //Save ptrs to iterators, so not to invaldate them after swap
    std::shared_ptr<Entry> fake_end;
    size_t ssize;
};
#endif //HASH_MAP_HASH_MAP_H
