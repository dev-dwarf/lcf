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
Arena* Arena_create_custom(u64 size) {
    Arena* a = (Arena*) LCF_MEMORY_reserve(size);
    LCF_MEMORY_commit(a, LCF_MEMORY_COMMIT_SIZE);
    a->size = size;
    a->pos = sizeof(Arena);
    a->commited_pos = 1;
    a->alignment = LCF_MEMORY_ALIGNMENT;
    return a;
}

Arena* Arena_create(void) {
    return Arena_create_custom(LCF_MEMORY_RESERVE_SIZE);
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
        upr commited_pos = a->commited_pos*LCF_MEMORY_COMMIT_SIZE;

        /* Commit memory if needed */
        if (new_pos > commited_pos) {
            upr new_commited_pos = next_alignment(new_pos, LCF_MEMORY_COMMIT_SIZE);
            if (LCF_MEMORY_commit(a, new_commited_pos)) {
                a->commited_pos = (s32)(new_commited_pos/LCF_MEMORY_COMMIT_SIZE);
                commited_pos = new_commited_pos;
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
    ASSERTM(pos < a->pos, "No need to reset Arena!");
    if (pos < a->pos) {
        pos = MAX(pos, sizeof(Arena));

        /* Decommit everything except what is needed, or one page */
        upr mem = (upr) a;
        upr needed_pos = (next_alignment(mem + a->pos, LCF_MEMORY_COMMIT_SIZE) - mem) ;
        upr over_commited = a->commited_pos*LCF_MEMORY_COMMIT_SIZE - needed_pos;
        if (needed_pos > 0 && over_commited >= LCF_MEMORY_DECOMMIT_THRESHOLD) {
            LCF_MEMORY_decommit(a+needed_pos, over_commited);
            a->commited_pos = (s32)(needed_pos / LCF_MEMORY_COMMIT_SIZE);
        }
        
        if (LCF_MEMORY_DEBUG_CLEAR) {
            /* Clear memory between pos and a->pos */
            MemorySet((void*) ((u64)a+pos), LCF_MEMORY_ARENA_CLEAR, a->pos - pos);
        }
        a->pos = pos;
    }
}

void Arena_reset_all(Arena *a) {
    Arena_reset(a, 0);
}

ArenaSession ArenaSession_begin(Arena *a) {
    ArenaSession s;
    s.Arena = a;
    s.save_point = a->pos;
    return s;
}

void ArenaSession_end(ArenaSession s) {
    Arena_reset(s.Arena, s.save_point);
}
/** **************************************/
#undef B_PTR
