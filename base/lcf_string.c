#include "lcf_string.h"
#include <string.h> /* only for memset, memcpy */
/** ASCII                            **/
#define RET_STR8(s,l)                           \
    str8 _s = ZERO_STRUCT;                      \
    _s.str = s;                                 \
    _s.len = l;                                 \
    return _s;

/* Create strs */
str8 str8_from(chr8* s, s64 len) {
    RET_STR8(s, len);
}

str8 str8_from_pointer_range(chr8 *p1, chr8 *p2) {
    RET_STR8(p1, (s64)(p2 - p1));
}

str8 str8_from_cstring(chr8 *cstr) {
    chr8* p2 = cstr;
    while(*p2 != 0)
        p2++;
    return str8_from_pointer_range(cstr, p2);
}

str8 str8_empty(void) {
    str8 s = ZERO_STRUCT;
    return s;
}

/* Basic/fast operations */
str8 str8_first(str8 s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str, len_clamped);
}

str8 str8_last(str8 s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str + s.len - len_clamped, len_clamped);
}

str8 str8_cut(str8 s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str, s.len - len_clamped);
}

str8 str8_skip(str8 s, s64 len) {
    s64 len_clamped = CLAMPTOP(len, s.len);
    RET_STR8(s.str + len_clamped, s.len - len_clamped);
}

str8 str8_substr_between(str8 s, s64 start, s64 end) {
    s64 end_clamped = CLAMPTOP(end, s.len);
    RET_STR8(s.str + start, end_clamped-start);
}

str8 str8_substr(str8 s, s64 start, s64 n) {
    s64 len_clamped = CLAMPTOP(s.len-start, n);
    RET_STR8(s.str + start, len_clamped);
}

/* Operations that need memory */
str8 str8_create_size(Arena *a, s64 len) {
    str8 s;
    s.len = len;
    s.str = (chr8*) Arena_take_zero(a, s.len);
    return s;
}

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

str8 str8_copy_first_n(Arena *a, str8 s, s64 n) {
    s = str8_first(s, n);
    return str8_copy(a, s);
}

str8 str8_copy_first_n_custom(void* memory, str8 s, s64 n) {
    s = str8_first(s, n);
    return str8_copy_custom((chr8*) memory, s);
}

str8 str8_copy_cstring(Arena *a, chr8 *c) {
    str8 cstr = str8_from_cstring(c);
    return str8_copy(a, cstr);
}

str8 str8_from_cstring_custom(str8 dest, chr8 *c) {
    str8 out = ZERO_STRUCT;
    out.str = dest.str;
    while (out.len < dest.len && *c != '\0') {
        out.len++;
        *dest.str++ = *c++;
    }
    out.len++;
    *dest.str = *c;
    return out;
}

str8 str8_concat(Arena *a, str8 s1, str8 s2) {
    str8 concat;
    concat.len = s1.len + s2.len;
    concat.str = (chr8*) Arena_take(a, concat.len);
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
    return (prefix.len <= s.len) &&
        (str8_not_empty(s)) &&
        (memcmp(s.str, prefix.str, prefix.len) == 0);
}

