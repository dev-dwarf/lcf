/** ************************************
  LCF, Created (September 03, 2022)

  Purpose:
  Abstract over virtual memory for use by allocators. Can (Should) be overriden by OS layer.
  Using virtual memory abstraction, provide managed Arena allocator.
  Provide simple Bump allocator, and fixed-size memory Pool allocator, both using user-memory.
  
  Changelog:

************************************ **/
#if !defined(LCF_MEMORY)
#define LCF_MEMORY "1.0.0"

#include "lcf_types.h"

/** Memory Helper Macros             **/
/* Sizes */
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

/** ******************************** **/


/** Backing Memory - Virtual Memory  **/
/* Definitions for Backing Memory Functions */
#define LCF_MEMORY_RESERVE_MEMORY(name) void* name(u64 size)
#define LCF_MEMORY_COMMIT_MEMORY(name) b32 name(void* start_address, u64 size)
#define LCF_MEMORY_DECOMMIT_MEMORY(name) void name(void* memory, u64 size)
#define LCF_MEMORY_FREE_MEMORY(name) void name(void* memory, u64 size)
/* typedef LCF_MEMORY_RESERVE_MEMORY(ReserveBackingMemory); */
/* typedef LCF_MEMORY_CHANGE_MEMORY(ChangeBackingMemory); */

/* Provide default backing memory functions using stdlib to help get off the ground
   NOTE(lcf): These should absolutely be provided by OS layer to take advantage of
       virtual memory.
 */
#if !defined(LCF_MEMORY_PROVIDE_MEMORY)
#define LCF_MEMORY_PROVIDE_MEMORY "stdlib"
#include <stdlib.h>

internal LCF_MEMORY_RESERVE_MEMORY(_lcf_memory_default_reserve) {
    return malloc(size);
}
internal LCF_MEMORY_COMMIT_MEMORY(_lcf_memory_default_commit) {
    return 1; /* malloc commits memory automatically */
}
internal LCF_MEMORY_DECOMMIT_MEMORY(_lcf_memory_default_decommit) {
    return;
}
internal LCF_MEMORY_FREE_MEMORY(_lcf_memory_default_free) {
    free(memory);
}

/* FIXME: may want a vtable for these instead, allowing different arenas to have different
   types of backing memory. Allen originally coded this way, but then removed it. Why? */
#define LCF_MEMORY_reserve _lcf_memory_default_reserve
#define LCF_MEMORY_commit _lcf_memory_default_commit
#define LCF_MEMORY_decommit _lcf_memory_default_decommit
#define LCF_MEMORY_free _lcf_memory_default_free
#define LCF_MEMORY_PAGE_SIZE KB(4)
#define LCF_MEMORY_DEFAULT_RESERVE_SIZE GB(1)
#define LCF_MEMORY_DEFAULT_COMMIT_SIZE (4*LCF_MEMORY_PAGE_SIZE)
#define LCF_MEMORY_DEFAULT_ALIGNMENT (2*sizeof(void*))
#endif
/** ******************************** **/


/* TODO:
 * Pool REF: https://www.gingerbill.org/article/2019/02/15/memory-allocation-strategies-003/#fnref:3
 * Memory allocator struct/type that can be used as parameter
 * Stack REF: https://www.gingerbill.org/article/2019/02/15/memory-allocation-strategies-003/
 * Linked List

 Functions that need allocation facilities should take an arena as parameter.
 Allocators should work with a backing block of OS-allocated memory, that can be passed in
 by platform layer.
 */

/* Macro to specify whether memory should be cleared
   For example, when enabled Arena_reset will clear
   memory beyond the Arena->pos, to try and force a crash
   if anyone is still holding on to it.
 */
#if !defined(LCF_MEMORY_DEBUG_CLEAR)
#define LCF_MEMORY_DEBUG_CLEAR 1
#define LCF_MEMORY_ARENA_CLEAR 0xCF
#endif

