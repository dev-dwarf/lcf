#if !defined(LCF_MEMORY)
#define LCF_MEMORY "1.0.0"

#include "lcf_types.h"
#include <string.h> /* only for memset, memcpy */

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

#define LCF_MEMORY_RESERVE_MEMORY(name) void* name(upr size)
#define LCF_MEMORY_COMMIT_MEMORY(name) b32 name(void* memory, upr size)
#define LCF_MEMORY_DECOMMIT_MEMORY(name) void name(void* memory, upr size)
#define LCF_MEMORY_FREE_MEMORY(name) void name(void* memory, upr size)

/* Provide default backing memory functions using stdlib to help get off the ground
   NOTE(lcf): These should absolutely be provided by OS layer to take advantage of
       virtual memory.
 */
#if !defined(LCF_MEMORY_PROVIDE_MEMORY)
 #include <stdlib.h>
 #define LCF_MEMORY_PROVIDE_MEMORY "stdlib"

 internal LCF_MEMORY_RESERVE_MEMORY(_lcf_memory_stdlib_reserve) {
     return malloc(size);
 }
 internal LCF_MEMORY_COMMIT_MEMORY(_lcf_memory_stdlib_commit) {
     (void) size, memory;
     return 1; /* malloc commits memory automatically */
 }
 internal LCF_MEMORY_DECOMMIT_MEMORY(_lcf_memory_stdlib_decommit) {
     (void) size, memory;
     return;
 }
 internal LCF_MEMORY_FREE_MEMORY(_lcf_memory_stdlib_free) {
     (void) size;
     free(memory);
 }
 
 #define LCF_MEMORY_reserve _lcf_memory_stdlib_reserve
 #define LCF_MEMORY_commit _lcf_memory_stdlib_commit
 #define LCF_MEMORY_decommit _lcf_memory_stdlib_decommit
 #define LCF_MEMORY_free _lcf_memory_stdlib_free
#endif

#if !defined(LCF_MEMORY_ARENA_SIZE)
 #define LCF_MEMORY_ARENA_SIZE GB(1)
#endif
#if !defined(LCF_MEMORY_COMMIT_SIZE)
 #define LCF_MEMORY_COMMIT_SIZE KB(4)
#endif
#if !defined(LCF_MEMORY_ALIGNMENT)
 #define LCF_MEMORY_ALIGNMENT (sizeof(void*))
#endif

/** Macro to specify whether memory should be cleared. For example, when enabled Arena_reset will 
    clear memory beyond the Arena->pos, to try and force a crash if anyone is still holding on to it.
 **/
#if !defined(LCF_MEMORY_DEBUG_CLEAR)
#define LCF_MEMORY_DEBUG_CLEAR 1
#define LCF_MEMORY_ARENA_CLEAR 0xCF
#endif

struct Arena {
    u64 pos;
    u64 size;
    u64 commit_pos; 
    u64 commit_size;
    u32 alignment;
};
typedef struct Arena Arena;

/* Create and destroy Arenas */
Arena* Arena_create(void); 
Arena* Arena_create_custom(Arena params);

#define Arena_default() Arena_create_custom((Arena){.size = LCF_MEMORY_ARENA_SIZE, .commit_size = LCF_MEMORY_COMMIT_SIZE, .alignment = LCF_MEMORY_ALIGNMENT}) 
#ifndef __cplusplus // TODO(lcf) this cant be used in c++. Do I care?
#define Arena_create(...) Arena_create_custom((Arena){ \
    .size = LCF_MEMORY_ARENA_SIZE, \
    .commit_size = LCF_MEMORY_COMMIT_SIZE, \
    .alignment = LCF_MEMORY_ALIGNMENT, \
    __VA_ARGS__ \
})
#endif
void Arena_destroy(Arena *a);

