#if !defined(LCF_STRING)
#define LCF_STRING "1.0.0"

#include <stdarg.h>

#include "lcf_types.h"
#include "lcf_memory.h"

/** ASCII                            **/
typedef char ch8;
struct str { 
    s64 len;
    ch8 *str;
};
typedef struct str str;

#define str_PRINTF_ARGS(s) (int)(s).len, (s).str

/* Create strs */
str str_from(ch8* s, s64 len);
str str_from_pointer_range(ch8 *p1, ch8 *p2);
str str_from_cstring(ch8 *cstr);

#define strl(s) str_from((ch8*)s, (s64)sizeof(s)-1)
#define strc(s) {sizeof(s)-1, (ch8*)s}
global str str_EMPTY = {0, 0};

/* Basic/fast operations */
str str_first(str s, s64 len); /* return first len chars of str, str[0, len) */
str str_skip(str s, s64 len); /* skip over len chars of str, str[len, s.len) */
str str_cut(str s, s64 len); /* cut len chars from str, str[0, s.len-len) */
str str_last(str s, s64 len); /* return last len chars of str, str[s.len-len, s.len) */
str str_substr_between(str s, s64 start, s64 end); /* return str[start, end) as a str */
str str_substr(str s, s64 start, s64 n); /* return str[start, start+n-1] */

/* Operations that need memory */
str str_create_size(Arena *a, s64 len);
str str_copy(Arena *a, str s);
str str_copy_custom(void* memory, str s);
str str_concat(Arena *a, str s1, str s2);
str str_make_cstring(Arena *a, str s);
str str_formatv(Arena *a, char *fmt, va_list args);
str str_format(Arena *a, char *fmt, ...);

/* Comparisons / Predicates */
#define str_is_empty(s) ((b32)((s).len == 0))
#define str_not_empty(s) ((b32)((s).len != 0))
b32 str_eq(str a, str b);
b32 str_has_prefix(str s, str prefix);
b32 str_has_suffix(str s, str suffix);
b32 ch8_is_whitespace(ch8 c);
b32 str_contains_char(str s, ch8 c);
b32 str_contains_substring(str s, str sub);
b32 str_contains_delimiter(str s, str delims);
#define LCF_STRING_NO_MATCH s64_MIN
s64 str_char_location(str s, ch8 c);
s64 str_char_location_backward(str s, ch8 find); 
s64 str_substring_location(str s, str sub);
s64 str_delimiter_location(str s, str delims); 
char char_lower(char c);
char char_upper(char c);

/* Conditional Operations */
str str_trim_prefix(str s, str prefix);
str str_trim_suffix(str s, str suffix);
str str_trim_whitespace(str s);
str str_trim_whitespace_front(str s);
str str_trim_whitespace_back(str s);

/* Paths */
str str_trim_last_slash(str s);
str str_trim_file_type(str s);
str str_get_file_type(str s);

/* Iterations */
#define str_iter_custom(s, i, c)                           \
    s64 i = 0;                                              \
    ch8 c = s.str[i];                                      \
    for (; (i < (s64) s.len); i++, c = s.str[i])

#define str_iter(s) str_iter_custom(s, i, c)

#define str_iter_backward_custom(s, i, c)                  \
    s64 i = s.len-1;                                        \
    ch8 c = s.str[i];                                      \
    for (; (i >= 0); i--, c = s.str[i])

#define str_iter_backward(s) str_iter_backward_custom(s, i, c)

/* The idea of these procedures is to search the string for a search_str(substring|delimiter|whitespace),
   then return the substring before the search_str, as well as advancing src to be past the search_str.
   
   Then the macros can be used to lazily iterate over these returned substrings.

   NOTE: for pop_at_first_whitespace src* will be advanced to the next non-whitespace character
       after the first found whitespace. If this is not what you want use pop_at_first_delimiter
        with a string of whitespace as the delims.
*/

/* WARN(lcf): These modify the src struct (not the data though). */
str str_pop_at_first_substring(str *src, str split_by);
str str_pop_at_first_delimiter(str *src, str delims);
str str_pop_at_first_whitespace(str *src);

