#include "lcf_string.h"
#include <string.h> /* only for memset, memcpy */

#define STB_SPRINTF_IMPLEMENTATION
#include "../libs/stb_sprintf.h"

/** ASCII                            **/
#define RET_STR(s,l)                           \
    str _s = ZERO_STRUCT;                      \
    _s.str = s;                                 \
    _s.len = l;                                 \
    return _s;

/* Create strs */
str str_from(char* s, s64 len) {
    RET_STR(s, len);
}

str str_from_pointer_range(char *p1, char *p2) {
    RET_STR(p1, (s64)(p2 - p1));
}

str str_from_cstring(char *cstr) {
    char* p2 = cstr;
    while(*p2 != 0)
        p2++;
    return str_from_pointer_range(cstr, p2);
}

str str_empty(void) {
    str s = ZERO_STRUCT;
    return s;
}

/* Basic/fast operations */
str str_first(str s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR(s.str, len_clamped);
}

str str_last(str s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR(s.str + s.len - len_clamped, len_clamped);
}

str str_cut(str s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR(s.str, s.len - len_clamped);
}

str str_skip(str s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR(s.str + len_clamped, s.len - len_clamped);
}

str str_substr_between(str s, s64 start, s64 end) {
    s64 end_clamped = CLAMPTOP(end, s.len);
    RET_STR(s.str + start, end_clamped-start);
}

str str_substr(str s, s64 start, s64 n) {
    s64 len_clamped = CLAMPTOP(s.len-start, n);
    RET_STR(s.str + start, len_clamped);
}

/* Operations that need memory */
str str_create_size(Arena *a, s64 len) {
    str s;
    s.len = len;
    s.str = (char*) Arena_take_zero(a, s.len);
    return s;
}

str str_copy(Arena *a, str s) {
    return str_copy_custom(Arena_take(a, s.len), s);
}

str str_copy_custom(void* memory, str s) {
    str copy;
    copy.len = s.len;
    copy.str = (char*) memory;
    memcpy(memory, s.str, s.len);
    return copy;
}

str str_copy_first_n(Arena *a, str s, s64 n) {
    s = str_first(s, n);
    return str_copy(a, s);
}

str str_copy_first_n_custom(void* memory, str s, s64 n) {
    s = str_first(s, n);
    return str_copy_custom((char*) memory, s);
}
 
str str_copy_cstring(Arena *a, char *c) {
    str cstr = str_from_cstring(c);
    return str_copy(a, cstr);
}

str str_from_cstring_custom(str dest, char *c) {
    str out = ZERO_STRUCT;
    out.str = dest.str;
    while (out.len < dest.len && *c != '\0') {
        out.len++;
        *dest.str++ = *c++;
    }
    out.len++;
    *dest.str = *c;
    return out;
}

str str_concat(Arena *a, str s1, str s2) {
    str concat;
    concat.len = s1.len + s2.len;
    concat.str = (char*) Arena_take(a, concat.len);
    memcpy(concat.str, s1.str, s1.len);
    memcpy(concat.str+s1.len, s2.str, s2.len);
    return concat;
}

str str_make_cstring(Arena *a, str s) {
    str cp;
    cp.len = s.len;
    cp.str = (char*) Arena_take(a, cp.len+1);
    memcpy(cp.str, s.str, s.len);
    cp.str[s.len] = '\0';
    return cp;
}

str strfv(Arena *a, char *fmt, va_list args) {
    str result = ZERO_STRUCT;
    va_list args2;
    va_copy(args2, args);
    result.len = stbsp_vsnprintf(0, 0, fmt, args2);
    result.str = Arena_take_array(a, char, result.len+1);
    stbsp_vsnprintf(result.str, (s32)result.len+1, fmt, args);
    return result;
}

str strf(Arena *a, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    str result = strfv(a, fmt, args);
    va_end(args);
    return result;
}


/* Comparisons / Predicates */
s32 str_eq(str a, str b) {
    return (a.len == b.len) &&
        ((str_is_empty(a))
         || (memcmp(a.str, b.str, a.len) == 0));
}

