//
// Created by dns on 16.04.2016.
//

#include "hash_map.h"

#include <algorithm>

map::iterator::iterator() : v(), m(), info("Default") {};

map::iterator::iterator(map::iterator const& other) : v(other.v), m(other.m) {
    info = "[Copied]" + other.info;
    m->iterators.push_back(this);
}

map::iterator::iterator(map::iterator const& other, std::string info) : v(other.v), m(other.m), info("[Copied with info]" + info) {
    m->iterators.push_back(this);
};

map::iterator::iterator(std::shared_ptr<Entry> const& en, map* const& m, std::string info) : v(en), m(m), info("[Created]" + info) {
    m->iterators.push_back(this);
};

map::iterator::~iterator() {
    m->iterators.erase(std::find(m->iterators.begin(), m->iterators.end(), this));
}

map::iterator& map::iterator::operator=(map::iterator const &other) {
    if (this != &other) {
        iterator tmp = other;
        swap(tmp);
        tmp.m->iterators.erase(std::find(tmp.m->iterators.begin(), tmp.m->iterators.end(), this));
        tmp.m = other.m;
        other.m->iterators.push_back(this);
    }
    return *this;
}

void map::iterator::swap(map::iterator &other)
{
    using std::swap;
    swap(this->m, other.m);
    swap(this->v, other.v);
    swap(this->info, other.info);
}


value_type& map::iterator::operator*() {
    assert(v && v->valid && !v->isFake);
    return v->value;
}
map::iterator& map::iterator::operator++() {
    assert(v && v->valid && !v->isFake);
    if (v->next)
        v = v->next;
    else {
        unsigned int h = v->hash;
        h %= m->table.size();
        if (h == m->last_bucket) {
            v = std::make_shared<Entry>();
            v->valid = true;
            v->isFake = true;
        } else {
            v = m->table[m->table[h]->next_bucket];
        }
    }
    return *this;
}

map::iterator map::iterator::operator++(int) {
    iterator tmp = *this;
    ++(*this);
    return tmp;
}

map::iterator& map::iterator::operator--() {
    assert(v && v->valid && v != m->table[m->first_bucket]);
    if (v->isFake) {
        auto now = m->table[m->last_bucket];
        while(now->next)
            now = now->next;
        v = now;
    } else {
        if (v->prev)
            v = v->prev;
        else {
            auto now = m->table[v->prev_bucket];
            while(now->next)
                now = now->next;
            v = now;
        }
    }
    return *this;
}

map::iterator map::iterator::operator--(int) {
    iterator tmp = *this;
    --(*this);
    return tmp;
}

bool operator==(map::iterator const &a, map::iterator const &b) {
    assert(a.v && b.v && a.v->valid && b.v->valid);
    if (a.m != b.m) {
        return false;
    }
    if (a.v->isFake && b.v->isFake) {
        return true;
    }
    return a.v->value == b.v->value;
}

bool operator!=(map::iterator const& a, map::iterator const &b) {
    return !(a == b);
}

map::map() {
    ssize = 0;
    table.resize(STANDARD_CAPACITY, nullptr);
    fake_end = std::make_shared<Entry>();
    fake_end->valid = true;
    fake_end->isFake = true;
}

map::map(int capacity) {
    ssize = 0;
    table.resize(capacity, nullptr);
    fake_end = std::make_shared<Entry>();
    fake_end->valid = true;
    fake_end->isFake = true;
}

map::map(map const &other) {
    if (this != &other) {
        /*table.resize(other.table, nullptr);
        for (int i = 0; i < table.size(); i++) {
            if (other.table[i]) {
                table[i] = std::make_shared<Entry>(other.table[i]);
                while(other.table[i]->next)
            }
        }*/
        first_bucket = other.first_bucket;
        last_bucket = other.last_bucket;
        ssize = other.size();
        fake_end = std::make_shared<Entry>();
        fake_end->valid = true;
        fake_end->isFake = true;
    }
}

map::~map() {
    clear();
}

map& map::operator=(map const &other) {
    if (this->fake_end != other.fake_end) {
        map tmp = map(other);
        swap(tmp);
    }
    return *this;
}

void map::swap(map &other) {
    swapper(*this, other);
}

bool map::empty() const {
    return ssize == 0;
}

size_t map::size() const {
    return ssize;
}

void map::clear() {
    for (auto it = begin(); it != end();)
        erase(it++);
    table.resize(table.size(), nullptr);
    ssize = 0;
}

