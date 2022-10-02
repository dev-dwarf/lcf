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

Prn8 Prn8_create(u32 flags) {
    Prn8 p = {0};
    p.arena = Arena_create_default();
    p.flags = flags;
    return p;
}

Prn8 Prn8_create_size(u32 flags, u64 size) {
    Prn8 p = {0};
    p.arena = Arena_create(size);
    p.flags = flags;
    return p;
}

Prn8 Prn8_set_output_file(Prn8 p, FILE* file) {
    p.flags |= OUTPUT_FILE;
    p.file = file;
    return p;
}

#define Prn8_MAKE_SPACE_FOR_TABS_AND_AUTO_NEWLINE()                     \
    b32 auto_newline = (ctx->flags & MANUAL_NEWLINE) == 0;              \
    i32 tabs = ctx->tabs;                                               \
                                                                        \
    if (auto_newline) {                                                 \
        len_to_write += 1; /* Need extra char to write newline */       \
        len_to_write += tabs; /* Always need space for tabs when doing newlines too. */ \
    }                                                                   \
                                                                        \
    if (!auto_newline && (tabs > 0)) {                                  \
        len_to_write += tabs;                                           \
        ctx->tabs = -tabs; /* Negative tabs means already written for this line */ \
    }

#define Prn8_INSERT_TABS()                      \
    if (tabs > 0) {                             \
        memset(head+pos, '\t', (u32) tabs); \
        pos += tabs;                            \
    }                                           

#define Prn8_INSERT_AUTO_NEWLINE()              \
    if (!(ctx->flags & MANUAL_NEWLINE)) {       \
        *(head+pos) = '\n';                     \
        pos += 1;                               \
    }

internal chr8* Prn8_get_space_for_str8(Prn8* ctx, u64 len) {
    void* head = NULL;

    ASSERT(ctx->arena != NULL && "Prn8 Arena is null! Must init with a valid Arena");
    
    head = Arena_take_custom(&ctx->arena, len, 1);
     /* TODO handle null
        if output mode is file try to write out to file and reset the buffer.
     */

    return (chr8*) head;
}

/* Format strings */
void Prn8_str8(Prn8* ctx, str8 s) {
    /* TODO: handle s.len > u32_MAX */
    ASSERT(s.len <= u32_MAX);
    
    /* Check if buffer has enough space */
    u64 len_to_write = s.len;
    
    Prn8_MAKE_SPACE_FOR_TABS_AND_AUTO_NEWLINE();

    /* Allocate needed space from ctx, 1 byte aligned. */
    chr8 *head = Prn8_get_space_for_str8(ctx, len_to_write);
    if (head == NULL) {
        return; 
    }
    
    /* Assume ctx valid */
    u32 pos = 0;

    Prn8_INSERT_TABS();
    
    memcpy(head + pos, s.str, s.len);
    pos += (u32) s.len;
    
    Prn8_INSERT_AUTO_NEWLINE();

    ASSERT(len_to_write == pos);
}

void Prn8_newline(Prn8* ctx) {
    chr8 *head = Prn8_get_space_for_str8(ctx, 1);
    if (head == NULL) {
        return; 
    }
    *head = '\n';
    ctx->tabs = abs(ctx->tabs);
}

