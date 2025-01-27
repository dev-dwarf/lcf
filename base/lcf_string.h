#if !defined(LCF_STRING)
#define LCF_STRING 1

#include <stdarg.h>
#include "lcf_types.h"
#include "lcf_memory.h"
#include "../libs/stb_sprintf.h"

/** ASCII                            **/
struct str { 
    s64 len;
    char *str;
};
typedef struct str str;

#define str_PRINTF_ARGS(s) (int)(s).len, (s).str

/* Create strs */
str str_from(char* s, s64 len);
str str_from_pointer_range(char *p1, char *p2);
str str_from_cstring(char *cstr);

#define strl(s) str_from((char*)s, (s64)sizeof(s)-1)
#define strc(s) {sizeof(s)-1, (char*)s}
global str str_EMPTY = {0, 0};

str strf(Arena *a, char *fmt, ...);

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
#define str_is_empty(s) ((s32)((s).len == 0))
#define str_not_empty(s) ((s32)((s).len != 0))
s32 str_eq(str a, str b);
s32 str_has_prefix(str s, str prefix);
s32 str_has_suffix(str s, str suffix);
s32 char_is_whitespace(char c);
s32 char_is_alpha(char c);
s32 char_is_num(char c);
s32 char_is_alphanum(char c);
s32 str_contains_char(str s, char c);
s32 str_contains_substring(str s, str sub);
s32 str_contains_delimiter(str s, str delims);
#define LCF_STRING_NO_MATCH (-1)
s64 str_char_location(str s, char c);
s64 str_char_location_backward(str s, char find); 
s64 str_substring_location(str s, str sub);
s64 str_delimiter_location(str s, str delims); 
s64 str_first_whitespace_location(str s);
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

/* Parsing */
u64 str_to_u64(str s, s32 *failure);
s64 str_to_s64(str s, s32 *failure);
f64 str_to_f64(str s, s32 *failure);

/* Iterations */
#define str_iter(s, i, c)                           \
    s64 i = 0;                                              \
    char c = s.str? s.str[i] : 0;                                      \
    for (; (i < (s64) s.len); i++, c = s.str[i])

#define str_iter_backward(s, i, c)                  \
    s64 i = s.len-1;                                        \
    char c = s.str[i];                                      \
    for (; (i >= 0); i--, c = s.str[i])

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

#define str_iter_substring(s, split_by, iter)               \
    for (                                                               \
        str MACRO_VAR(_str) = (s),                                     \
            MACRO_VAR(_split_by) = (split_by),                          \
            iter = str_pop_at_first_substring(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
            ;                                                           \
        (!str_is_empty(iter) || !str_is_empty(MACRO_VAR(_str)))       \
            ;                                                           \
        iter = str_pop_at_first_substring(&MACRO_VAR(_str),MACRO_VAR(_split_by)) \
        )

#define str_iter_delimiter(s, delims, iter)            \
    for (                                                               \
        str MACRO_VAR(_str) = (s),                                          \
            MACRO_VAR(_delims) = (delims),                              \
            iter = str_pop_at_first_delimiter(&MACRO_VAR(_str),MACRO_VAR(_delims)) \
            ;                                                           \
        (!str_is_empty(iter) || !str_is_empty(MACRO_VAR(_str)))                      \
            ;                                                           \
        iter = str_pop_at_first_delimiter(&MACRO_VAR(_str),MACRO_VAR(_delims)) \
        )
        
global str str_NEWLINE = {1, "\n"};
#define str_iter_line(s, l) str_iter_delimiter(s, str_NEWLINE, l)

#define str_iter_whitespace(s, iter)                    \
    for (                                                           \
        str MACRO_VAR(_str) = (s),                                 \
            iter = str_pop_at_first_whitespace(&MACRO_VAR(_str))   \
            ;                                                       \
        (!str_is_empty(iter) || !str_is_empty(MACRO_VAR(_str)))   \
            ;                                                       \
        iter = str_pop_at_first_whitespace(&MACRO_VAR(_str))       \
        )

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

/* Rendering */
struct StrJoin {
    str prefix;
    str seperator;
    str suffix;
};
typedef struct StrJoin StrJoin;
str StrList_join(Arena *a, StrList list, StrJoin join);
StrList StrList_copy(Arena *a, StrList list);

/** Unicode                          **/
/* TODO(lcf) */

#endif