void swapper(map& a, map& b) {
    using std::swap;
    swap(a.table, b.table);
    swap(a.first_bucket, b.first_bucket);
    swap(a.last_bucket, b.last_bucket);
    swap(a.ssize, b.ssize);
    swap(a.fake_end, b.fake_end);
    swap(a.iterators, b.iterators);
    for (auto& e : a.iterators)
        e->m = &a;
    for (auto& e : b.iterators)
        e->m = &b;
}

map::iterator map::insert(value_type const& value) {
    unsigned int h = Entry::hash_f(value.first);
    h %= table.size();
    auto now = table[h];
    if (!now) { //First element creating bucket
        table[h] = std::make_shared<Entry>(value);
        table[h]->valid = true;
        if (first_bucket == -1)
            first_bucket = last_bucket = h;
        else {
            table[last_bucket]->next_bucket = h;
            table[h]->prev_bucket = last_bucket;
            last_bucket = h;
        }
        ssize++;
        return iterator(table[h], this, "Insert in new bucket");
    } else { //Insert in bucket
        if (now->value.first == value.first) {
            return iterator(now, this, "Element already is inserted and is first in bucket");
        } else {
            while (now->next != nullptr) {
                now = now->next;
                if (now->value.first == value.first)
                    return iterator(now, this, "Element already is inserted, and isn't first");
            }
            now->next = std::make_shared<Entry>(Entry(value));
            now->next->valid = true;
            now->next->prev = now;
            ssize++;
            return iterator(now->next, this, "Inserted in existing bucket");
        }
    }
}

std::pair<size_t, std::shared_ptr<Entry>> map::eraser(key_type const& key) {
    unsigned int h = Entry::hash_f(key);
    h %= table.size();
    auto now = table[h];
    if (!now) //No bucket with such hash
        return std::make_pair(0, fake_end);
    if (now->value.first == key) { //First element in bucket to remove
        now->valid = false;
        if (!now->next) { //The only element in bucket
            if (now->next_bucket == -1 && now->prev_bucket == -1) { //Last element in map
                first_bucket = last_bucket = -1;
                ssize--;
                return std::make_pair(1, fake_end);
            }
            if (now->prev_bucket != -1) { //There is previous bucket
                if (h == last_bucket)
                    last_bucket = now->prev_bucket;
                 table[now->prev_bucket]->next_bucket = table[h]->next_bucket;
            }
            if (now->next_bucket != -1)  { //There is next bucket
                if (h == first_bucket)
                    first_bucket = now->next_bucket;
                table[now->next_bucket]->prev_bucket = table[h]->prev_bucket;
            }
        }
        else { //There is another element in bucket
            now->next->next_bucket = now->next_bucket;
            now->next->prev_bucket = now->prev_bucket;
        }
        table[h] = now->next;
        if (table[h])
            table[h]->prev = nullptr;
        now->next = nullptr;
        ssize--;
        if (table[h])
            return std::make_pair(1, table[h]);
        else if (now->next_bucket != -1)
            return std::make_pair(1, table[now->next_bucket]);
        else
            return std::make_pair(1, fake_end);
    } else { //Element somewhere in the middle of bucket
        while (now->next != nullptr && now->next->value.first != key) {
            now = now->next;
        }
        auto to_er = now->next;
        if (!to_er) //No element found in bucket with that key
            return std::make_pair(0, fake_end);
        to_er->valid = false;
        now->next = to_er->next;
        if (now->next)
            now->next->prev = now;
        to_er->next = nullptr;
        ssize--;
        if (now->next)
            return std::make_pair(1, now->next);
        else if (table[h]->next_bucket != -1)
            return std::make_pair(1, table[table[h]->next_bucket]);
        else
            return std::make_pair(1, fake_end);
    }
}

map::iterator map::find(key_type const& key) {
    unsigned int h = Entry::hash_f(key);
    h %= table.size();
    auto now = table[h];
    while(now != nullptr && now->value.first != key) {
        now = now->next;
    }
    if (!now)
        return iterator(fake_end, this, "No element found");
    else
        return iterator(now, this, "Found");
}

map::iterator map::erase(map::iterator const& it) {
    return iterator(eraser(it.v->value.first).second, this, "Erased");
}

size_t map::erase(key_type const& key) {
    return eraser(key).first;
}

map::iterator map::begin() {
    if (first_bucket == -1)
        return iterator(fake_end, this, "Begin with empty map");
    return iterator(table[first_bucket], this, "Begin");
}

map::iterator map::end() {
    return iterator(fake_end, this, "End");
}
