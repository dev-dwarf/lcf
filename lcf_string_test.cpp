
#include "lcf_string.h"
#include "lcf_memory.cpp"
#include "lcf_string.cpp"


int main(int argn, char** argv) {

    // Print to stdout
    u32 buf_len = 1024;
    char8 buf[1024];
    Prn8 ctx = Prn8_stdout(buf_len, buf);

    Prn8_lit(&ctx, "Hello World!");

    Prn8_end(&ctx);
    
    ctx = Prn8_stdout(buf_len, buf);
    Prn8_lit(&ctx, "Testing Indentation...");
    Prn8_begin_tab(&ctx);
    Prn8_lit(&ctx, "Tabbed region!");
    Prn8_add_tabs(&ctx, 3);
    Prn8_lit(&ctx, "Multiple tabs at once!");
    Prn8_begin_same_line(&ctx);
    Prn8_lit(&ctx, "These ");
    Prn8_lit(&ctx, "Separated ");
    Prn8_lit(&ctx, "Words ");
    Prn8_lit(&ctx, "Are ");
    Prn8_lit(&ctx, "On ");
    Prn8_lit(&ctx, "Same ");
    Prn8_lit(&ctx, "Line ");
    Prn8_end_same_line(&ctx);
    Prn8_del_tabs(&ctx, 3);
    Prn8_lit(&ctx, "End of tabbed region.");
    Prn8_end_tab(&ctx);
    Prn8_lit(&ctx, "End of indentation test.");

    Prn8_end(&ctx);
}