#define str_iter_pop_substring_custom(s, split_by, iter)               \
    for (                                                               \
        str MACRO_VAR(_str) = (s),                                     \
            MACRO_VAR(_split_by) = (split_by),                          \
            iter = str_pop_at_first_substring(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
            ;                                                           \
        (!str_is_empty(iter) || !str_is_empty(MACRO_VAR(_str)))       \
            ;                                                           \
        iter = str_pop_at_first_substring(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
        )
#define str_iter_pop_substring(s, split_by) str_iter_pop_substring_custom(s, split_by, sub)

#define str_iter_pop_delimiter_custom(s, delims, iter)            \
    for (                                                               \
        str MACRO_VAR(_str) = (s),                                          \
            MACRO_VAR(_delims) = (delims),                              \
            iter = str_pop_at_first_delimiter(&MACRO_VAR(_str),MACRO_VAR(_delims)) \
            ;                                                           \
        (!str_is_empty(iter) || !str_is_empty(MACRO_VAR(_str)))                      \
            ;                                                           \
        iter = str_pop_at_first_delimiter(&MACRO_VAR(_str),MACRO_VAR(_delims)) \
        )
#define str_iter_pop_delimiter(s, delims) str_iter_pop_delimiter_custom(s, delims, sub)
global str str_NEWLINE = {1, "\n"};
#define str_iter_pop_line(s) str_iter_pop_delimiter_custom(s, str_NEWLINE, line)

#define str_iter_pop_whitespace_custom(s, iter)                    \
    for (                                                           \
        str MACRO_VAR(_str) = (s),                                 \
            iter = str_pop_at_first_whitespace(&MACRO_VAR(_str))   \
            ;                                                       \
        (!str_is_empty(iter) || !str_is_empty(MACRO_VAR(_str)))   \
            ;                                                       \
        iter = str_pop_at_first_whitespace(&MACRO_VAR(_str))       \
        )
#define str_iter_pop_whitespace(s) str_iter_pop_whitespace_custom(s, sub)

/** Str Lists                       **/
struct StrNode {
    struct StrNode *next;
    struct str str;
};
struct StrList {
    struct StrNode *first;
    struct StrNode *last;
    s64 count;
    s64 total_len;
};
typedef struct StrNode StrNode;
typedef struct StrList StrList;

/* List manipulation */
void StrList_push_node(StrList *list, StrNode *n);
void StrList_push_noden(StrList *list, u32 n, StrNode *node[]);
void StrList_push(Arena *a, StrList *list, str str);
void StrList_pushn(Arena *a, StrList *list, u32 n, str str[]);
#define StrList_pushv(a, list, s, ...) do { \
        str _strarray[] = {s, __VA_ARGS__};                        \
        StrList_pushn(a, list, ARRAY_LENGTH(_strarray), _strarray); \
    } while(0);
#define StrList_push_nodev(list,  n, ...) do {    \
        StrNode _narray[] =  {n, __VA_ARGS__};                          \
        StrList_push_noden(list, sizeof(_narray)/sizeof(StrNode), _narray); \
    } while(0);

void StrList_prepend(StrList *list, StrList nodes);
void StrList_append(StrList *list, StrList nodes);
StrNode* StrList_pop_node(StrList *list);
StrList StrList_pop(StrList *list, s64 n);
void StrList_insert(StrList *list, StrNode *prev, StrList nodes);
StrNode* StrList_skip_node(StrList *list);
StrList StrList_skip(StrList *list, s64 n);

/* Split, Search, Replace */
struct StrSearch {
    str str;
    StrNode *node;
    s64 index;
};
typedef struct StrSearch StrSearch;
StrSearch StrList_find_next(StrNode *head, str str);
void StrList_split(Arena *a, StrList *list, StrSearch *pos);
void StrList_split_remove(Arena *a, StrList *list, StrSearch *pos);
StrSearch StrList_replace_next(Arena *a, StrList *list, str find, str replace);
/* NOTE: ^ above should return the node position of what was replaced */

/* Rendering */
struct StrJoin {
    str prefix;
    str seperator;
    str suffix;
};
typedef struct StrJoin StrJoin;
str StrList_join(Arena *a, StrList list, StrJoin join);
StrList StrList_copy(Arena *a, StrList list);
StrList StrList_reverse(Arena *a, StrList list);

/** Unicode                          **/
/* TODO(lcf) */


/** ******************************** **/
#endif
