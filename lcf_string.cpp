#include "lcf_string.h"
#include <string.h> /* only for memset, memcpy */

/** ASCII                            **/
#define RET_STR8(s,l)             \
    str8 _str8;                   \
    _str8.str = (s);              \
    _str8.len = (l);              \
    return _str8

/* Create strs */
str8 str8_from(chr8* s, u64 len) {
    RET_STR8(s, len);
}

str8 str8_from_pointer_range(chr8 *p1, chr8 *p2) {
    RET_STR8(p1, (u64)(p2 - p1));
}
str8 str8_from_cstring(chr8 *cstr) {
    chr8* p2 = cstr;
    for (; *p2 != 0; p2++)
        ;
    return str8_from_pointer_range(cstr, p2);
}

str8 str8_empty(void) {
    str8 s = { 0 };
    return s;
}

/* Basic/fast operations */
str8 str8_first(str8 s, u64 len) {
    u64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str, len_clamped);
}

str8 str8_last(str8 s, u64 len) {
    u64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str + s.len - len_clamped, len_clamped);
}

str8 str8_cut_first(str8 s, u64 len) {
    u64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str + len_clamped, s.len - len_clamped);
}

str8 str8_cut_last(str8 s, u64 len) {
    u64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str, s.len - len_clamped);
}

/* NOTE(lcf): equivalent to str8_cut_first, but feels different */
str8 str8_skip(str8 s, u64 len) {
    u64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str + len_clamped, s.len - len_clamped);
}

str8 str8_substr_between(str8 s, u64 start, u64 end) {
    u64 end_clamped = CLAMPTOP(end, s.len);
    RET_STR8(s.str + start, end_clamped-start);
}

str8 str8_substr(str8 s, u64 start, u64 n) {
    u64 len_clamped = CLAMPTOP(s.len-start, n);
    RET_STR8(s.str + start, len_clamped);
}

/* Operations that need memory */
str8 str8_copy(Arena *a, str8 s) {
    return str8_copy_custom(Arena_take(a, s.len), s);
}

str8 str8_copy_custom(void* memory, str8 s) {
    str8 copy;
    copy.len = s.len;
    copy.str = (chr8*) memory;
    memcpy(memory, s.str, s.len);
    return copy;
}

str8 str8_concat(Arena *a, str8 s1, str8 s2) {
    str8 concat;
    concat.len = s1.len + s2.len;
    concat.str = (chr8*) Arena_take(a, concat.len);
    memcpy(concat.str, s1.str, s1.len);
    memcpy(concat.str+s1.len, s2.str, s2.len);
    return concat;
}

str8 str8_concat_custom(void *memory, str8 s1, str8 s2) {
    str8 concat;
    concat.len = s1.len+s2.len;
    concat.str = (chr8*) memory;
    memcpy(concat.str, s1.str, s1.len);
    memcpy(concat.str+s1.len, s2.str, s2.len);
    return concat;
}

/* Comparisons / Predicates */
b32 str8_eq(str8 a, str8 b) {
    return (a.len == b.len) &&
        ((str8_is_empty(a))
         || (memcmp(a.str, b.str, a.len) == 0));
}

b32 str8_has_prefix(str8 s, str8 prefix) {
    return (prefix.len < s.len) &&
        (str8_not_empty(prefix)) &&
        (memcmp(s.str, prefix.str, prefix.len) == 0);
}

b32 str8_has_postfix(str8 s, str8 postfix) {
    return (postfix.len < s.len) &&
        (str8_not_empty(postfix)) && 
        (memcmp(s.str+(s.len-postfix.len), postfix.str, postfix.len) == 0);
}

b32 chr8_is_whitespace(chr8 c) {
    switch (c) {
    case ' ':
    case '\n':
    case '\t':
    case '\r':
        return true;
    default:
        return false;
    }
}

b32 str8_contains_char(str8 s, chr8 find) {
    return str8_char_location(s,find) != LCF_STRING_NO_MATCH;
}
u64 str8_char_location(str8 s, chr8 find) {
    str8_iter(s) {
        if (c == find) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}
u64 str8_first_whitespace_location(str8 s) {
    if (str8_is_empty(s)) {
        return LCF_STRING_NO_MATCH;
    }
    str8_iter(s) {
        if (chr8_is_whitespace(c)) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}

b32 str8_contains_substring(str8 s, str8 sub) {
    return str8_substring_location(s,sub) != LCF_STRING_NO_MATCH;
}
u64 str8_substring_location(str8 s, str8 sub) {
    u32 match = 0;
    if (str8_is_empty(s) || str8_is_empty(sub)) {
        return LCF_STRING_NO_MATCH;
    }
    str8_iter(s) {
        if (c == sub.str[match]) {
            match++;
            if (match == sub.len) {
                return i+1-match;
            }
        } else {
            match = 0;
        }
    }
    return LCF_STRING_NO_MATCH;
}

/* NOTE(lcf): similar to substring functions, but delims is used as a list of chars to
   look for instead of matching the entire substring. */
b32 str8_contains_delimiter(str8 s, str8 delims) {
    return str8_delimiter_location(s, delims) != LCF_STRING_NO_MATCH;
}
u64 str8_delimiter_location(str8 s, str8 delims) {
    if (str8_is_empty(s)) {
        return LCF_STRING_NO_MATCH;
    }
    if (str8_is_empty(delims)) {
        return 0;
    }
    str8_iter(s) {
        str8_iter_custom(delims, j, delim) {
            if (c == delim) {
                return i;
            }
        }
    }
    return LCF_STRING_NO_MATCH;
}

/* Conditional Operations */
str8 str8_trim_prefix(str8 s, str8 prefix) {
    if (str8_has_prefix(s, prefix)) {
        s.str = s.str + prefix.len;
        s.len = s.len - prefix.len;
    }
    return s;
}

str8 str8_trim_postfix(str8 s, str8 postfix) {
    if (str8_has_postfix(s, postfix)) {
        s.len = s.len - postfix.len;
    }
    return s;
}


str8 str8_trim_whitespace(str8 s) {
    s = str8_trim_whitespace_front(s);
    return str8_trim_whitespace_back(s);
}

str8 str8_trim_whitespace_front(str8 s) {
    /* trim from start */
    while ((s.len > 0) && chr8_is_whitespace(s.str[0])) {
        s.str++;
        s.len--;
    }
    return s;
}

str8 str8_trim_whitespace_back(str8 s) {
    /* trim from end */
    while ((s.len > 0) && chr8_is_whitespace(s.str[s.len-1])) {
        s.len--;
    }

    /* trim null character(s) from end */
    while ((s.len > 0) && s.str[s.len-1]) {
        s.len--;
    }
    
    return s;
}

str8 str8_pop_at_first_substring(str8 *src, str8 split_by) {
    str8 s = *src;
    u64 match = str8_substring_location(s, split_by);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        u64 delta = match + split_by.len;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        s.len = match;
    }
    return s;
}

str8 str8_pop_at_first_delimiter(str8 *src, str8 delims) {
    str8 s = *src;
    u64 match = str8_delimiter_location(s, delims);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        u64 delta = match + 1;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        s.len = match;
    }
    return s;
}

str8 str8_pop_at_first_whitespace(str8 *src) {
    str8 s = *src;
    u64 match = str8_first_whitespace_location(s);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        u64 delta = match + 1;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        *src = str8_trim_whitespace_front(*src);
        s.len = match;
    }
    return s;
}

#undef RET_STR8
/** ******************************** **/
