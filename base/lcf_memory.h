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
#define LCF_MEMORY_RESERVE_MEMORY(name) void* name(upr size)
#define LCF_MEMORY_COMMIT_MEMORY(name) b32 name(void* memory, upr size)
#define LCF_MEMORY_DECOMMIT_MEMORY(name) void name(void* memory, upr size)
#define LCF_MEMORY_FREE_MEMORY(name) void name(void* memory, upr size)

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
#if !defined(LCF_MEMORY_DECOMMIT_THRESHOLD)
#define LCF_MEMORY_DECOMMIT_THRESHOLD (16*(LCF_MEMORY_COMMIT_SIZE))
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

/* NOTE: study ryan fleury Arena. In particular, why are Arenas a linked list?
   Need to understand the advantage offered by doing it that way and how it might
   affect usage code */

/** Arena Allocator
    "Arena" provides managed memory blocks. The backing memory functions provided above
    are used to automatically reserve, commit/decommit, and free virtual memory.
    There is some granularity for the user w.r.t aligning/commiting/decommiting memory.

    There are also procs to "reset" the Arena to a certain position, allowing the memory
    to be used again by the Arena. These are wrapped by the ARENA_SESSION macro and
    ArenaSession_(begin|end) procs to cover the most common scenario.
                                     **/
struct Arena {
    upr pos;
    u64 size;
    s32 alignment;
    s32 commited_pos; /* In pages (LCF_MEMORY_COMMIT_SIZE) */
};
typedef struct Arena Arena;

/* Create and destroy Arenas */
Arena* Arena_create(void); 
Arena* Arena_create_custom(u64 size);
void Arena_destroy(Arena *a); 

/* Take memory from the Arena */
void* Arena_take(Arena *a, u64 size);
void* Arena_take_custom(Arena *a, u64 size, u32 alignment);
void* Arena_take_zero(Arena *a, u64 size);
void* Arena_take_zero_custom(Arena *a, u64 size, u32 alignment);
#define Arena_take_array(a, type, count) ((type*) Arena_take(a, sizeof(type)*count))
#define Arena_take_array_zero(a, type, count) ((type*) Arena_take_zero(a, sizeof(type)*count))
#define Arena_take_struct(a, type) ((type*) Arena_take(a, sizeof(type)))
#define Arena_take_struct_zero(a, type) ((type*) Arena_take_zero(a, sizeof(type)))

/* Reset Arena to a certain position */
void Arena_reset(Arena *a, u64 pos);
void Arena_reset_all(Arena *a);

/* Arena sessions - wraps resetting memory */
struct ArenaSession {
    Arena *Arena;
    u64 save_point;
};
typedef struct ArenaSession ArenaSession;

ArenaSession ArenaSession_begin(Arena *a);
void ArenaSession_end(ArenaSession s);
#define ARENA_SESSION(Arena) DEFER_LOOP( \
        ArenaSession MACRO_VAR(session) = ArenaSession_begin(Arena),    \
        ArenaSession_end(MACRO_VAR(session)) \
        )

/** ******************************** **/

/** Singly and Doubly Linked Lists   **/
/* Implements internal Stack and Queue operations on linked lists. Implemented as macros
   to be useful with arbitrary data structures in C and C++ */
#define Nextsym next
#define Zerosym 0 
#define CheckNull(p) ((p) == Zero)
#define Nullify(p) ((p) = Zero)

/* Scheming : ] */
#define PushFCustom(first,node,nextsym)             \
    ((node)->nextsym = (first), (first) = (node))
#define PopFCustom(first,out,nextsym,checkfun)              \
    (checkfun(first)? zerosym :                             \
     ((out) = (first), (first) = (first)->nextsym),  (out))
#define PushQCustom(first,last,node,nextsym,checkfun,nullfun)   \
    ((checkfun(last)? ((first) = (last) = (node))               \
      : ((last)->nextsym = (node), (last) = (node))),           \
     nullfun((node)->nextsym))
#define PushQFrontCustom(first,last,node,nextsym,checkfun,nullfun)      \
    (checkfun(first)? ((first) = (last) = (node), nullfun((node)->nextsym)) \
     : ((node)->nextsym = (first), (first) = (node)))
#define PopQCustom(first,last,out,nextsym,nullfun)                      \
    (((out) = (first),                                                  \
        ((first) == (last))? ((nullfun(first),nullfun(last))            \
                              : ((first) = (first)->nextsym))),         \
     (out))

#define PushF(f,n) PushFCustom(f,n,Nextsym)
#define PopF(f) PopFCustom(f,Nextsym,CheckNull,Zerosym)
#define PushQ(f,l,n) PopFCustom(f,l,n,Nextsym,CheckNull,Nullify)
#define PushQFront(f,l,n) PushQFrontCustom(f,l,n,Nextsym,CheckNull,Nullify)
#define PopQ(f,l,o) PopQCustom(f,l,o,Nextsym,CheckNull,Nullify,Zerosym)

/* TODO: doubly linked list macros (haven't needed them yet) */
/* TODO: concat two lists */

#undef Zero
#undef CheckNull
#undef Nullify

/** ******************************** **/
#endif
