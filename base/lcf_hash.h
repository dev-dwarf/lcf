#ifndef LCF_HASH
#define LCF_HASH

// TODO(lcf) consider adding deletion functions, if useful

// WARN(lcf): Caller must guarantee that there is always atleast 1 free entry, ie insertions should stop before 
// keys == (1 << exp)-1 otherwise Table_i will get stuck.
// In practice, for good performance (which is why you would use a hash table), insertion
// should start long before, roughly when keys == (1 << (exp-1)).

struct Table {
    u32 exp;
    s32 keys;
    u64 first;
};
typedef struct Table Table; 

u64 Table_lookup(Table *t, u64 hash) {
    // REF(lcf) https://nullprogram.com/blog/2022/08/08/
    u32 mask = ((u32)1 << t->exp) - 1;
    u32 step = (hash >> (64 - t->exp)) | 1;
    for (s32 i = hash;;) {
        i = (i + step) & mask;
        u64 *entry = (&t->first) + 2*i;
        if (!entry[1] || entry[1] == hash) {
            return entry[0];
        }
    }
}

u64* Table_insert(Table *t, u64 hash, u64 data) {
    if (t->keys == (1 << (t->exp)) - 1) {
        return 0;
    }

    u32 mask = ((u32)1 << t->exp) - 1;
    u32 step = (hash >> (64 - t->exp)) | 1;
    for (s32 i = hash;;) {
        i = (i + step) & mask;
        u64 *entry = (&t->first) + 2*i;
        if (!entry[1] || entry[1] == hash) {
            entry[0] = data;
            entry[1] = hash;
            t->keys++;
            return entry;
        }
    }
}

inline u16 _round_up_exp_pow2(u32 x) {
    // Round up to power of 2, opted for a fairly simple binary search alg.
    // REF: Hacker's Delight, pg 100
    s32 n = 32;
    u32 y; 
    y = x >> 16; if (y > 0) {n = n - 16; x = y;}
    y = x >> 8; if (y > 0) {n = n - 8; x = y;}
    y = x >> 4; if (y > 0) {n = n - 4; x = y;}
    y = x >> 2; if (y > 0) {n = n - 2; x = y;}
    y = x >> 1;
    n = (y > 0)? n - 2 : n - x;
    // n is amount of leading zeros in x.
    return 32 - n;
}

Table* Table_create(Arena *a, u32 capacity) {
    u16 exp = _round_up_exp_pow2(capacity - 1);
    Table* out = Arena_take_zero(a, sizeof(Table) + (1 << exp)*2*sizeof(u64));
    *out = (Table) {
        .exp = exp,
    };
    return out;
}

// djb2
inline u64 hash_str(str s, u64 *h) {
    u64 hash = h? *h : 5381;
    str_iter(s, i, c) {
        hash = ((hash << 5) + hash) + (unsigned)c; /* hash * 33 + c */
    }
    return hash;
}

#endif