/** Arena Allocator
    "Arena" provides managed memory blocks. The backing memory functions provided above
    are used to automatically reserve, commit/decommit, and free virtual memory.
    There is some granularity for the user w.r.t aligning/commiting/decommiting memory.

    There are also procs to "reset" the arena to a certain position, allowing the memory
    to be used again by the arena. These are wrapped by the ARENA_SESSION macro and
    ArenaSession_(begin|end) procs to cover the most common scenario.
                                     **/
struct lcf_Arena {
    u64 size;
    u64 pos;
    u64 commited_pos;
    void *memory;
};
typedef struct lcf_Arena Arena;

/* Create and destroy Arenas */
Arena Arena_create_default(void); 
Arena Arena_create(u64 size);
Arena Arena_create_custom(u64 size, u64 commit_size);
void Arena_destroy(Arena *a); 

/* Take memory from the arena */
void* Arena_take(Arena *a, u64 size);
void* Arena_take_custom(Arena *a, u64 size, u64 alignment, u64 commit_size);

/* Reset arena to a certain position */
void Arena_reset(Arena *a, u64 pos);
void Arena_reset_all(Arena *a);
void Arena_reset_decommit_custom(Arena *a, u64 pos, u64 commit_size);
void Arena_reset_decommit(Arena *a, u64 pos);
void Arena_reset_all_decommit(Arena *a);

/* Resize blocks without destroying data.
   (only a performance savings when old_memory was most recently taken block) */
void* Arena_resize(Arena *a, void* old_memory, u64 old_size, u64 new_size);
void* Arena_resize_custom(Arena *a, void* old_memory, u64 old_size, u64 new_size, u64 alignment, u64 commit_size);
/* Arena sessions - wraps resetting memory */
struct lcf_ArenaSession {
    Arena *arena;
    u64 session_start;
};
typedef struct lcf_ArenaSession ArenaSession;

ArenaSession ArenaSession_begin(Arena *a);
void ArenaSession_end(ArenaSession s);

#define ARENA_SESSION(arena, code) {                                    \
        ArenaSession session##__FILE__##__LINE__ = ArenaSession_begin(arena); \
        {code}                                                          \
        ArenaSession_end(session##__FILE__##__LINE__);                  \
    }

/** ******************************** **/

/** Bump Allocator
    "Bump" manages a user-provided chunk of memory. Functionality is similar to "Arena",
    however there is no virtual memory features. If you are not looking to provide your
    own block of memory, use "Arena" instead.
                                     **/
struct lcf_Bump {
    u64 size;
    u64 pos;
    void* memory;
};
typedef struct lcf_Bump Bump;

/* Create Bumps */
Bump Bump_create(u64 size, void *memory);
void Bump_create_inplace(u64 memory_size, Bump *bump_memory);

/* Take and Reset */
void* Bump_take(Bump *b, u64 size);
void* Bump_take_custom(Bump *b, u64 size, u64 alignment);
void Bump_reset(Bump *b, u64 pos);
void Bump_reset_all(Bump *b);

/* Resize blocks without destroying data.
   (only a performance savings when old_memory was most recently taken block) */
void* Bump_resize(Bump *b, void* old_memory, u64 old_size, u64 new_size);
void* Bump_resize_custom(Bump *b, void* old_memory, u64 old_size, u64 new_size, u64 alignment);

/* Bump Sessions */
struct lcf_BumpSession {
    Bump* bump;
    u64 session_start;
};
typedef struct lcf_BumpSession BumpSession;
BumpSession BumpSession_begin(Bump *b);
void BumpSession_end(BumpSession s);

#define BUMP_SESSION(bump, code) {                                     \
        BumpSession session##__FILE__##__LINE__ = BumpSession_begin(bump); \
        {code}                                                          \
        BumpSession_end(session##__FILE__##__LINE__);                   \
    }

/** ******************************** **/

#endif