s32 str_has_prefix(str s, str prefix) {
    return (prefix.len <= s.len) &&
        (str_not_empty(s)) &&
        (memcmp(s.str, prefix.str, prefix.len) == 0);
}

s32 str_has_suffix(str s, str suffix) {
    return (suffix.len <= s.len) &&
        (str_not_empty(s)) && 
        (memcmp(s.str+(s.len-suffix.len), suffix.str, suffix.len) == 0);
}

s32 char_is_whitespace(char c) {
    switch (c) {
    case ' ':
    case '\n':
    case '\t':
    case '\r':
    case '\v':
        return true;
    default:
        return false;
    }
}

s32 char_is_alpha(char c) {
    return ((unsigned)c|32) - 'a' < 26;
}

s32 char_is_num(char c) {
    return ((unsigned)c) - '0' < 10;
}

s32 char_is_alphanum(char c) {
    return (((unsigned)c|32) - 'a' < 26) || (((unsigned)c) - '0' < 10);
}

s32 str_contains_char(str s, char find) {
    return str_char_location(s,find) != LCF_STRING_NO_MATCH;
}
s64 str_char_location(str s, char find) {
    str_iter(s, i, c) {
        if (c == find) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}
s64 str_char_location_backward(str s, char find) {
    str_iter_backward(s, i, c) {
        if (c == find) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}
s64 str_first_whitespace_location(str s) {
    if (str_is_empty(s)) {
        return LCF_STRING_NO_MATCH;
    }
    str_iter(s, i, c) {
        if (char_is_whitespace(c)) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}

s32 str_contains_substring(str s, str sub) {
    return str_substring_location(s,sub) != LCF_STRING_NO_MATCH;
}
s64 str_substring_location(str s, str sub) {
    u32 match = 0;
    if (str_is_empty(s) || str_is_empty(sub)) {
        return LCF_STRING_NO_MATCH;
    }
    str_iter(s, i, c) {
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
s32 str_contains_delimiter(str s, str delims) {
    return str_delimiter_location(s, delims) != LCF_STRING_NO_MATCH;
}
s64 str_delimiter_location(str s, str delims) {
    if (str_is_empty(s)) {
        return LCF_STRING_NO_MATCH;
    }
    if (str_is_empty(delims)) {
        return 0;
    }
    str_iter(s, i, c) {
        str_iter(delims, j, delim) {
            if (c == delim) {
                return i;
            }
        }
    }
    return LCF_STRING_NO_MATCH;
}

static read_only char LCF_CHAR_LOWER = 'a' - 'A';
char char_lower(char c) {
    if (c >= 'A' && c <= 'Z') {
        c += LCF_CHAR_LOWER;
    }
    return c;
}

static read_only char LCF_CHAR_UPPER = 'A' - 'a';
char char_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        c += LCF_CHAR_UPPER;
    }
    return c;
}

/* Conditional Operations */
str str_trim_prefix(str s, str prefix) {
    if (str_has_prefix(s, prefix)) {
        s.str = s.str + prefix.len;
        s.len = s.len - prefix.len;
    }
    return s;
}

str str_trim_suffix(str s, str suffix) {
    if (str_has_suffix(s, suffix)) {
        s.len -= suffix.len;
    }
    return s;
}


str str_trim_whitespace(str s) {
    s = str_trim_whitespace_front(s);
    return str_trim_whitespace_back(s);
}

str str_trim_whitespace_front(str s) {
    /* trim from start */
    while ((s.len > 0) && char_is_whitespace(s.str[0])) {
        s.str++;
        s.len--;
    }
    return s;
}

str str_trim_whitespace_back(str s) {
    /* trim from end */
    while ((s.len > 0) && (char_is_whitespace(s.str[s.len-1]) || s.str[s.len-1] == 0)) {
        s.len--;
    }

    return s;
}

/* Paths */
str str_trim_last_slash(str s) {
    if (s.str[s.len-1] == '\\' || s.str[s.len-1] == '/') {
        s.len -= 1;
    }
    return s;
}
str str_trim_file_type(str s) {
    s64 loc = str_char_location_backward(s, '.');
    if (loc == LCF_STRING_NO_MATCH) {
        s = str_first(s, loc);
    }
    return s;
}
str str_get_file_type(str s) {
    s64 loc = str_char_location_backward(s, '.');
    if (loc == LCF_STRING_NO_MATCH) {
        s = str_skip(s, loc+1);
    }
    return s;
}


str str_pop_at_first_substring(str *src, str split_by) {
    str s = *src;
    s64 match = str_substring_location(s, split_by);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        s64 delta = match + split_by.len;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        s.len = match;
    }
    return s;
}

str str_pop_at_first_delimiter(str *src, str delims) {
    str s = *src;
    s64 match = str_delimiter_location(s, delims);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        s64 delta = match + 1;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        s.len = match;
    }
    return s;
}

str str_pop_at_first_whitespace(str *src) {
    str s = *src;
    s64 match = str_first_whitespace_location(s);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        s64 delta = match + 1;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        *src = str_trim_whitespace_front(*src);
        s.len = match;
    }
    return s;
}


