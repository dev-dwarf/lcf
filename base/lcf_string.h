/** ************************************
    LCF, Created (September 04, 2022)

    Purpose:
    Length based string library.
    Lots of procedures for trimming, searching, and iterating immutable strings.
    Lists of strings.
    ASCII (str8)
    UNICODE (str32)
  
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

#define str8_PRINTF_ARGS(s) (int)(s).len, (s).str

/* Create str8s */
str8 str8_from(chr8* s, u64 len);
str8 str8_from_pointer_range(chr8 *p1, chr8 *p2);
str8 str8_from_cstring(chr8 *cstr);
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
#define str8_iter_custom(s, i, c)                           \
    u64 i = 0;                                              \
    chr8 c = s.str[i];                                      \
    for (; (i < s.len); i++, c = s.str[i])

#define str8_iter(s) str8_iter_custom(s, i, c)

#define str8_iter_backward_custom(s, i, c)                  \
    u64 i = s.len-1;                                        \
    chr8 c = s.str[i];                                      \
    for (; (i >= 0); i--, c = s.str[i])

#define str8_iter_backward(s) str8_iter_backward_custom(s, i, c)

/* The idea of these procedures is to search the string for a search_str(substring|delimiter|whitespace),
   then return the substring before the search_str, as well as advancing src to be past the search_str.
   
   Then the macros can be used to lazily iterate over these returned substrings.

   NOTE: for pop_at_first_whitespace src* will be advanced to the next non-whitespace character
       after the first found whitespace. If this is not what you want use pop_at_first_delimiter
        with a string of whitespace as the delims.
*/

/* WARN(lcf): These modify the src struct (not the data though). */
str8 str8_pop_at_first_substring(str8 *src, str8 split_by);
str8 str8_pop_at_first_delimiter(str8 *src, str8 delims);
str8 str8_pop_at_first_whitespace(str8 *src);

#define str8_iter_pop_substring_custom(s, split_by, iter)               \
    for (                                                               \
        str8 MACRO_VAR(_str) = (s),                                     \
            MACRO_VAR(_split_by) = (split_by),                          \
            iter = str8_pop_at_first_substring(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
            ;                                                           \
        (!str8_is_empty(iter) || !str8_is_empty(MACRO_VAR(_str)))       \
            ;                                                           \
        iter = str8_pop_at_first_substring(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
        )
#define str8_iter_pop_substring(s, split_by) str8_iter_pop_substring_custom(s, split_by, sub)

#define str8_iter_pop_delimiter_custom(s, delims, iter)                 \
    for (                                                               \
        str8 MACRO_VAR(_str) = (s),                                     \
            MACRO_VAR(_delims) = (delims),                              \
            iter = str8_pop_at_first_delimiter(&MACRO_VAR(_str),MACRO_VAR(_delims)) \
            ;                                                           \
        (!str8_is_empty(iter) || !str8_is_empty(MACRO_VAR(_str)))      \
            ;                                                           \
        iter = str8_pop_at_first_delimiter(&MACRO_VAR(_str),MACRO_VAR(_delims)) \
        )
#define str8_iter_pop_delimiter(s, delims) str8_iter_pop_delimiter_custom(s, delims, sub)


#define str8_iter_pop_whitespace_custom(s, iter)                    \
    for (                                                           \
        str8 MACRO_VAR(_str) = (s),                                 \
            iter = str8_pop_at_first_whitespace(&MACRO_VAR(_str))   \
            ;                                                       \
        (!str8_is_empty(iter) || !str8_is_empty(MACRO_VAR(_str)))   \
            ;                                                       \
        iter = str8_pop_at_first_whitespace(&MACRO_VAR(_str))       \
        )
#define str8_iter_pop_whitespace(s) str8_iter_pop_whitespace_custom(s, sub)

/** Str8 Lists                       **/

struct lcf_str8Node {
    struct lcf_str8Node *next;
    struct lcf_str8 str;
};
struct lcf_str8List {
    struct lcf_str8Node *first;
    struct lcf_str8Node *last;
    u64 count;
    u64 len;
};
typedef struct lcf_str8Node Str8Node;
typedef struct lcf_str8List Str8List;

/* List manipulation */
void Str8List_add_node(Str8List *list, Str8Node *n);
void Str8List_append(Str8List *list, Str8List nodes);
void Str8List_add(Arena *arena, Str8List *list, str8 str);
str8 Str8List_join(Arena *arena, Str8List list, str8 prefix, str8 seperator, str8 postfix);

/** Unicode                          **/
/* TODO(lcf) */

/** ******************************** **/
#endif
