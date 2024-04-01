#include "lcf_memory.h"
#define B_PTR(p) (u8*)(p)

internal s32 is_power_of_2(u64 x) {
    return ((x & (x-1)) == 0);
}

internal u64 next_alignment(u8* mem, u64 offset, u64 alignment) {
    ASSERTM(is_power_of_2(alignment), "Alignments must be a power of 2.");

    u64 ptr = ((upr) mem + offset);
    
    /* Fast replacement for mod because alignment is power of 2 */
    u64 modulo = ptr & (alignment-1);

    if (modulo != 0) {
        ptr += alignment - modulo;
    }

    return ptr - (upr) mem;
}

Arena* Arena_create_custom(Arena params) {
    ASSERT(is_power_of_2(params.commit_size));
    
    u64 reserve_size = next_alignment(0, params.size, params.commit_size);
    Arena* a = (Arena*) LCF_MEMORY_reserve(reserve_size);

    u64 commit_pos = params.commit_pos? next_alignment(a, params.commit_pos, params.commit_size) : commit_size;
    LCF_MEMORY_commit(a, commit_pos);
    
    *a = params;
    a->commit_pos = commit_pos;
    a->size = reserve_size;
    return a;
}

void Arena_destroy(Arena *a) {
    LCF_MEMORY_free(a, a->size);
}

void* Arena_take_custom(Arena *a, u64 size, u32 alignment) {
    void* result = 0;
    
    /* Align pos pointer */
    u8 *mem = Arena_mem_start(a);
    u64 aligned_pos = next_alignment(mem, a->pos, alignment);
    u64 new_pos = aligned_pos + size;

    /* Check that there is space */
    if (new_pos < a->size - sizeof(Arena)) {
        /* Commit memory if needed */
        s32 in_commit_range = new_pos <= a->commit_pos;
        if (!in_commit_range) {
            upr new_commit_pos = next_alignment(mem, new_pos, a->commit_size);
            in_commit_range = LCF_MEMORY_commit(mem, new_commit_pos); 
            a->commit_pos = new_commit_pos;
        }
        if (in_commit_range) {
            result = mem + aligned_pos;
            a->pos = new_pos;
        }
    }
    
    ASSERT(result); // Arena out of memory!
    return result;
}

inline void* Arena_take(Arena *a, u64 size) {
    return Arena_take_custom(a, size, a->alignment);
}

inline void* Arena_take_zero_custom(Arena *a, u64 size, u32 alignment) {
    void* mem = Arena_take_custom(a, size, alignment);
    memset(mem, 0, size);
    return mem;
}

inline void* Arena_take_zero(Arena *a, u64 size) {
    void* mem = Arena_take_custom(a, size, a->alignment);
    memset(mem, 0, size);
    return mem;
}

void Arena_reset(Arena *a, u64 pos) {
    if (LCF_MEMORY_DEBUG_CLEAR) {
        if (pos < a->pos) {
            /* Clear memory between pos and a->pos */
            memset(Arena_mem_start(a) + pos, LCF_MEMORY_ARENA_CLEAR, a->pos - pos);
        }
    }
    a->pos = pos;
}

void Arena_decommit(Arena *a, u64 needed_pos) {
    u8 *mem = Arena_mem_start(a);
    if (!needed_pos) {
        /* Decommit everything except what is needed, or one page */
        needed_pos = MAX(a->pos, a->commit_size);
    } else {
        /* Should never decommit currently in use memory! */
        ASSERT(needed_pos > a->pos);
    }
    
    u64 new_commit_pos = next_alignment(mem, needed_pos, a->commit_size);
    u64 over_commited = a->commit_pos - new_commit_pos;
    if (over_commited >= a->commit_size) {
        LCF_MEMORY_decommit(mem+new_commit_pos, over_commited);
        a->commit_pos = new_commit_pos;
    }
}

void Arena_resetp(Arena *a, void* previous_alloc) {
    u64 pos = ((u8*)(previous_alloc) - Arena_mem_start(a));
    Arena_reset(a, pos);
}

ArenaSession ArenaSession_begin(Arena *a) {
    ArenaSession s;
    s.arena = a;
    s.save_point = a->pos;
    return s;
}

void ArenaSession_end(ArenaSession s) {
    if (s.arena) {
        Arena_reset(s.arena, s.save_point);
    }
}

/* Scratch Memory */
#define LCF_SCRATCH_COUNT 2
per_thread Arena* _arena_scratch_pool[LCF_SCRATCH_COUNT];
void Arena_thread_init_scratch() {
    if (_arena_scratch_pool[0] == 0) {
        for (s32 i = 0; i < LCF_SCRATCH_COUNT; i++) {
            _arena_scratch_pool[i] = Arena_default();
        }
    }
}

Arena* Arena_scratch_custom(Arena** conflicts, s32 n) {
    Arena_thread_init_scratch();
    Arena* out = 0;
    for (s32 i = 0; i < LCF_SCRATCH_COUNT; i++) {
        out = _arena_scratch_pool[i];
        for (s32 j = 0; j < n; j++) {
            if (out == conflicts[j]) {
                out = 0;
            }
        }

        if (out != 0) {
            break;
        }
    }
    return out;
}

ArenaSession Scratch_session_custom(Arena** conflicts, s32 n) {
    ArenaSession out = ZERO_STRUCT;
    Arena *s = Arena_scratch_custom(conflicts, n);
    if (s) {
        out = ArenaSession_begin(s);
    }
    return out;
}

#undef B_PTR
