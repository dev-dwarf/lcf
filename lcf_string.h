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
typedef char chr8;
struct lcf_str8 { /* TODO(lcf): spread operator macro?? */
    u64 len;
    chr8 *str;
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
#define str8_lit(s) str8_from((chr8*)(s),(u64)sizeof(s)-1) /* -1 to exclude null character */

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
b32 chr8_is_whitespace(chr8 c);
b32 str8_contains_char(str8 s, chr8 c);
b32 str8_contains_substring(str8 s, str8 sub);
b32 str8_contains_delimiter(str8 s, str8 delims);
#define LCF_STRING_NO_MATCH 0x80000000
u64 str8_char_location(str8 s, chr8 c);
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
    chr8 c = s.str[0];                                      \
    for (; (i < s.len) && ((c = s.str[i]) || true); i++)
#define str8_iter(s) str8_iter_custom(s, i, c);

/* TODO(lcf): macro for backwards iteration over string */

/* Find the first substring split_by, and return everything before it,
   advancing src to after the substring. */
str8 str8_pop_first_split_substring(str8 *src, str8 split_by);
#define str8_iter_splits_custom(s, split_by, iter)                      \
    for (                                                               \
        str8 MACRO_VAR(_str) = (s),                                     \
            MACRO_VAR(_split_by) = (split_by),                          \
            iter = str8_pop_first_split_substrings(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
            ;                                                           \
        (str_is_empty(MACRO_VAR(_str)) != false)                        \
            ;                                                           \
        iter = str8_pop_first_split(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
        )
#define str8_iter_split_substring(s, split_by) str8_iter_split_substring_custom(s, split_by, it)

/* TODO(lcf): split by delimiter, iter split delimiters
   TODO(lcf): split by whitespace, iter split whitespace
*/

/** Printing and Formatting **/
#include <stdio.h>

enum lcf_str8FormatFlags { /* ZII for default values TODO(lcf): rename these. */
    MANUAL_NEWLINE         = 0x1,
    HEX_LOWERCASE          = 0x2,
    DISABLE_HEX_PREFIX     = 0x4,
      DISABLE_OCT_PREFIX   = 0x4,
    SIGN_ALWAYS            = 0x8,
    LEFT_ALIGN             = 0x10,
    RIGHT_ALIGN_WITH_ZEROS = 0x20,
    BASE_16                = 0x40,
      HEX                  = 0x40,
    BASE_8                 = 0x80,
      OCT                  = 0x80,
        BASE_64            = 0xC,
};
typedef enum lcf_str8FormatFlags str8FormatFlags;

/* TODO: WARN: This struct and enum sucks... Need something simpler here.
   want do I actually want to be able to do??

   buffer modes:
   * fixed (Bump), truncates / writes to file immediately and maybe warns when out of space.
   * grow (Arena).

   output types:
   * str8, output a str8 containing the buffer memory and size.
   * FILE, output the string to a file.

   file output mode uses the buffer as an intermediary and then writes to file whenever out of space or as user
   requests it.

   str8 output mode will eventually just return its buffer as the str8 so nothing too special is needed.

   also writing shit to the buffer should be a macro so that we can handle the different modes easily.

 */
enum lcf_str8OutputType { /* TODO(lcf): rename these */
    STR8_FIXED, /* User passes fixed size str8. Truncates once size is reached. */
    STR8_ARENA, /* User passes Arena. buf grows to fit data. output str is {buf_len, buf} */
    STDIO_FILE /* Output to FILE stream. */
};
typedef enum lcf_str8OutputType str8OutputType;
struct lcf_str8PrintContext {
    /* Format info */
    u32 flags; 
    i32 tabs; /* TODO(lcf): what else could go here? */
    /* Internal buffer */
    u32 buf_len; 
    u32 buf_pos;
    chr8* buf;
    /* Output */
    str8OutputType output_type;
    union out {
        FILE* file;
        str8* str;
    } out;

    #ifdef __cplusplus
    void str(str8 s);
    void lit(char* literal);
    void newline();
    void begin_same_line();
    void end_same_line();

    void u64(u64 u);
    
    #endif
};
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

    TODO(lcf): context mode for accumulating size;
    Usecase: allow same immediate-mode calls to be made to discover the required size,
    as you would use to actually perform the formatting, toggled by a flag in context.
 */
/* Create Prn8 Contexts */
Prn8 Prn8_create_stdout(u32 buf_len, chr8* buf);

/* Format strings

   Relevant Prn8 flags:
   * MANUAL_NEWLINE - User must manually write newlines (default is a newline after each _str8 or _lit proc call).
 */
void Prn8_str8(Prn8* ctx, str8 s);
#define Prn8_lit(ctx, lit) Prn8_str8(ctx, str8_lit(lit))
void Prn8_newline(Prn8* ctx);

/* Format primitive types
   Prn8_(i|u)XX procs default to printing a minimum of one character, ie. 0, 1, 10, 100, etc.
   You can specifiy a different minimum number of characters using the _custom procs.
   You can control what characters are printed using the flags.
   
   Relevant Prn8 flags:
   * HEX_LOWERCASE - If formatting as hex use lowercase (default is uppercase) for a-f digits.
   * DISABLE_HEX_PREFIX | DISABLE_OCT_PREFIX - Disable the 0x or 0 prefix for hex or octal numbers respectively.
   * SIGN_ALWAYS - Always print the sign of signed values (default is only for neg. numbers).
   * LEFT_ALIGN - Align left based on width. Ex(width=4): "1    10   100  1000" vs default "   1   10  100 1000".
   * RIGHT_ALIGN_WITH_ZEROS - Use zeros instead of spaces to right-align. Ex(width = 4): "0001 0010 0100 1000".
   * HEX | BASE_16 - Use base 16 / hex to print. Prints signed numbers as their raw two's complement representation.
   * OCT | BASE_8 - Use base 8 / octal to print. Prints signed numbers as their raw two's complement representation.
   * BASE_64 - Use base 64 to print. WARN TODO(lcf): base 64 not supported yet.
 */

void Prn8_i64(Prn8 *ctx, i64 s);
void Prn8_i32(Prn8 *ctx, i32 s);
void Prn8_i16(Prn8 *ctx, i16 s);
void Prn8_i8(Prn8 *ctx, i8 s);
void Prn8_u64(Prn8 *ctx, u64 u);
void Prn8_u32(Prn8 *ctx, u32 u);
void Prn8_u16(Prn8 *ctx, u16 u);
void Prn8_u8(Prn8 *ctx, u8 u);
void Prn8_i64_custom(Prn8 *ctx, i64 s, u16 width);
void Prn8_i32_custom(Prn8 *ctx, i32 s, u16 width);
void Prn8_i16_custom(Prn8 *ctx, i16 s, u16 width);
void Prn8_i8_custom(Prn8 *ctx, i8 s, u16 width);
void Prn8_u64_custom(Prn8 *ctx, u64 u, u16 width);
void Prn8_u32_custom(Prn8 *ctx, u32 u, u16 width);
void Prn8_u16_custom(Prn8 *ctx, u16 u, u16 width);
void Prn8_u8_custom(Prn8 *ctx, u8 u, u16 width);
/* TODO: print arrays of the above ^ */

/* TODO: f32, f64 */
/* NOTE: Study options available in printf to understand what capabilities we need for floats */

/* Immediate-Mode formatting regions */
void Prn8_begin_same_line(Prn8 *ctx);
void Prn8_end_same_line(Prn8 *ctx);
void Prn8_end(Prn8* ctx);
void Prn8_add_tabs(Prn8* ctx, i32 tabs);
void Prn8_del_tabs(Prn8* ctx, i32 tabs);
#define Prn8_begin_tab(ctx) Prn8_add_tabs(ctx, 1);
#define Prn8_end_tab(ctx) Prn8_del_tabs(ctx, 1);

/** Scanning / Parsing **/
/* TODO(lcf) */

/** ******************************** **/


/** Unicode                          **/
// TODO(lcf)
/** ******************************** **/
#endif
