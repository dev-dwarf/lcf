#include "lcf_memory.h"
#define B_PTR(p) (u8*)(p)

internal b32 is_power_of_2(upr x) {
    return ((x & (x-1)) == 0);
}

internal u64 next_alignment(upr ptr, upr alignment) {
    ASSERTM(is_power_of_2(alignment), "Alignments must be a power of 2.");

    /* Fast replacement for mod because alignment is power of 2 */
    u64 modulo = ptr & (alignment-1);

    if (modulo != 0) {
        ptr += alignment - modulo;
    }

    return ptr;
}

/** Arena ********************************/
Arena* Arena_create(u64 size) {
    Arena* a = (Arena*) LCF_MEMORY_reserve(size);
    LCF_MEMORY_commit(a, LCF_MEMORY_COMMIT_SIZE);
    a->size = size;
    a->pos = sizeof(Arena);
    a->commited_pos = LCF_MEMORY_COMMIT_SIZE;
    a->alignment = LCF_MEMORY_ALIGNMENT;
    return a;
}

Arena* Arena_create_default(void) {
    return Arena_create(LCF_MEMORY_RESERVE_SIZE);
}

void Arena_destroy(Arena *a) {
    LCF_MEMORY_free(a, a->size);
}

void* Arena_take_custom(Arena *a, u64 size, u32 alignment) {
    void* result = 0;
    
    /* Align pos pointer to check if "size" can fit */
    upr mem = (upr) a;
    upr aligned_pos = next_alignment(mem + a->pos, alignment) - mem;
    upr new_pos = aligned_pos + size;
        
    /* Check that there is space */
    if (new_pos < a->size) {
        upr commited_pos = a->commited_pos;

        /* Commit memory if needed */
        if (new_pos > commited_pos) {
            upr new_commited_pos = next_alignment(mem + new_pos, LCF_MEMORY_COMMIT_SIZE)-mem;
            if (LCF_MEMORY_commit(a, new_commited_pos)) {
                a->commited_pos = commited_pos = new_commited_pos;
            }
        }

        /* If enough memory is commited, set result and pos. */
        if (new_pos <= commited_pos) {
            result = (void*)(mem + aligned_pos);
            a->pos = new_pos;
        }
    }
    ASSERTM(result != NULL, "Arena out of memory!");
    return result;
}

void* Arena_take(Arena *a, u64 size) {
    return Arena_take_custom(a, size, LCF_MEMORY_ALIGNMENT);
}

void* Arena_take_zero_custom(Arena *a, u64 size, u32 alignment) {
    void* mem = Arena_take_custom(a, size, alignment);
    MemoryZero(mem, size);
    return mem;
}

void* Arena_take_zero(Arena *a, u64 size) {
    void* mem = Arena_take(a, size);
    MemoryZero(mem, size);
    return mem;
}

void Arena_reset(Arena *a, u64 pos) {
    if (pos < a->pos) {
        pos = MAX(pos, sizeof(Arena));
        if (LCF_MEMORY_DEBUG_CLEAR) {
            /* Clear memory between pos and a->pos */
            MemorySet((void*) ((u64)a+pos), LCF_MEMORY_ARENA_CLEAR, a->pos - pos);
        }
        a->pos = pos;
    }
}

void Arena_reset_all(Arena *a) {
    Arena_reset(a, sizeof(Arena));
}

void Arena_reset_decommit(Arena *a, u64 pos) {
    Arena_reset(a, pos);

    /* Decommit everything except what is needed, or one page */
    if (a->commited_pos > 1) {
        upr mem = (upr) a;
        upr needed_pos = (next_alignment(mem + a->pos, LCF_MEMORY_COMMIT_SIZE) - mem) ;
        upr over_commited = a->commited_pos - needed_pos;
        LCF_MEMORY_decommit(a+needed_pos, over_commited);
        a->commited_pos = needed_pos / LCF_MEMORY_COMMIT_SIZE;
    }
}

void Arena_reset_all_decommit(Arena *a) {
    Arena_reset(a, 0);

    /* Decommit everything except one page */
    LCF_MEMORY_decommit(a+LCF_MEMORY_COMMIT_SIZE, LCF_MEMORY_COMMIT_SIZE*MAX(a->commited_pos-1, 0));
    a->commited_pos = 0;
}

void* Arena_resize_custom(Arena *a, void* old_memory, u64 old_size, u64 new_size, u64 alignment) {
    void* result = 0;

    if (old_memory == 0 || old_size == 0) {
        return Arena_take_custom(a, new_size, alignment);
    } else if ((a <= old_memory) && (B_PTR(old_memory) < B_PTR(a)+a->size)) {
        /* Check that old_memory was most recently taken block */
        if (B_PTR(old_memory) == (B_PTR(a) + (a->pos - old_size))) {
            result = old_memory;
            a->pos = a->pos - old_size + new_size;
            if (old_size < new_size) {
                MemorySet(B_PTR(old_memory)+old_size, 0, new_size-old_size);
            }
        } else {
            result = Arena_take_custom(a, new_size, alignment);
            MemoryCopy(result, old_memory, old_size);
        }
    } else {
        BADPATH("old_memory is not within the bounds of the Arena's buffer.");
    }

    return result;
}

void* Arena_resize(Arena *a, void* old_memory, u64 old_size, u64 new_size) {
    return Arena_resize_custom(a, old_memory, old_size, new_size,
                               LCF_MEMORY_ALIGNMENT);
}

ArenaSession ArenaSession_begin(Arena *a) {
    ArenaSession s;
    s.arena = a;
    s.session_start = a->pos;
    return s;
}

void ArenaSession_end(ArenaSession s) {
    Arena_reset(s.arena, s.session_start);
}


#ifdef __cplusplus
Arena* Arena::create_default(void) {
    return Arena_create_default();
}
Arena* Arena::create(u64 size) {
    return Arena_create(size);
}
void Arena::destroy() {
    Arena_destroy(this);
}
void* Arena::take(u64 sz) {
    return Arena_take(this, sz);
}
void* Arena::take(u64 sz, u32 align) {
    return Arena_take_custom(this, sz, align);
}
void* Arena::take_zero(u64 sz) {
    return Arena_take_zero(this, sz);
}
void* Arena::take_zero(u64 sz, u32 align) {
    return Arena_take_zero_custom(this, sz, align);
}
template<typename T> void* Arena::take_struct() {
    return Arena_take(this, sizeof(T));
}
template<typename T> void* Arena::take_struct_zero() {
    return Arena_take_zero(this, sizeof(T));
}
template<typename T> void* Arena::take_array(u64 count) {
    return Arena_take(this, sizeof(T)*count);
}
template<typename T> void* Arena::take_array_zero(u64 count) {
    return Arena_take_zero(this, sizeof(T)*count);
}
void Arena::reset(u64 ps) {
    Arena_reset(this, ps);
}
void Arena::reset() {
    Arena_reset_all(this);
}
void Arena::reset_decommit(u64 ps) {
    Arena_reset_decommit(this, ps);
}
void Arena::reset_decommit() {
    Arena_reset_all_decommit(this);
}
#endif
/** **************************************/
#undef B_PTR