/* Parsing */
u64 str_to_u64(str s, s32 *failure) {
    s32 base = 10;
    if (s.str[0] == '0') {
        if (s.str[1] == 'x') {
            base = 16;
            s.str += 2; s.len -= 2;
        } else if (s.str[1] == 'b') {
            base = 2;
            s.str += 2; s.len -= 2;
        } else {
            base = 8;
            s.str += 1; s.len -= 1;
        }
    }

    s32 i = 0;
    u64 n = 0;
    switch (base) {
        case 2: {
            s.len = MIN(s.len, 64); // 64 = log(2^64)/log(2)
            for (i = 0; i < s.len; i++) {
                u8 digit = s.str[i] - '0';
                if (digit >= 2) {
                    break;
                }
                n = 2*n + digit;
            }
        } break;
        case 8: {
            s.len = MIN(s.len, 21); // 21 = log(2^64)/log(8)
            for (i = 0; i < s.len; i++) {
                u8 digit = s.str[i] - '0';
                if (digit >= 8) {
                    break;
                }
                n = 8*n + digit;
            }
        } break;
        case 10: {
            s.len = MIN(s.len, 19); // 19 = log(2^64)/log(10)
            for (i = 0; i < s.len; i++) {
                u8 digit = s.str[i] - '0';
                if (digit >= 10) {
                    break;
                }
                n = 10*n + digit;
            }
        } break;
        case 16: {
            s.len = MIN(s.len, 16); // 16 = log(2^64)/log(16)
            for (i = 0; i < s.len; i++) {
                u8 digit = s.str[i] - '0';
                if (digit >= 10) digit = (s.str[i]|32) + 10 - 'a';
                if (digit >= 16) {
                    break;
                }
                n = 16*n + digit;
            }
        } break;
    }

    if (i == 0) {
        if (failure) *failure = 1;
        return 0;
    }

    if (failure) *failure = 0;
    return n;
}

s64 str_to_s64(str s, s32 *failure) {
    u64 sign = 0;
    if (s.str[0] == '-' || s.str[0] == '+') {
        sign = s.str[0] == '-'? -1 : 0;
        s.str++; s.len--;
    }

    u64 u = str_to_u64(s, failure);
    if (failure && *failure) {
        return 0;
    } else {
        return (s64)((u^sign)-sign);
    }
}

#ifndef INFINITY
#define INFINITY (1e5000f)
#endif
#ifndef NAN
#define NAN (0.0/0.0)
#endif