/* Take memory from the Arena */
void* Arena_take(Arena *a, u64 size);
inline void* Arena_take_custom(Arena *a, u64 size, u32 alignment);
inline void* Arena_take_zero(Arena *a, u64 size);
inline void* Arena_take_zero_custom(Arena *a, u64 size, u32 alignment);
#define Arena_take_array(a, type, count) ((type*) Arena_take(a, sizeof(type)*(count)))
#define Arena_take_array_zero(a, type, count) ((type*) Arena_take_zero(a, sizeof(type)*(count)))
#define Arena_take_struct(a, type) ((type*) Arena_take(a, sizeof(type)))
#define Arena_take_struct_zero(a, type) ((type*) Arena_take_zero(a, sizeof(type)))
#define Arena_mem_start(a) (((u8 *)a) + sizeof(Arena))

/* Reset Arena to a certain position */
void Arena_reset(Arena *a, u64 pos);
void Arena_resetp(Arena *a, void* previous_alloc);

/* Arena sessions - wraps resetting memory */
struct ArenaSession {
    Arena *arena;
    u64 save_point;
};
typedef struct ArenaSession ArenaSession;

ArenaSession ArenaSession_begin(Arena *a);
void ArenaSession_end(ArenaSession s);
#define ARENA_SESSION(Arena) DEFER_LOOP( \
        ArenaSession MACRO_VAR(session) = ArenaSession_begin(Arena),    \
        ArenaSession_end(MACRO_VAR(session)) \
        )

/* Scratch Memory */
void Arena_thread_init_scratch();
Arena* Arena_scratch_custom(Arena** conflicts, s32 n);
ArenaSession Scratch_session_custom(Arena** conflicts, s32 n);
#define Arena_scratch() Arena_scratch_custom(0, 0)
#define Scratch_session() Scratch_session_custom(0, 0)
#define SCRATCH_SESSION(session) DEFER_LOOP( \
    ArenaSession session = Scratch_session(), \
    ArenaSession_end(session))

/* Implements internal Stack and Queue operations on linked lists. Implemented as macros
   to be useful with arbitrary data structures in C and C++ */
#define lcfNextsym next
#define lcfZerosym 0 
#define lcfCheckNull(p) ((p) == lcfZerosym)
#define lcfNullify(p) ((p) = lcfZerosym)

/* Scheming : ] */
#define PushSCustom(first,node,nextsym)             \
    ((node)->nextsym = (first), (first) = (node))
#define PopSCustom(first,out,nextsym,checkfun)              \
    (checkfun(first)? zerosym :                             \
     ((out) = (first), (first) = (first)->nextsym),  (out))
#define PushQCustom(first,last,node,nextsym,checkfun,nullfun)   \
    ((checkfun(last)? ((last) = (first) = (node))                 \
      : ((last) = (last)->nextsym = (node))),            \
      nullfun((node)->nextsym))
#define PushQFrontCustom(first,last,node,nextsym,checkfun,nullfun)      \
    (checkfun(first)? ((first) = (last) = (node), nullfun((node)->nextsym)) \
     : ((node)->nextsym = (first), (first) = (node)))
#define PopQCustom(first,last,out,nextsym,nullfun)                      \
    (((out) = (last),                                                  \
        ((first) == (last))? (nullfun(first),nullfun(last))            \
                              : ((first) = (first)->nextsym)),         \
     (out))

#define PushS(l,n) PushSCustom((l)->first,n,lcfNextsym)
#define PopS(l) PopSCustom((l)->first,lcfNextsym,lcfCheckNull,lcfZerosym)
#define PushQ(l,n) PushQCustom((l)->first,(l)->last,n,lcfNextsym,lcfCheckNull,lcfNullify)
#define PushQFront(l,n) PushQFrontCustom((l)->first,(l)->last,n,lcfNextsym,lcfCheckNull,lcfNullify)
#define PopQ(l,o) PopQCustom((l)->first,(l)->last,o,lcfNextsym,lcfNullify)

/* TODO: doubly linked list macros (haven't needed them yet) */
/* TODO: concat two lists */

#endif
