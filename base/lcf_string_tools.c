#include "lcf_string_tools.h"

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
        MemorySet(head+pos, '\t', (u32) tabs); \
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
    
    head = Arena_take_custom(ctx->arena, len, 1);
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
    
    MemoryCopy(head + pos, s.str, s.len);
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
    raw = ctx->arena->memory;
    len = ctx->arena->pos;
    
    fprintf(ctx->file, "%.*s", len, (chr8*) raw);
}

void Prn8_end(Prn8* ctx) {
    Prn8_write_buffer_file(ctx);
}
