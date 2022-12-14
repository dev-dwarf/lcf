/** ************************************
  LCF, Created (September 03, 2022)

  Purpose:
  Virtual memory operations.
  Arena Memory management primitive using virtual memory.

  TODO:
  Pool Memory?

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

#include <stdlib.h>
#include <string.h> /* only for memset, memcpy */

#if !defined(MemoryCopy)
 #define MemoryCopy memcpy
 #define MemoryMove memmove
 #define MemorySet memset
#endif

#define MemoryCopyStruct(d,s) do { Assert(sizeof(*(d))==sizeof(*(s))); MemoryCopy((d),(s),sizeof(*(d))); } while(0)
#define MemoryCopyArray(d,s) do{ Assert(sizeof(d)==sizeof(s)); MemoryCopy((d),(s),sizeof(s)); }while(0)

#define MemoryZero(p,s) MemorySet((p), 0, (s))
#define MemoryZeroStruct(p) MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(a) MemoryZero((a), sizeof(a))

/** ******************************** **/


/** Backing Memory - Virtual Memory  **/
/* Definitions for Backing Memory Functions */
#define LCF_MEMORY_RESERVE_MEMORY(name) void* name(u64 size)
#define LCF_MEMORY_COMMIT_MEMORY(name) b32 name(void* memory, u64 size)
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

 internal LCF_MEMORY_RESERVE_MEMORY(_lcf_memory_default_reserve) {
     return malloc(size);
 }
 internal LCF_MEMORY_COMMIT_MEMORY(_lcf_memory_default_commit) {
     (void) size, memory;
     return 1; /* malloc commits memory automatically */
 }
 internal LCF_MEMORY_DECOMMIT_MEMORY(_lcf_memory_default_decommit) {
     (void) size, memory;
     return;
 }
 internal LCF_MEMORY_FREE_MEMORY(_lcf_memory_default_free) {
     (void) size;
     free(memory);
 }
 
 #define LCF_MEMORY_reserve _lcf_memory_default_reserve
 #define LCF_MEMORY_commit _lcf_memory_default_commit
 #define LCF_MEMORY_decommit _lcf_memory_default_decommit
 #define LCF_MEMORY_free _lcf_memory_default_free
#endif

#if !defined(LCF_MEMORY_RESERVE_SIZE)
 #define LCF_MEMORY_RESERVE_SIZE GB(1)
#endif
#if !defined(LCF_MEMORY_COMMIT_SIZE)
 #define LCF_MEMORY_COMMIT_SIZE KB(4)
#endif
#if !defined(LCF_MEMORY_ALIGNMENT)
 #define LCF_MEMORY_ALIGNMENT (sizeof(void*))
#endif
/** ******************************** **/


/* Macro to specify whether memory should be cleared
   For example, when enabled Arena_reset will clear
   memory beyond the Arena->pos, to try and force a crash
   if anyone is still holding on to it.
 */
#if !defined(LCF_MEMORY_DEBUG_CLEAR)
#define LCF_MEMORY_DEBUG_CLEAR 1
#define LCF_MEMORY_ARENA_CLEAR 0xCF
#endif

/* NOTE: study ryan fleury arena. In particular, why are arenas a linked list?
   Try to understand the advantage offered by doing it that way and how it might
   affect usage code */

/** Arena Allocator
    "Arena" provides managed memory blocks. The backing memory functions provided above
    are used to automatically reserve, commit/decommit, and free virtual memory.
    There is some granularity for the user w.r.t aligning/commiting/decommiting memory.

    There are also procs to "reset" the arena to a certain position, allowing the memory
    to be used again by the arena. These are wrapped by the ARENA_SESSION macro and
    ArenaSession_(begin|end) procs to cover the most common scenario.
                                     **/
struct lcf_Arena {
    u64 pos;
    u64 size;
    u64 alignment;
    u64 commited_pos;
};
typedef struct lcf_Arena Arena;

/* Create and destroy Arenas */
Arena* Arena_create_default(void); 
Arena* Arena_create(u64 size);
void Arena_destroy(Arena *a); 

/* Take memory from the arena */
void* Arena_take(Arena *a, u64 size);
void* Arena_take_custom(Arena *a, u64 size, u64 alignment);
void* Arena_take_zero(Arena *a, u64 size);
void* Arena_take_zero_custom(Arena *a, u64 size, u64 alignment);
#define Arena_take_array(a, type, count) ((type*) Arena_take(a, sizeof(type)*count))
#define Arena_take_array_zero(a, type, count) ((type*) Arena_take_zero(a, sizeof(type)*count))
#define Arena_take_struct(a, type) ((type*) Arena_take(a, sizeof(type)))
#define Arena_take_struct_zero(a, type) ((type*) Arena_take_zero(a, sizeof(type)))

/* Reset arena to a certain position */
void Arena_reset(Arena *a, u64 pos);
void Arena_reset_all(Arena *a);
void Arena_reset_decommit_custom(Arena *a, u64 pos, u64 commit_size);
void Arena_reset_decommit(Arena *a, u64 pos);
void Arena_reset_all_decommit(Arena *a);

/* Resize blocks without destroying data.
   (only a performance savings when old_memory was most recently taken block) */
void* Arena_resize(Arena *a, void* old_memory, u64 old_size, u64 new_size);
void* Arena_resize_custom(Arena *a, void* old_memory, u64 old_size, u64 new_size, u64 alignment);

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
#endif