f64 str_to_f64(str s, s32 *failure) {
    s32 i;
    str p = str_trim_whitespace(s);

    s32 sign = 1;
    if (*p.str == '+' || *p.str == '-') {
        sign = (*p.str == '-')? -1 : 1;
        p.str += 1; p.len -= 1;
    }

    { // match inf and infinity
        for (i = 0; i < MIN(8, p.len); i++) {
            if ((p.str[i]|32) != "infinity"[i]) {
                break;
            }
        }
        if (i == 3 || i == 8) { 
            if (failure) *failure = 0;
            return sign * INFINITY;
        } else if (i > 0) {
            if (failure) *failure = 1;
            return 0.0;
        }
    }

    { // match nan 
        for (i = 0; i < MIN(3, p.len); i++) {
            if ((p.str[i]|32) != "nan"[i]) {
                break;
            }
        }
        if (i == 3) {
            if (failure) *failure = 0;
            return NAN;
        } else if (i > 0) {
            return 1;
        }
    }

    s32 is_hex = 0;
    if (p.str[0] == '0' && p.str[1] == 'x') { // Hex
        is_hex = 1;
        p.str += 2; p.len -= 2;
    }

    { // skip leading zeros
        for (i = 0; i < p.len; i++) {
            if (p.str[i] != '0') {
                break;
            }
        }
        p.str += i; p.len -= i;
    }

    s32 got_frac = 0;
    s32 exp_offset = 0;
    if (*p.str == '.') { // dot before digits, eg 0.505
        got_frac = 1;
        p.str += 1; p.len -= 1;

        { // handle zeros after dot, eg 0.000000000505
            for (i = 0; i < p.len; i++) {
                if (p.str[i] != '0') {
                    break;
                }
            }
            p.str += i; p.len -= i;
            exp_offset = -i;
        }
    }

    u64 mantissa = 0;
    if (p.len > 0) {
        if (is_hex) {
            s32 len = MIN((s32)p.len, 19); // NOTE(lf): 19 = floor(log10(2^64))
            s32 zeros = 0;

            { // Catch garbage strings
                u8 digit = p.str[i] - '0';
                if (digit >= 10) digit = 10 + (p.str[i]|32)-'a';
                if (digit > 16) {
                    if (failure) *failure = 1;
                    return 0.0;
                }
            }
                
            for (i = 0; i < len; i++) {
                if (p.str[i] == '.') {
                    if (got_frac) {
                        p.len = i;
                        break;
                    }
                    got_frac = 1;
                    
                    while (zeros > 0) {
                        mantissa *= 16;
                        zeros--;
                    }

                    exp_offset = 0; 
                    continue;
                }
            
                u8 digit = p.str[i] - '0';
                if (digit >= 10) digit = 10 + (p.str[i]|32)-'a';
                if (digit < 16) {
                    if (digit > 0) {
                        if (got_frac) {
                            exp_offset -= 1 + zeros;
                        }
                        while (zeros > 0) {
                            mantissa *= 16;
                            zeros--;
                        }
                        mantissa = mantissa*16 + digit; 
                    } else {
                        zeros++;
                    }
                } else {
                    break;
                }
            }
            if (zeros > 0) {
                exp_offset += zeros;
            }
            p.str += i; p.len -= i;
        } else {
            s32 len = MIN((s32)p.len, 19); // NOTE(lf): 19 = floor(log10(2^64))
            s32 zeros = 0;

            { // Catch garbage strings
                u8 digit = *p.str - '0';
                if (digit >= 10) {
                    if (failure) *failure = 1;
                    return 0.0;
                }
            }
                
            for (i = 0; i < len; i++) {
                if (p.str[i] == '.') {
                    if (got_frac) {
                        p.len = i;
                        break;
                    }
                    got_frac = 1;
                    
                    while (zeros > 0) {
                        mantissa *= 10;
                        zeros--;
                    }

                    exp_offset = 0; 
                    continue;
                }
            
                u8 digit = p.str[i] - '0';
                if (digit < 10) {
                    if (digit > 0) {
                        if (got_frac) {
                            exp_offset -= 1 + zeros;
                        }
                        while (zeros > 0) {
                            mantissa *= 10;
                            zeros--;
                        }
                        mantissa = mantissa*10 + digit; 
                    } else {
                        zeros++;
                    }
                } else {
                    break;
                }
            }
            if (zeros > 0) {
                exp_offset += zeros;
            }
            p.str += i; p.len -= i;
        }
    }

    if (mantissa == 0) {
        if (failure) *failure = 0;
        return sign == -1? -0.0 : 0.0;
    }

    f64 f = sign*(f64)(mantissa);

    s32 exp = 0;
    char exp_delim = (is_hex)? 'p' : 'e';
    for (i = 0; i < p.len; i++) {
        if ((p.str[i]|32) == exp_delim) {
            break;
        }
    }
    p.str += i+1; p.len -= i+1;
    
    if (p.len > 0) {
        s32 exp_sign = 1;
        if (*p.str == '+' || *p.str == '-') {
            exp_sign = (*p.str == '-')? -1 : 1;
            p.str += 1; p.len -= 1;
        }

        s32 got_digit = 0;
        for (i = 0; i < p.len; i++) {
            u8 digit = p.str[i] - '0';
            if (digit >= 10) {
                break;
            }
            got_digit = 1;
            exp = 10 * exp + digit;
        }

        if (!got_digit) {
            if (failure) *failure = 1;
            return 0.0;
        } 
        
        exp = (exp * exp_sign);
    }

    if (is_hex) {
        exp += 4*exp_offset;
        while (exp > 0) {
            f *= 2; exp--;
            // Goofy ahh optimization attempt
            if (exp >=+16) { f *= 0x1p+16; exp -= 16; }
            if (exp >=+8) { f *= 0x1p+8; exp -= 8; }
            if (exp >=+4) { f *= 0x1p+4; exp -= 4; }
        }
        while (exp < 0) {
            f *= 0.5; exp++;
            if (exp <=-16) { f *= 0x1p-16; exp += 16; }
            if (exp <=-8) { f *= 0x1p-8; exp += 8; }
            if (exp <=-4) { f *= 0x1p-4; exp += 4; }
        }
    } else {
        exp += exp_offset;
        while (exp > 0) {
            f *= 10.0; exp--;
            if (exp >=+16) { f *= 1e+16; exp -= 16; }
            if (exp >=+8) { f *= 1e+8; exp -= 8; }
            if (exp >=+4) { f *= 1e+4; exp -= 4; }
        }
        while (exp < 0) {
            f *= 0.1; exp++;
            if (exp <=-16) { f *= 1e-16; exp += 16; }
            if (exp <=-8) { f *= 1e-8; exp += 8; }
            if (exp <=-4) { f *= 1e-4; exp += 4; }
        }
    }

    if (failure) *failure = 0;
    return f;
}

