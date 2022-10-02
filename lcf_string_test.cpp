
#include "lcf_string.h"
#include "lcf_memory.cpp"
#include "lcf_string.cpp"

/* TODO:
 * Huge strings
 * Check size of strings
 */

int main(int argn, char** argv) {
    // Print to stdout
    u32 buf_len = 1024;
    chr8 buf[1024];
    Prn8 ctx = Prn8_create(SIGN_ALWAYS | HEX);
    ctx = Prn8_set_output_file(ctx, stdout);
    
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
    Prn8_i64_custom(&ctx, 0, 2);Prn8_lit(&ctx, " ");
    Prn8_i64_custom(&ctx, 1, 2);Prn8_lit(&ctx, " ");
    Prn8_i64_custom(&ctx, 16, 2);Prn8_lit(&ctx, " ");
    Prn8_u64_custom(&ctx, u64_MAX, 2);Prn8_lit(&ctx, " ");
    Prn8_i64_custom(&ctx, 128, 2);
    
    Prn8_end_same_line(&ctx);
    Prn8_del_tabs(&ctx, 3);
    Prn8_lit(&ctx, "End of tabbed region.");
    Prn8_end_tab(&ctx);
    Prn8_lit(&ctx, "End of indentation test.");

    Prn8_end(&ctx);

}
