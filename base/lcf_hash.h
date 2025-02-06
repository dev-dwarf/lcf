#ifndef LCF_HASH
#define LCF_HASH

// TODO(lcf) consider adding deletion functions, if useful
struct Table {
    u32 exp;
    s32 keys;
    void* first;
};
typedef struct Table Table; 

// WARN(lcf): Caller must guarantee that there is always atleast 1 free entry, ie insertions should stop before 
// keys == (1 << exp)-1 otherwise Table_i will get stuck. This is done by Table_insert
// In practice, for good performance (which is why you would use a hash table), insertion
// should start long before, roughly when keys == (1 << (exp-1)).
static void* Table_lookup(Table *t, u64 hash) {
    // REF(lcf) https://nullprogram.com/blog/2022/08/08/
    u32 mask = ((u32)1 << t->exp) - 1;
    u32 step = (u32) (hash >> (64 - t->exp)) | 1;
    for (s32 i = (s32) hash;;) {
        i = (i + step) & mask;
        u64 *entry = (u64 *)(&t->first) + 2*i;
        if (!entry[1] || entry[1] == hash) {
            return (void*) entry[0];
        }
    }
}

static void* Table_insert(Table *t, u64 hash, void* data) {
    if (t->keys == (1 << (t->exp)) - 1) {
        return 0;
    }

    u32 mask = ((u32)1 << t->exp) - 1;
    u32 step = (u32) (hash >> (64 - t->exp)) | 1;
    for (s32 i = (s32) hash;;) {
        i = (i + step) & mask;
        u64 *entry = (u64 *)(&t->first) + 2*i;
        if (!entry[1]) {
            t->keys++;
        }
        if (!entry[1] || entry[1] == hash) {
            ((void**) entry)[0] = data;
            entry[1] = hash;
            return entry;
        }
    }
}

static inline u16 round_up_exp_pow2(u32 x) {
    // Round up to power of 2, opted for a fairly simple binary search alg.
    // REF: Hacker's Delight, pg 100
    x = x-1;
    u16 n = 32;
    u32 y; 
    y = x >> 16; if (y > 0) {n = n - 16; x = y;}
    y = x >> 8; if (y > 0) {n = n - 8; x = y;}
    y = x >> 4; if (y > 0) {n = n - 4; x = y;}
    y = x >> 2; if (y > 0) {n = n - 2; x = y;}
    y = x >> 1;
    n = (y > 0)? n - 2 : n - (u16) x;
    // n is amount of leading zeros in x.
    return 32 - n;
}

static Table* Table_create(Arena *a, u32 capacity) {
    u16 exp = round_up_exp_pow2(capacity - 1);
    Table* out = (Table*) Arena_take_zero(a, sizeof(Table) + (1 << exp)*2*sizeof(u64));
    out->exp = exp;
    return out;
}

// fnv1a_32
static inline u64 hash_str(str s, u64 hash) {
    hash = (hash)? hash : 0x811C9DC5;
    str_iter(s, i, c) {
        hash = (((u64) c) ^ hash) * 0x01000193;
    }
    return hash;
}
#endif