/* Format primitive types */
/* TODO(lcf): floating point (oh christ). */
union integer64 {
    i64 i;
    u64 u;
};
internal inline void Prn8_integer_custom(Prn8 *ctx, union integer64 in, u16 width, u16 size, b16 is_signed) {
    static chr8 signsym[] = "+-";
    static chr8 decimal[] = "0123456789";
    static chr8 hex[] = "0123456789abcdefxp";
    static chr8 hexu[] = "0123456789ABCDEFXP";
    static u64 base[] = {10, 16, 8, 64};
    static u64 base_bits_per_char[] = {3, 4, 3, 6};

    /* log8(i64_MAX) < 32, so plenty of space for bases 8, 10 and 16. */
    #define Prn8_integer_custom_INTERNAL_BUF_SIZE 32
    chr8 internal_buf[Prn8_integer_custom_INTERNAL_BUF_SIZE];

    ASSERT(width < 28);
    
    u32 flags = ctx->flags;
    
    /* Count digits needed */
    b32 hex_lowercase = (flags & HEX_LOWERCASE) > 0;
    b32 base_8 = (flags & BASE_8) > 0;
    b32 base_16 = ((flags & BASE_16) > 0);
    u64 b = base[base_16+2*base_8];
    u64 b_digits = base_bits_per_char[base_16+2*base_8];

    u64 u;
    b32 neg = 0;
    if (is_signed) {
        if (b == 10) {
            neg = in.i < 0;
            u = (u64)(neg? -in.i : in.i);
        } else {
            u = in.u;
        }
    } else {
        u = in.u;
    }
    
    u64 needed_digits = 0;
    u32 internal_buf_pos = Prn8_integer_custom_INTERNAL_BUF_SIZE;
    for (;;) {
        internal_buf_pos--;

        if (u <= 0 || needed_digits > size/(b_digits)) {
            break;
        }

        needed_digits += 1;
        if (b == 16) {
            internal_buf[internal_buf_pos] = hex_lowercase? hex[u % 16] : hexu[u % 16];
        } else {
            internal_buf[internal_buf_pos] = decimal[u % b];
        }
        u /= b;
    }

    b32 align_zeros = (flags & RIGHT_ALIGN_WITH_ZEROS) > 0;
    if (align_zeros) {
        for (; width > needed_digits; width--) {
            internal_buf[internal_buf_pos--] = '0';
        }
    }

    /* Use minimum passed in width value */
    u64 len_to_write = CLAMPBOTTOM(needed_digits, width);

    /* Make space for sign bit */
    if (is_signed) {
        b32 sign_always = (flags & SIGN_ALWAYS) > 0;
        if (b == 10 && (sign_always || neg)) {
            len_to_write += 1; /* char for sign bit */
            internal_buf[internal_buf_pos--] = signsym[neg];
        }
    }

    /* Octal and Hex prefixes */
    if (!((flags & DISABLE_HEX_PREFIX) > 0)) {
        if (b == 16) {
            len_to_write += 2;
            internal_buf[internal_buf_pos--] = 'x';
            internal_buf[internal_buf_pos--] = '0';
        }
        if (b == 8) {
            len_to_write += 1;
            internal_buf[internal_buf_pos--] = 'o';
        }
    }

    Prn8_MAKE_SPACE_FOR_TABS_AND_AUTO_NEWLINE();

    chr8 *head = Prn8_get_space_for_str8(ctx, len_to_write);
    if (head == NULL) {
        return; 
    }

    /* Assume we have enough space */
    u32 pos = 0;

    Prn8_INSERT_TABS();
    
    b32 left_align = (flags & LEFT_ALIGN) > 0;
    /* Write to ctx buffer with padding*/
    if (!align_zeros && !left_align) {
        for (; width > needed_digits; width--) {
            head[pos++] = ' ';
        }
    }

    internal_buf_pos++;
    for (; internal_buf_pos < Prn8_integer_custom_INTERNAL_BUF_SIZE; ) {
        head[pos] = internal_buf[internal_buf_pos];
        internal_buf_pos++;
        pos++;
    }

    if (!align_zeros && left_align) {
        for (; width > needed_digits; width--) {
            head[pos++] = ' ';
        }
    }

    Prn8_INSERT_AUTO_NEWLINE();
    
    ASSERT(len_to_write == pos);
}

#define DEFINE_Prn8_SIGNED(bits)                                    \
    void Prn8_i##bits##_custom(Prn8 *ctx, i##bits i, u16 width) {   \
        union integer64 in;                                         \
        in.i = i;                                                   \
        Prn8_integer_custom(ctx, in, width, bits, true);            \
    }                                                               \
    void Prn8_i##bits(Prn8 *ctx, i##bits i) {                       \
        union integer64 in;                                         \
        in.i = i;                                                   \
        Prn8_integer_custom(ctx, in, 1, bits, true);                \
    }

DEFINE_Prn8_SIGNED(8);
DEFINE_Prn8_SIGNED(16);
DEFINE_Prn8_SIGNED(32);
DEFINE_Prn8_SIGNED(64);

#undef DEFINE_Prn8_SIGNED

#define DEFINE_Prn8_UNSIGNED(bits)                                  \
    void Prn8_u##bits##_custom(Prn8 *ctx, u##bits u, u16 width) {   \
        union integer64 in;                                         \
        in.u = u;                                                   \
        Prn8_integer_custom(ctx, in, width, bits, false);           \
    }                                                               \
    void Prn8_u##bits(Prn8 *ctx, u##bits u) {                       \
        union integer64 in;                                         \
        in.u = u;                                                   \
        Prn8_integer_custom(ctx, in, 1, bits, false);               \
    }

DEFINE_Prn8_UNSIGNED(8);
DEFINE_Prn8_UNSIGNED(16);
DEFINE_Prn8_UNSIGNED(32);
DEFINE_Prn8_UNSIGNED(64);

#undef DEFINE_Prn8_UNSIGNED

/* Immediate-Mode formatting regions */
void Prn8_begin_same_line(Prn8 *ctx) {
    ctx->flags |= MANUAL_NEWLINE;
}
void Prn8_end_same_line(Prn8 *ctx) {
    ctx->flags &= ~MANUAL_NEWLINE;
    Prn8_newline(ctx);
}

void Prn8_add_tabs(Prn8* ctx, i32 tabs) {
    ctx->tabs += tabs;
}

void Prn8_del_tabs(Prn8* ctx, i32 tabs) {
    ctx->tabs = CLAMPBOTTOM(ctx->tabs-tabs, 0);
}

internal void Prn8_write_buffer_file(Prn8* ctx) {
    void *raw;
    u32 len;
    raw = ctx->arena.memory;
    len = ctx->arena.pos;
    
    fprintf(ctx->file, "%.*s", len, (chr8*) raw);
}

void Prn8_end(Prn8* ctx) {
    Prn8_write_buffer_file(ctx);
}

#undef RET_STR8
/** ******************************** **/
