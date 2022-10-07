
#include "../base/lcf_string.h"
#include "../base/lcf_memory.cpp"
#include "../base/lcf_string.cpp"
#include "../base/lcf_string_tools.cpp"

/* TODO:
 * Huge strings
 * Check size of strings
 */

int main(int argn, char** argv) {
    {
        i64 i = 0;
        printf("\nTest pop substring\n");
        str8 s = str8_lit("subway submarine oranges subtraction apples");
        str8 substring = str8_lit("sub");

        str8_iter_pop_substring(s, substring) {
            printf("%lld ", i);
            printf("%.*s\n", str8_PRINTF_ARGS(sub));
            if (i++ > 100) break;
        }

        str8 t1 = s;
        str8 p = str8_pop_at_first_substring(&t1, substring);
        printf("%.*s | %.*s = %d\n", str8_PRINTF_ARGS(p), str8_PRINTF_ARGS(t1), !str8_is_empty(t1));
    }
    
    {
        i64 i = 0;
        printf("\nTest pop delimiter\n");
        str8 s = str8_lit(",test,of,comma,seperated,values;and;semicolons,!");
        str8 del = str8_lit(",;");
        str8_iter_pop_delimiter(s, del) {
            printf("%lld %.*s\n", i, str8_PRINTF_ARGS(sub));
            if (i++ > 100) break;
        }
    }
    
    {
        i64 i = 0;
        printf("\nTest pop whitespace\n");
        str8 s = str8_lit(R"V0G0N(    {
        i64 i = 0;
        printf("\nTest pop delimiter\n");
        str8 s = str8_lit(",test,of,comma,seperated,values;and;semicolons,!");
        str8 del = str8_lit(",;");
        str8_iter_pop_delimiter(s, del) {
            printf("%lld %.*s\n", i, str8_PRINTF_ARGS(sub));
            if (i++ > 100) break;
        }
    })V0G0N");
        str8 del = str8_lit(",;");
        str8_iter_pop_whitespace(s) {
            printf("%lld %.*s\n", i, str8_PRINTF_ARGS(sub));
            if (i++ > 100) break;
        }
    }

    {
        printf("\Test concat");
        str8 s1 = str8_lit("hello ");
        str8 s2 = str8_lit("world!");

        Arena a = Arena_create(1024);
        str8 concat = str8_concat(s1, s2);
        printf("%.*s\n", str8_PRINTF_ARGS(sub));
    }
    
    printf("\n");
    
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