b32 str8_has_suffix(str8 s, str8 suffix) {
    return (suffix.len <= s.len) &&
        (str8_not_empty(s)) && 
        (memcmp(s.str+(s.len-suffix.len), suffix.str, suffix.len) == 0);
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
s64 str8_char_location(str8 s, chr8 find) {
    str8_iter(s) {
        if (c == find) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}
s64 str8_first_whitespace_location(str8 s) {
    if (str8_is_empty(s)) {
        return LCF_STRING_NO_MATCH;
    }
    str8_iter(s) {
        if (chr8_is_whitespace(c)) {
            return i;
        }
    }
    return LCF_STRING_NO_MATCH;
}

b32 str8_contains_substring(str8 s, str8 sub) {
    return str8_substring_location(s,sub) != LCF_STRING_NO_MATCH;
}
s64 str8_substring_location(str8 s, str8 sub) {
    u32 match = 0;
    if (str8_is_empty(s) || str8_is_empty(sub)) {
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
s64 str8_delimiter_location(str8 s, str8 delims) {
    if (str8_is_empty(s)) {
        return LCF_STRING_NO_MATCH;
    }
    if (str8_is_empty(delims)) {
        return 0;
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

str8 str8_trim_suffix(str8 s, str8 suffix) {
    if (str8_has_suffix(s, suffix)) {
        s.len -= suffix.len;
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
    s64 match = str8_substring_location(s, split_by);
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

str8 str8_pop_at_first_delimiter(str8 *src, str8 delims) {
    str8 s = *src;
    s64 match = str8_delimiter_location(s, delims);
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

str8 str8_pop_at_first_whitespace(str8 *src) {
    str8 s = *src;
    s64 match = str8_first_whitespace_location(s);
    if (match == LCF_STRING_NO_MATCH) {
        src->str = 0;
        src->len = 0;
    } else {
        s64 delta = match + 1;
        src->str = s.str + delta;
        src->len = (s.len > delta)? s.len - delta : 0;
        *src = str8_trim_whitespace_front(*src);
        s.len = match;
    }
    return s;
}

#undef RET_STR8
/** Str8 Lists                       **/

/* List manipulation */
Str8Node* Str8Node_from(Arena *arena, str8 str) {
    Str8Node *n = Arena_take_array(arena, Str8Node, 1);
    n->str = str;
    n->next = 0;
    return n;
}

void Str8List_add_node(Str8List *list, Str8Node *n) {
    if (list->count == 0) {
        list->first = n;
    } else {
        list->last->next = n;
    }
    list->last = n;
    list->count++;
    list->total_len += n->str.len;
}

void Str8List_prepend_node(Str8List *list, Str8Node *n) {
    if (list->count == 0) {
        list->last = n;
    } else {
        n->next = list->first;
    }
    list->first = n;
    list->count++;
    list->total_len += n->str.len;
}

void Str8List_add(Arena *arena, Str8List *list, str8 str) {
    Str8List_add_node(list, Str8Node_from(arena, str));
}

Str8Node* Str8List_pop_node(Str8List *list) {
    Str8Node *out = 0;
    if (list->count == 1) {
        out = list->first;
        *list = ZERO_STRUCT;
    } else if (list->count != 0) {
        Str8Node *new_last = list->first->next;
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

Str8List Str8List_pop(Str8List *list, s64 n) {
    Str8List out = ZERO_STRUCT;
    for (s64 i = 0; i < n; i++) {
        Str8Node *pop = Str8List_pop_node(list);
        Str8List_add_node(&out, pop);
        if (pop == 0) {
            break;
        }
    }
    return out;
}

void Str8List_prepend(Str8List *list, Str8List nodes) {
    if (nodes.count != 0) {
        /* If the list is empty, replace it with nodes */
        if (list->count == 0) {
            *list = nodes;
        } else {
            ASSERTM(nodes.last->next == 0, "nodes.last should be the end of the Str8List.");
            nodes.last->next = list->first;
            list->first = nodes.first;
            list->count += nodes.count;
            list->total_len += nodes.total_len;
        }
    }
}

void Str8List_append(Str8List *list, Str8List nodes) {
    if (nodes.count != 0) {
        /* If the list is empty, replace it with nodes */
        if (list->count == 0) {
            *list = nodes;
        } else {
            ASSERTM(nodes.last->next == 0, "nodes.last should be the end of the Str8List.");
            list->last->next = nodes.first;
            list->last = nodes.last;
            list->count += nodes.count;
            list->total_len += nodes.total_len;
        }
    }
}

void Str8List_insert(Str8List *list, Str8Node *prev, Str8List nodes) {
    if (nodes.count != 0) {
        if (list->count == 0) {
            *list = nodes;
        } else if (prev != 0) {
            ASSERTM(nodes.last->next == 0, "nodes.last should be the end of the Str8List.");
            nodes.last->next = prev->next;
            prev->next = nodes.first;
            list->count += nodes.count;
            list->total_len += nodes.total_len;
        }
    }
}

Str8Node* Str8List_skip_node(Str8List *list) {
    Str8Node* out = 0;

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

Str8List Str8List_skip(Str8List *list, s64 n) {
    Str8List out = ZERO_STRUCT;
    for (u32 i = 0; (list->count > 0) && (i < n); i++) {
        Str8Node *f = list->first;
        list->count--;
        list->total_len -= f->str.len;
        list->first = list->first->next;
        Str8List_add_node(&out, f);
    }
    return out;
}

str8 Str8List_join(Arena *arena, Str8List list, Str8ListJoin join) {
    /* Calculate size */
    str8 result = ZERO_STRUCT;
    result.len = join.prefix.len +
        list.total_len + join.seperator.len*((list.count > 1)? list.count - 1: 0) +
        join.suffix.len;
    result.str = Arena_take_array(arena, chr8, result.len);

    /* Fill result */
    chr8 *ptr = result.str;

    MemoryCopy(ptr, join.prefix.str, join.prefix.len);
    ptr += join.prefix.len;

    Str8Node *node = list.first;
    for (s64 i = 0; i < list.count; i++, node = node->next) {
        MemoryCopy(ptr, node->str.str, node->str.len);
        ptr += node->str.len;
        if (node != list.last) {
            MemoryCopy(ptr, join.seperator.str, join.seperator.len);
            ptr += join.seperator.len;
        }
    }
    
    MemoryCopy(ptr, join.suffix.str, join.suffix.len);
    ptr += join.suffix.len;

    return result;
}

/* Makes copies of nodes, but not of their strings */
Str8List Str8List_copy(Arena *arena, Str8List list) {
    Str8List copy = ZERO_STRUCT;
    Str8Node *n = list.first;
    for (s64 i = 0; i < list.count; i++, n = n->next) {
        Str8Node *copyn = Arena_take_struct_zero(arena, Str8Node);
        copyn->str = n->str;
        Str8List_add_node(&copy, copyn);
    }
    return copy;
}
    
#ifdef __cplusplus
/* Create str8s */
str8::str8(): str(0), len(0){}
str8::str8(chr8 *p1, chr8 *p2) {*this = str8_from_pointer_range(p1, p2); }
str8::str8(chr8 *p) {*this = str8_from_cstring(p); }
str8::str8(Arena *a, s64 l) {*this = str8_create_size(a, l); }
/* Basic/fast operations */
str8 str8::first(s64 l) {return str8_first(*this, l); }
str8 str8::skip(s64 l) {return str8_skip(*this, l); }
str8 str8::cut(s64 l) {return str8_cut(*this, l); }
str8 str8::last(s64 l) {return str8_last(*this, l); }
str8 str8::substr_between(s64 start, s64 end) {return str8_substr_between(*this, start, end); }
str8 str8::substr(s64 start, s64 n) {return str8_substr(*this, start, n); }
/* Operations that need memory */
str8 str8::copy(Arena *a) {return str8_copy(a, *this); }
str8 str8::copy(void* memory) {return str8_copy_custom(memory, *this); }
str8 str8::concat(Arena *a, str8 s2) {return str8_concat(a, *this, s2);}
/* Comparisons / Predicates */
b32 str8::is_empty() {return str8_is_empty(*this); }
b32 str8::not_empty() {return str8_not_empty(*this); }
b32 str8::eq(str8 b) {return str8_eq(*this, b); }
b32 str8::operator==(const str8& r) { return str8_eq(*this, r); }
b32 str8::has_prefix(str8 prefix) {return str8_has_prefix(*this, prefix); }
b32 str8::has_suffix(str8 suffix) {return str8_has_suffix(*this, suffix); }
b32 str8::contains_char(chr8 c) {return str8_contains_char(*this, c); }
b32 str8::contains_substring(str8 sub) {return str8_contains_substring(*this, sub); }
b32 str8::contains_delimiter(str8 delims) {return str8_contains_delimiter(*this, delims); }
s64 str8::char_location(chr8 c) {return str8_char_location(*this, c); }
s64 str8::substring_location(str8 sub) {return str8_substring_location(*this, sub); }
s64 str8::delimiter_location(str8 delims) {return str8_delimiter_location(*this, delims); }
/* Additional Operations */
str8 str8::trim_prefix(str8 prefix) {return str8_trim_prefix(*this, prefix); }
str8 str8::trim_suffix(str8 suffix) {return str8_trim_suffix(*this, suffix); }
str8 str8::trim_whitespace() {return str8_trim_whitespace(*this); }
str8 str8::trim_whitespace_front() {return str8_trim_whitespace_front(*this); }
str8 str8::trim_whitespace_back() {return str8_trim_whitespace_back(*this); }

Str8Node::Str8Node(str8 s) {next = 0; str = s;}
void Str8List::add_node(Str8Node *n) {return Str8List_add_node(this, n);}
void Str8List::add(Arena* arena, str8 str) {return Str8List_add(arena, this, str); }
void Str8List::prepend(Str8List nodes) {return Str8List_prepend(this, nodes); }
void Str8List::append(Str8List nodes) {return Str8List_append(this, nodes); }
Str8Node* Str8List::pop_node() {return Str8List_pop_node(this); }
Str8List Str8List::pop(s64 n) {return Str8List_pop(this, n); }
void Str8List::insert(Str8Node *prev, Str8List nodes) {return Str8List_insert(this, prev, nodes); }
Str8Node* Str8List::skip_node() {return Str8List_skip_node(this); }
Str8List Str8List::skip(s64 n) {return Str8List_skip(this, n); }
str8 Str8List::join(Arena *arena, Str8ListJoin join) {return Str8List_join(arena, *this, join); }
Str8List Str8List::copy(Arena *arena) {return Str8List_copy(arena, *this); }
#endif
/** ******************************** **/

/* TODO: formatting unsigned, signed, and floats */
