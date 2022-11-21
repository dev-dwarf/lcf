/** ************************************
    LCF, Created (October 02, 2022)

    Purpose:
    Provide higher-level functionality ontop of lcf_string.
    String Formatting (Immediate Mode)

    TODO:
    String scanning/parsing
    String interning?
    
    ************************************ **/
#if !defined(LCF_STRING_TOOLS)
#define LCF_STRING_TOOLS "1.0.0"

#include "lcf_string.h"

/** Printing and Formatting **/
#include <stdio.h>

enum lcf_str8FormatFlags { /* ZII for default values TODO(lcf): rename these. */
    MANUAL_NEWLINE         = 0x01,
    HEX_LOWERCASE          = 0x02,
    DISABLE_HEX_PREFIX     = 0x04,
      DISABLE_OCT_PREFIX   = 0x04,
    SIGN_ALWAYS            = 0x08,
    LEFT_ALIGN             = 0x10,
    RIGHT_ALIGN_WITH_ZEROS = 0x20,
    BASE_16                = 0x40,
      HEX                  = 0x40,
    BASE_8                 = 0x80,
      OCT                  = 0x80,
         BASE_64           = 0xC0,

    OUTPUT_FILE            = 0x8000,
};
typedef enum lcf_str8FormatFlags str8FormatFlags;

/* TODO: WARN: This struct and enum sucks... Need something simpler here.
   want do I actually want to be able to do??

   output types:
   * str8, output a str8 containing the buffer memory and size.
   * FILE, output the string to a file.

   file output mode uses the buffer as an intermediary and then writes to file whenever out of space or as user requests it.

   str8 output mode will eventually just return its buffer as the str8 so nothing too special is needed.

   also writing shit to the buffer should be a macro so that we can handle the different modes easily.

*/
struct lcf_str8PrintContext {
    /* Format info */
    u32 flags; 
    s32 tabs; /* TODO(lcf): what else could go here? */
    Arena* arena;
    FILE* file;

#ifdef __cplusplus
    /* TODO: c++ style api for string formatting. */
    void str(str8 s);
    void lit(char* literal);
    void newline();
    void begin_same_line();
    void end_same_line();

    void u64(u64 u);
    
#endif
};
typedef struct lcf_str8PrintContext Prn8;

/* TODO(lcf): immediate-mode style string formating.
 * "print" primitives from lcf_types.h to string.
 * print strings into other strings.
 * print to output streams (stdout, stderr, etc)
 * print string repeated n times
 * add_tab, end_tab (all strings afterward will be tabbed)
 * ^ for above maybe need print context struct?
 * print context could also determine if we are printing to a buffer, or output stream.

 printf kinda sucks, why would we want an "interpreted-mode" api for something
 as basic as printing strings? stop embedding weird micro langs into subsystems.

 TODO(lcf): context mode for accumulating size;
 Usecase: allow same immediate-mode calls to be made to discover the required size,
 as you would use to actually perform the formatting, toggled by a flag in context.
*/
/* Create Prn8 Contexts */
/* TODO */


/* Format strings

   Relevant Prn8 flags:
   * MANUAL_NEWLINE - User must manually write newlines (default is a newline after each _str8 or _lit proc call).
   */
void Prn8_str8(Prn8* ctx, str8 s);
#define Prn8_lit(ctx, lit) Prn8_str8(ctx, str8_lit(lit))
void Prn8_newline(Prn8* ctx);

/* Format primitive types
   Prn8_(i|u)XX procs default to printing a minimum of one character, ie. 0, 1, 10, 100, etc.
   You can specifiy a different minimum number of characters using the _custom procs.
   You can control what characters are printed using the flags.
   
   Relevant Prn8 flags:
   * HEX_LOWERCASE - If formatting as hex use lowercase (default is uppercase) for a-f digits.
   * DISABLE_HEX_PREFIX | DISABLE_OCT_PREFIX - Disable the 0x or 0 prefix for hex or octal numbers respectively.
   * SIGN_ALWAYS - Always print the sign of signed values (default is only for neg. numbers).
   * LEFT_ALIGN - Align left based on width. Ex(width=4): "1    10   100  1000" vs default "   1   10  100 1000".
   * RIGHT_ALIGN_WITH_ZEROS - Use zeros instead of spaces to right-align. Ex(width = 4): "0001 0010 0100 1000".
   * HEX | BASE_16 - Use base 16 / hex to print. Prints signed numbers as their raw two's complement representation.
   * OCT | BASE_8 - Use base 8 / octal to print. Prints signed numbers as their raw two's complement representation.
   * BASE_64 - Use base 64 to print. WARN TODO(lcf): base 64 not supported yet.
   */

void Prn8_s64(Prn8 *ctx, s64 s);
void Prn8_s32(Prn8 *ctx, s32 s);
void Prn8_s16(Prn8 *ctx, s16 s);
void Prn8_s8(Prn8 *ctx, s8 s);
void Prn8_u64(Prn8 *ctx, u64 u);
void Prn8_u32(Prn8 *ctx, u32 u);
void Prn8_u16(Prn8 *ctx, u16 u);
void Prn8_u8(Prn8 *ctx, u8 u);
void Prn8_s64_custom(Prn8 *ctx, s64 s, u16 width);
void Prn8_s32_custom(Prn8 *ctx, s32 s, u16 width);
void Prn8_s16_custom(Prn8 *ctx, s16 s, u16 width);
void Prn8_s8_custom(Prn8 *ctx, s8 s, u16 width);
void Prn8_u64_custom(Prn8 *ctx, u64 u, u16 width);
void Prn8_u32_custom(Prn8 *ctx, u32 u, u16 width);
void Prn8_u16_custom(Prn8 *ctx, u16 u, u16 width);
void Prn8_u8_custom(Prn8 *ctx, u8 u, u16 width);
/* TODO: print arrays of the above ^ */

/* TODO: f16, f32, f64 */
/* NOTE: Study options available in printf to understand what capabilities we need for floats
 * Also IEEE 754 standard for roundtripping when printing.
 */

/* Immediate-Mode formatting regions */
void Prn8_begin_same_line(Prn8 *ctx);
void Prn8_end_same_line(Prn8 *ctx);
void Prn8_end(Prn8* ctx);
void Prn8_add_tabs(Prn8* ctx, s32 tabs);
void Prn8_del_tabs(Prn8* ctx, s32 tabs);
#define Prn8_begin_tab(ctx) Prn8_add_tabs(ctx, 1);
#define Prn8_end_tab(ctx) Prn8_del_tabs(ctx, 1);

/** Scanning / Parsing **/
/* TODO(lcf) */

/** ******************************** **/
#endif