#undef RET_STR
/** Str Lists                       **/

/* List manipulation */
StrNode* StrNode_from(Arena *a, str str) {
    StrNode *n = Arena_take_array(a, StrNode, 1);
    n->str = str;
    n->next = 0;
    return n;
}

void StrList_push_node(StrList *list, StrNode *n) {
    PushQ(list, n);
    list->count++;
    list->total_len += n->str.len;
}

void StrList_prepend_node(StrList *list, StrNode *n) {
    PushQFront(list, n);
    list->count++;
    list->total_len += n->str.len;
}

void StrList_push(Arena *a, StrList *list, str str) {
    StrList_push_node(list, StrNode_from(a, str));
}

void StrList_push_noden(StrList *list, u32 n, StrNode *node[]) {
    while (n-- > 0) {
        StrList_push_node(list, *(node++));
    }
}

void StrList_pushn(Arena *a, StrList *list, u32 n, str str[]) {
    while (n-- > 0) {
        StrList_push(a, list, *(str++));
    }
}

StrNode* StrList_pop_node(StrList *list) {
    StrNode *out = 0;
    if (list->count == 1) {
        out = list->first;
        /* NOTE(lcf): Compiler bug? */
        StrList zero = ZERO_STRUCT;
        (*list) = zero;
    } else if (list->count != 0) {
        StrNode *new_last = list->first->next;
        while (new_last->next != list->last) {
            new_last = new_last->next;
        }
        out = list->last;
        new_last->next = 0;
        list->total_len -= list->last->str.len;
        list->count--;
        list->last = new_last;
    }
    return out;
}

