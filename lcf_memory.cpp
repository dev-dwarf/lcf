#include "lcf_memory.h"
#include <string.h> /* only for memset, memcpy */
#define B_PTR(p) (u8*)(p)

internal b32 is_power_of_2(u64 x) {
    return ((x & (x-1)) == 0);
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

Arena Arena_create_default(void) {
    return Arena_create(LCF_MEMORY_DEFAULT_RESERVE_SIZE);
}

void Arena_destroy(Arena *a) {
    LCF_MEMORY_free(a->memory, a->size);
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
            u64 need_to_commit_size = new_commited_pos - commited_pos;
            if (LCF_MEMORY_commit(B_PTR(mem + commited_pos), need_to_commit_size)) {
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
        LCF_MEMORY_decommit(B_PTR(mem+needed_commited_pos), commited_pos-needed_commited_pos);
        a->commited_pos = needed_commited_pos;
    }
}

void Arena_reset_decommit(Arena *a, u64 pos) {
    Arena_reset_decommit_custom(a, pos, LCF_MEMORY_DEFAULT_COMMIT_SIZE);
}

void Arena_reset_all_decommit(Arena *a) {
    Arena_reset(a, 0);

    /* decommit all */
    LCF_MEMORY_decommit(a->memory, a->commited_pos);
    a->commited_pos = 0;
}

void* Arena_resize_custom(Arena *a, void* old_memory, u64 old_size, u64 new_size, u64 alignment, u64 commit_size) {
    void* result = 0;

    if (old_memory == 0 || old_size == 0) {
        return Arena_take_custom(a, new_size, alignment, commit_size);
    } else if ((a->memory <= old_memory) && (B_PTR(old_memory) < B_PTR(a->memory)+a->size)) {
        /* Check that old_memory was most recently taken block */
        if (B_PTR(old_memory) == (B_PTR(a->memory) + (a->pos - old_size))) {
            result = old_memory;
            a->pos = a->pos - old_size + new_size;
            if (old_size < new_size) {
                memset(B_PTR(old_memory)+old_size, 0, new_size-old_size);
            }
        } else {
            result = Arena_take_custom(a, new_size, alignment, commit_size);
            memcpy(result, old_memory, old_size);
        }
    } else {
        ASSERT(0 && "old_memory is not within the bounds of the Arena's buffer.");
    }

    return result;
}

void* Arena_resize(Arena *a, void* old_memory, u64 old_size, u64 new_size) {
    return Arena_resize_custom(a, old_memory, old_size, new_size,
                               LCF_MEMORY_DEFAULT_ALIGNMENT, LCF_MEMORY_DEFAULT_COMMIT_SIZE);
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
/** ****w**********************************/


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
    b.memory = B_PTR((u64)bump_memory+sizeof(Bump));
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

void* Bump_resize_custom(Bump *b, void* old_memory, u64 old_size, u64 new_size, u64 alignment) {
    void* result = 0;

    if (old_memory == 0 || old_size == 0) {
        return Bump_take_custom(b, new_size, alignment);
    } else if ((b->memory <= old_memory) && (B_PTR(old_memory) < B_PTR(b->memory)+b->size)) {
        /* Check that old_memory was most recently taken block */
        if (B_PTR(old_memory) == (B_PTR(b->memory) + (b->pos - old_size))) {
            result = old_memory;
            b->pos = b->pos - old_size + new_size;
            if (old_size < new_size) {
                memset(B_PTR(old_memory)+old_size, 0, new_size-old_size);
            }
        } else {
            result = Bump_take_custom(b, new_size, alignment);
            memcpy(result, old_memory, old_size);
        }
    } else {
        ASSERT(0 && "old_memory is not within the bounds of the Bump's buffer.");
    }

    return result;
}

void* Bump_resize(Bump *b, void* old_memory, u64 old_size, u64 new_size) {
    return Bump_resize_custom(b, old_memory, old_size, new_size,
                              LCF_MEMORY_DEFAULT_ALIGNMENT);
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

#undef B_PTR
