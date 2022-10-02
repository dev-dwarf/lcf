
#include "lcf_string.h"
#include "lcf_memory.cpp"
#include "lcf_string.cpp"


int main(int argn, char** argv) {
    // Print to stdout
    u32 buf_len = 1024;
    chr8 buf[1024];
    Prn8 ctx = Prn8_create_stdout(buf_len, buf);

    Prn8_lit(&ctx, "Hello World!");

    Prn8_end(&ctx);

    /* TODO(lcf): lets make macros for some of these things to make things feel more
       convenient. */
    ctx = Prn8_create_stdout(buf_len, buf);
    ctx.flags |= SIGN_ALWAYS | RIGHT_ALIGN_WITH_ZEROS | HEX;
    Prn8_lit(&ctx, "Testing Indentation...");
    Prn8_begin_tab(&ctx);
    Prn8_lit(&ctx, "Tabbed region!");
    Prn8_add_tabs(&ctx, 3);
    Prn8_lit(&ctx, "Multiple tabs at once!");
    Prn8_begin_same_line(&ctx);
    Prn8_lit(&ctx, "Same ");
    Prn8_lit(&ctx, "Line ");
    Prn8_i64_custom(&ctx, -128, 2); Prn8_lit(&ctx, " ");
    Prn8_i64_custom(&ctx, -16, 2); Prn8_lit(&ctx, " ");
    Prn8_i64_custom(&ctx, -1, 2);Prn8_lit(&ctx, " ");
    Prn8_i64(&ctx, 0);Prn8_lit(&ctx, " ");
    Prn8_u64_custom(&ctx, 1, 2);Prn8_lit(&ctx, " ");
    Prn8_u64_custom(&ctx, 16, 2);Prn8_lit(&ctx, " ");
    Prn8_u64_custom(&ctx, u64_MAX, 2);Prn8_lit(&ctx, " ");
    Prn8_u64_custom(&ctx, 128, 2);
    
    Prn8_end_same_line(&ctx);
    Prn8_del_tabs(&ctx, 3);
    Prn8_lit(&ctx, "End of tabbed region.");
    Prn8_i64_custom(&ctx, -128, 2);
    Prn8_i64_custom(&ctx, 1, 2); 
    Prn8_i64(&ctx, 256);
    Prn8_lit(&ctx, "End of indentation test.");

    Prn8_end(&ctx);

    printf("%+2X\n", -128);
    printf("%+2x\n", -128);
    printf("%+I64x\n", u64_MAX);
}