StrList StrList_pop(StrList *list, s64 n) {
    StrList out = ZERO_STRUCT;
    for (s64 i = 0; i < n; i++) {
        StrNode *pop = StrList_pop_node(list);
        StrList_push_node(&out, pop);
         if (pop == 0) {
            break;
        }
    }
    return out;
}

void StrList_prepend(StrList *list, StrList nodes) {
    if (nodes.count != 0) {
        /* If the list is empty, replace it with nodes */
        if (list->count == 0) {
            *list = nodes;
        } else {
            ASSERTM(nodes.last->next == 0, "nodes.last should be the end of the StrList.");
            nodes.last->next = list->first;
            list->first = nodes.first;
            list->count += nodes.count;
            list->total_len += nodes.total_len;
        }
    }
}

void StrList_append(StrList *list, StrList nodes) {
    if (nodes.count != 0) {
        /* If the list is empty, replace it with nodes */
        if (list->count == 0) {
            *list = nodes;
        } else {
            ASSERTM(nodes.last->next == 0, "nodes.last should be the end of the StrList.");
            list->last->next = nodes.first;
            list->last = nodes.last;
            list->count += nodes.count;
            list->total_len += nodes.total_len;
        }
    }
}

void StrList_insert(StrList *list, StrNode *prev, StrList nodes) {
    if (nodes.count != 0) {
        if (list->count == 0) {
            *list = nodes;
        } else if (prev != 0) {
            ASSERTM(nodes.last->next == 0, "nodes.last should be the end of the StrList.");
            nodes.last->next = prev->next;
            prev->next = nodes.first;
            list->count += nodes.count;
            list->total_len += nodes.total_len;
        }
    }
}

StrNode* StrList_skip_node(StrList *list) {
    StrNode* out = 0;

    if (list->count == 1) {
        list->last = 0;
    }
    if (list->first) {
        out = list->first;
        list->first = out->next;
        out->next = 0;
        list->count--;
        list->total_len -= out->str.len;
    }

    return out;
}

StrList StrList_skip(StrList *list, s64 n) {
    StrList out = ZERO_STRUCT;
    for (u32 i = 0; (list->count > 0) && (i < n); i++) {
        StrNode *f = list->first;
        list->count--;
        list->total_len -= f->str.len;
        list->first = list->first->next;
        StrList_push_node(&out, f);
    }
    return out;
}

str StrList_join(Arena *a, StrList list, StrJoin join) {
    /* Calculate size */
    str result = ZERO_STRUCT;
    result.len = join.prefix.len +
        list.total_len + join.seperator.len*((list.count > 1)? list.count - 1: 0) +
        join.suffix.len;
    result.str = Arena_take_array(a, char, result.len);

    /* Fill result */
    char *ptr = result.str;

    memcpy(ptr, join.prefix.str, join.prefix.len);
    ptr += join.prefix.len;

    StrNode *node = list.first;
    for (s64 i = 0; i < list.count; i++, node = node->next) {
        memcpy(ptr, node->str.str, node->str.len);
        ptr += node->str.len;
        if (node != list.last) {
            memcpy(ptr, join.seperator.str, join.seperator.len);
            ptr += join.seperator.len;
        }
    }
    
    memcpy(ptr, join.suffix.str, join.suffix.len);
    ptr += join.suffix.len;

    return result;
}

/* Makes copies of nodes, but not of their strings */
StrList StrList_copy(Arena *a, StrList list) {
    StrList copy = ZERO_STRUCT;
    StrNode *n = list.first;
    for (s64 i = 0; i < list.count; i++, n = n->next) {
        StrNode *copyn = Arena_take_struct_zero(a, StrNode);
        copyn->str = n->str;
        StrList_push_node(&copy, copyn);
    }
    return copy;
}


/** ******************************** **/
