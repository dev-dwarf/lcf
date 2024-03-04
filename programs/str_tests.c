#include "lcf/lcf.h"
#include "lcf/lcf.c"

#include <stdio.h>

int main() {
    // str_to_f64 tests
    s32 f; str s;
    s = strl("0.01"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("0.001"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("0.00025"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("0.0000000025001"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("0.000000002500000000000001"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));

    s = strl("25e-12"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl(".25e-10"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("2.5E-10"); printf("%.*s -> %G | %G\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl(".7410984687618698162648531893023320585475897039214871466383785237510132609053132e-323"); 
 printf("%.*s -> %a | %a\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));

    s = strl("0x1p-1022"); printf("%.*s -> %A | %A\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("0x1.23456789ABCDEFp-10"); printf("%.*s -> %A | %A\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));

    s = strl("0.0"); printf("%.*s -> %f | %f\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("-0.0"); printf("%.*s -> %f | %f\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("+inf"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("1e+100000"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("-inf"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("1e-100000"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("nan"); printf("%.*s -> %g | %g\n", (s32)s.len, s.str, str_to_f64(s, &f), atof(s.str));
    s = strl("guh"); str_to_f64(s, &f); ASSERT(f != 0);

    // str_to_int64 tests
    s = strl("1234"); printf("%.*s -> %lld | %lld\n", (s32)s.len, s.str, str_to_u64(s, &f), strtoll(s.str, 0, 0));
    s = strl("-0x1234123412341234"); printf("%.*s -> 0x%llX | 0x%llX", (s32)s.len, s.str, str_to_s64(s, &f), strtoll(s.str, 0, 0));

    return 0;
}
