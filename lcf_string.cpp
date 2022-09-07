#include "lcf_string.h"
#include <string.h> /* only for memset, memcpy */

/** ASCII                            **/
#define RET_STR8(s,l)             \
    str8 _str8;                   \
    _str8.str = (s);              \
    _str8.len = (l);              \
    return _str8

/* Create strs */
str8 str8_from(u8* s, u64 len) {
    RET_STR8(s, len);
}

str8 str8_from_pointer_range(u8 *p1, u8 *p2) {
    RET_STR8(p1, (u64)(p2 - p1));
}
str8 str8_from_cstring(u8 *cstr) {
    u8* p2 = cstr;
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
    copy.str = memcpy(memory, s.str, s.len);
    return copy;
}

str8 str8_concat(Arena *a, str8 s1, str8 s2) {
    str8 concat;
    concat.len = s1.len + s2.len;
    concat.str = Arena_take(a, concat.len);
    memcpy(concat.str, s1.str, s1.len);
    memcpy(concat.str+s1.len, s2.str, s2.len);
    return concat;
}

str8 str8_concat_custom(void *memory, str8 s1, str8 s2) {
    str8 concat;
    concat.len = s1.len+s2.len;
    concat.str = memory;
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

b32 char8_is_whitespace(char8 c) {
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

b32 str8_contains_char(str8 s, char8 find) {
    return str8_char_location(s,find) != LCF_STRING_NO_MATCH;
}
u64 str8_char_location(str8 s, char8 find) {
    str8_iter(s) {
        if (c == find) {
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
    if (str8_is_empty(s)) {
        return 0;
    }
    if (str8_is_empty(sub)) {
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
        return 0;
    }
    if (str8_is_empty(delims)) {
        return LCF_STRING_NO_MATCH;
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
    while ((s.len > 0) && char8_is_whitespace(s.str[0])) {
        s.str++;
        s.len--;
    }
    return s;
}

str8 str8_trim_whitespace_back(str8 s) {
    /* trim from end */
    while ((s.len > 0) && char8_is_whitespace(s.str[s.len-1])) {
        s.len--;
    }

    /* trim null character(s) from end */
    while ((s.len > 0) && s.str[s.len-1]) {
        s.len--;
    }
    
    return s;
}

str8 str8_pop_first_split_substring(str8 *src, str8 split_by) {
    str8 s = *src;
    u64 match = str8_substring_location(s, split_by);
    if (match == LCF_STRING_NO_MATCH) {
        s.str = 0;
        s.len = 0;
    } else {
        u64 delta = match + split_by.len;
        src->str = s.str + delta;
        src->len = s.len - delta;
        s.len = match;
    }
    return s;
}

/* Formatting */
/* TODO: remove dependency on sprintf, replace with custom formatting */
#include <stdio.h>

Prn8 Prn8_stdout(u32 buf_len, char8* buf) {
    Prn8 p = { 0 };
    p.buf_len = buf_len;
    p.buf = buf;
    p.output_type = STDIO_FILE;
    p.out.file = stdout;
    memset(buf, 0, buf_len);
    return p;
}

void Prn8_str8(Prn8* ctx, str8 s) {
    /* Check if buffer has enough space */
    i32 tabs = ctx->tabs;
    u32 len_to_write = s.len;
    b32 auto_newline = (ctx->flags & MANUAL_NEWLINE) == 0;

    if (auto_newline) {
        len_to_write += 1; /* Need extra char to write newline */
        len_to_write += tabs; /* Always need space for tabs when doing newlines too. */
    }

    /* Negative tabs means already written for this line */
    if (!auto_newline && (tabs > 0)) { 
        len_to_write += tabs;
        ctx->tabs = -tabs;
    }
    
    if (!(ctx->buf_pos+len_to_write < ctx->buf_len)) {
        // Flush buffer?? 
    }

    /* Assume ctx valid */
    u32 pos = ctx->buf_pos;
    if (tabs > 0) {
        memset(ctx->buf+pos, '\t', tabs);
        pos += tabs;
    }
    memcpy(ctx->buf + pos, s.str, s.len);
    pos += s.len;
    
    if (!(ctx->flags & MANUAL_NEWLINE)) {
        ctx->buf[pos] = '\n';
        pos += 1;
    }

    ASSERT(ctx->buf_pos+len_to_write == pos);
    ctx->buf_pos = pos;
}

void Prn8_newline(Prn8* ctx) {
    /* TODO: check that there is enough space
       Want to avoid duplicating code from Prn8_str8.
     */

    ctx->buf[ctx->buf_pos] = '\n';
    ctx->tabs = abs(ctx->tabs);
    ctx->buf_pos += 1;
}

void Prn8_begin_same_line(Prn8 *ctx) {
    ctx->flags |= MANUAL_NEWLINE;
}
void Prn8_end_same_line(Prn8 *ctx) {
    ctx->flags &= ~MANUAL_NEWLINE;
    Prn8_newline(ctx);
}


void Prn8_add_tabs(Prn8* ctx, u32 tabs) {
    ctx->tabs += tabs;
}

void Prn8_del_tabs(Prn8* ctx, u32 tabs) {
    ctx->tabs = CLAMPBOTTOM(ctx->tabs-tabs, 0);
}

void Prn8_end(Prn8* ctx) {
    Prn8_write_buffer(ctx);
}
void Prn8_write_buffer(Prn8* ctx) {
    printf(ctx->buf);
}


#undef RET_STR8
/** ******************************** **/
