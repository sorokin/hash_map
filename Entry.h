//
// Created by dns on 16.04.2016.
//

#ifndef HASH_MAP_ENTRY_H
#define HASH_MAP_ENTRY_H
#include <string>
#include <memory>

typedef std::string key_type;
typedef int mapped_type;
typedef std::pair<key_type, mapped_type> value_type;

struct Entry {
    Entry() : hash(0x811c9dc5), next(nullptr), prev(nullptr), valid(false), isFake(false) {};
    Entry(value_type const& value) : hash(hash_f(value.first)), value(value), next(nullptr), prev(nullptr), isFake(false) {};
    unsigned int hash;
    int next_bucket = -1, prev_bucket = -1;
    std::shared_ptr<Entry> next, prev;
    value_type value;
    bool valid, isFake;

    static const unsigned FNV_32_PRIME = 0x01000193;

    static unsigned int hash_f(std::string const &obj) {
        unsigned int hval = 0x811c9dc5;
        for (size_t i = 0; i < obj.size(); i++) {
            hval ^= (unsigned int)obj[i];
            hval *= FNV_32_PRIME;
        }
        return hval;
    }
};
#endif //HASH_MAP_ENTRY_H
