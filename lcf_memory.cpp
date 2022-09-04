#include "lcf_memory.h"
#include <string.h> /* only for memset */

/** Arena ********************************/
Arena Arena_create_custom(u64 size, u64 commit_size) {
    Arena a;
    a.size = size;
    a.pos = 0;
    a.commited_pos = 0;
    a.memory = LCF_MEMORY_reserve(size);

    if (commit_size > 0) {
        LCF_MEMORY_commit(a.memory, commit_size);
        a.commited_pos = commit_size;
    }
    return a;
}

Arena Arena_create(u64 size) {
    Arena a;
    a.size = size;
    a.pos = 0;
    a.commited_pos = 0;
    a.memory = LCF_MEMORY_reserve(size);

    LCF_MEMORY_commit(a.memory, LCF_MEMORY_DEFAULT_COMMIT_SIZE);
    a.commited_pos = LCF_MEMORY_DEFAULT_COMMIT_SIZE;
    return a;
}

Arena Arena_create_default() {
    return Arena_create(LCF_MEMORY_DEFAULT_RESERVE_SIZE);
}

void Arena_destroy(Arena *a) {
    LCF_MEMORY_free(a->memory, a->size);
    *a = { 0 };
}

internal b32 is_power_of_2(u64 x) {
    return (x & (x-1) == 0);
}

internal u64 next_alignment(u64 ptr, u64 alignment) {
    ASSERT(is_power_of_2(alignment));

    /* Fast replacement for mod because alignment is power of 2 */
    u64 modulo = ptr & (alignment-1);

    if (modulo != 0) {
        ptr += alignment - modulo;
    }

    return ptr;
}

void* Arena_take_custom(Arena *a, u64 size, u64 alignment, u64 commit_size) {
    void* result = 0;
    /* Align pos pointer to check if "size" can fit */
    u64 mem = (u64) a->memory;
    u64 aligned_pos = next_alignment(mem + a->pos, alignment) - mem;
    u64 new_pos = aligned_pos + size;
        
    /* Check that there is space */
    if (aligned_pos + size < a->size) {
        u64 commited_pos = a->commited_pos;

        /* Commit memory if needed */
        if (new_pos >= commited_pos) {
            u64 new_commited_pos = next_alignment(mem + new_pos, commit_size);
            u64 commit_size = new_commited_pos - commited_pos;
            if (LCF_MEMORY_commit((u8*)(mem + commited_pos), commit_size)) {
                a->commited_pos = commited_pos = new_commited_pos;
            }
        }

        /* If enough memory is commited, set result and pos. */
        if (new_pos < commited_pos) {
            result = (void*)(mem + aligned_pos);
            a->pos = new_pos;
        }
    }
    ASSERT(result != 0);
    return result;
}

void* Arena_take(Arena *a, u64 size) {
    return Arena_take_custom(a, size, LCF_MEMORY_DEFAULT_ALIGNMENT, LCF_MEMORY_DEFAULT_COMMIT_SIZE);
}

void Arena_reset(Arena *a, u64 pos) {
    if (LCF_MEMORY_DEBUG_CLEAR) {
        /* Clear memory between pos and a->pos */
        memset((void*) ((u64)a->memory+pos), LCF_MEMORY_ARENA_CLEAR, a->pos - pos);
    }
    a->pos = pos;
}

void Arena_reset_all(Arena *a) {
    Arena_reset(a, 0);
}

void Arena_reset_decommit_custom(Arena *a, u64 pos, u64 commit_size) {
    Arena_reset(a, pos);

    /* If the new pos is on a previous page than what is commited,
       decommit the difference between commited_pos and what commited_pos needs to be
       to accomodate the new pos */
    u64 mem = (u64) a->memory;
    u64 commited_pos = a->commited_pos;
    u64 needed_commited_pos = next_alignment(mem + pos, commit_size) - mem;
    if (needed_commited_pos < commited_pos) {
        LCF_MEMORY_decommit((u8*)(mem+needed_commited_pos), commited_pos-needed_commited_pos);
        a->commited_pos = needed_commited_pos;
    }
}

void Arena_reset_decommit_custom(Arena *a, u64 pos) {
    Arena_reset_decommit_custom(a, pos, LCF_MEMORY_DEFAULT_COMMIT_SIZE);
}

void Arena_reset_all_decommit(Arena *a) {
    Arena_reset(a, 0);

    /* decommit all */
    LCF_MEMORY_decommit(a->memory, a->commited_pos);
    a->commited_pos = 0;
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
/** **************************************/


/** Bump *********************************/

Bump Bump_create(u64 size, void *memory) {
    Bump b;
    b.size = size;
    b.pos = 0;
    b.memory = memory;
    return b;
}

void Bump_create_inplace(u64 size, Bump *bump_memory) {
    Bump b;
    b.size = size-sizeof(Bump);
    b.pos = 0;
    b.memory = (u8*)((u64)bump_memory+sizeof(Bump));
    *bump_memory = b;
}

void* Bump_take_custom(Bump *b, u64 size, u64 alignment) {
   void* result = 0;
    /* Align pos pointer to check if "size" can fit */
    u64 mem = (u64) b->memory;
    u64 aligned_pos = next_alignment(mem + b->pos, alignment) - mem;
    u64 new_pos = aligned_pos + size;
    /* Check that there is space */
    if (new_pos < b->size) {
        result = (void*)(mem + aligned_pos);
        b->pos = new_pos;
    }
    ASSERT(result != 0);
    return result;
}

void* Bump_take(Bump *b, u64 size) {
    return Bump_take_custom(b, size, LCF_MEMORY_DEFAULT_ALIGNMENT);
}

void Bump_reset(Bump *b, u64 pos) {
    if (LCF_MEMORY_DEBUG_CLEAR) {
        /* Clear memory between pos and b->pos */
        memset((void*) ((u64)b->memory+pos), LCF_MEMORY_ARENA_CLEAR, b->pos - pos);
    }
    b->pos = pos;
}

void Bump_reset_all(Bump *b) {
    Bump_reset(b, 0);
}

BumpSession BumpSession_begin(Bump *b) {
    BumpSession s;
    s.bump = b;
    s.session_start = b->pos;
    return s;
}

void BumpSession_end(BumpSession s) {
    Bump_reset(s.bump, s.session_start);
}
/** **************************************/
