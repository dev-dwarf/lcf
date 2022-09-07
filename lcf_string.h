/** ************************************
  LCF, Created (September 04, 2022)

  Purpose:
  Provide ASCII and Unicode strings through str (immutable view), strList (linked-list of str)
  , and String (advanced builder), all using length-based structs.
  Integrations with lcf_memory allocators.
  Lots of string manipulation functionality.
  Conversion to null-terminated strings for commpatibility.
  
  Changelog:

************************************ **/
#if !defined(LCF_STRING)
#define LCF_STRING "1.0.0"

#include "lcf_types.h"
#include "lcf_memory.h"

/** ASCII                            **/
typedef u8 char8;
struct lcf_str8 { /* TODO(lcf): spread operator macro?? */
    u64 len;
    char8 *str;
};
typedef struct lcf_str8 str8;

struct lcf_str8Node {
    struct lcf_str8Node *next;
    struct lcf_str8 string;
};
struct lcf_str8List {
    struct lcf_str8Node *first;
    struct lcf_str8Node *last;
    u64 count;
    u64 size; // ?
};
typedef struct lcf_str8Node str8Node;
typedef struct lcf_str8List str8List;

struct lcf_String {
    str8List list; // TODO(lcf) is String even separate? or just str and strList
};

/* Create strs */
str8 str8_from(u8* s, u64 len);
str8 str8_from_pointer_range(u8 *p1, u8 *p2);
str8 str8_from_cstring(u8 *cstr);
str8 str8_empty(void);
#define str8_lit(s) ((str8){sizeof(s)-1, (u8*)(s)}) /* -1 to exclude null character */

/* Basic/fast operations */
str8 str8_first(str8 s, u64 len); /* return first len chars of str */
str8 str8_last(str8 s, u64 len); /* return last len chars of str */
str8 str8_cut_first(str8 s, u64 len); /* return str with len chars removed from the start */
str8 str8_cut_last(str8 s, u64 len); /* return str with len chars removed from the end */
str8 str8_skip(str8 s, u64 len); /* skip over len chars of str */
str8 str8_substr_between(str8 s, u64 start, u64 end); /* return str[start, end) as a str8 */
str8 str8_substr(str8 s, u64 start, u64 n); /* return str[start, start+n-1] */

/* Operations that need memory */
str8 str8_copy(Arena *a, str8 s);
str8 str8_copy_custom(void* memory, str8 s);
str8 str8_concat(Arena *a, str8 s1, str8 s2);
str8 str8_concat_custom(void *memory, str8 s1, str8 s2);
/* TODO(lcf): variadic/list concat/custom */
/* TODO(lcf): (maybe) add operations for Bump,
   alternatively, make functions that take allocator struct. */
/* We could make a string allocator context?? */

/* Comparisons / Predicates */
#define str8_is_empty(s) ((b32)((s).len == 0))
#define str8_not_empty(s) ((b32)((s.len) != 0))
b32 str8_eq(str8 a, str8 b);
b32 str8_has_prefix(str8 s, str8 prefix);
b32 str8_has_postfix(str8 s, str8 postfix);
b32 char8_is_whitespace(char8 c);
b32 str8_contains_char(str8 s, char8 c);
b32 str8_contains_substring(str8 s, str8 sub);
b32 str8_contains_delimiter(str8 s, str8 delims);
#define LCF_STRING_NO_MATCH 0x80000000
u64 str8_char_location(str8 s, char8 c);
u64 str8_substring_location(str8 s, str8 sub);
u64 str8_delimiter_location(str8 s, str8 delims); 

/* Conditional Operations */
str8 str8_trim_prefix(str8 s, str8 prefix);
str8 str8_trim_postfix(str8 s, str8 postfix);
str8 str8_trim_whitespace(str8 s);
str8 str8_trim_whitespace_front(str8 s);
str8 str8_trim_whitespace_back(str8 s);

/* Iterations */
#define str8_iter_custom(s, i, c)                            \
    u64 i = 0;                                               \
    char8 c = s.str[0];                                      \
    for (; (i < s.len) && ((c = s.str[i]) || true); i++)
#define str8_iter(s) str8_iter_custom(s, i, c);

/* TODO(lcf): macro for backwards iteration over string */

/* Find the first substring split_by, and return everything before it,
   advancing src to after the substring. */
str8 str8_pop_first_split_substring(str8 *src, str8 split_by);
#define str8_iter_splits_custom(s, split_by, iter)                      \
    for (                                                               \
        str8 MACRO_VAR(_str) = (s),                                     \
            MACRO_VAR(_split_by) = (split_b),                           \
            iter = str8_pop_first_split_substrings(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
            ;                                                           \
        !str_empty(MACRO_VAR(_str))                                     \
            ;                                                           \
        iter = str8_pop_first_split(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
        )
#define str8_iter_split_substring(s, split_by) str8_iter_split_substring_custom(s, split_by, it)

/* TODO(lcf): split by delimiter, iter split delimiters
   TODO(lcf): split by whitespace, iter split whitespace
*/

/* Printing and Formatting */
#include <stdio.h>
enum lcf_str8OutputType { /* TODO(lcf): rename these */
    STR8_FIXED, /* User passes fixed size str8. Truncates once size is reached. */
    STR8_ARENA, /* User passes Arena. buf grows to fit data. output str is {buf_len, buf} */
    STDIO_FILE /* Output to FILE stream. */
};
typedef enum lcf_str8OutputType str8OutputType;
enum lcf_str8FormatFlags { /* ZII for default values TODO(lcf): rename these. */
    MANUAL_NEWLINE = 1, /* Disable newline after every call */
};
typedef enum lcf_str8FormatFlag str8FormatFlags;
struct lcf_str8PrintContext {
    /* Format info */
    u32 flags; 
    i32 tabs; /* TODO(lcf): what else could go here? */
    /* Internal buffer */
    u32 buf_len; 
    u32 buf_pos;
    char8* buf;
    /* Output */
    str8OutputType output_type;
    union out {
        FILE* file;
        str8* str;
    } out;
};
/* TODO(lcf): redo naming scheme to allow for shorter name than this. */
typedef struct lcf_str8PrintContext Prn8;

/* TODO(lcf): immediate-mode style string formating.
    * "print" primitives from lcf_types.h to string.
    * print strings into other strings.
    * print to output streams (stdout, stderr, etc)
    * print string repeated n times
    * add_tab, end_tab (all strings afterward will be tabbed)
    * ^ for above maybe need print context struct?
    * print context could also determine if we are printing to a buffer, or output stream.

    printf kinda sucks, why would we want an "interpreted-mode" api for something
    as basic as printing strings? stop embedding weird micro langs into subsystems.
 */
Prn8 Prn8_stdout(u32 buf_len, char8* buf);

void Prn8_str8(Prn8* ctx, str8 s);
#define Prn8_lit(ctx, lit) Prn8_str8(ctx, str8_lit(lit))

void Prn8_newline(Prn8* ctx);

void Prn8_begin_same_line(Prn8 *ctx);
void Prn8_end_same_line(Prn8 *ctx);

void Prn8_end(Prn8* ctx);
void Prn8_add_tabs(Prn8* ctx, i32 tabs);
void Prn8_del_tabs(Prn8* ctx, i32 tabs);
#define Prn8_begin_tab(ctx) Prn8_add_tabs(ctx, 1);
#define Prn8_end_tab(ctx) Prn8_del_tabs(ctx, 1);

/** ******************************** **/


/** Unicode                          **/
// TODO(lcf)
/** ******************************** **/
#endif
