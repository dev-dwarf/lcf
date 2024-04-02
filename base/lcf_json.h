#if !defined(LCF_JSON)
#define LCF_JSON 1

#ifndef LCF_JSON_DEPTH
#define LCF_JSON_DEPTH 64
#endif

/* TODO add convenience json functions for querying for keys and parsing numbers */

enum JSON_TYPES {
    JSON_UNDEFINED = 0,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_KEY,
    JSON_STRING,
    JSON_NULL,
    JSON_BOOL,
    JSON_NUM,
};

enum JSON_NUM_FLAGS {
    JSON_NUM_DEFAULT = 0,
    JSON_NUM_FLOAT = 1 << 1,
    JSON_NUM_HEX = 1 << 2,
};

struct json_token {
    enum JSON_TYPES type;
    u32 parent;
    u32 line;
    u32 n;
    str str;
};
typedef struct json_token json_token;

struct json {
    Arena *arena;
    str input; 
    
    json_token *token;
    s64 c; // cursor
    s32 tokens;
    s32 line;
    s16 err;

    // parent stack
    s32 p;
    s32 parent[LCF_JSON_DEPTH];
};
typedef struct json json;

static s32 _json_next_tok(json *j) {
    json_token *t = j->token + j->tokens;
    t->parent = j->parent[j->p];
    if (t->type == JSON_KEY) {
        t->n = str_hash(t->str, 0);
    } 
    if (j->token[t->parent].type != JSON_KEY) {
        j->token[t->parent].n++;
    }
    s32 r = j->tokens++;
    Arena_take_struct_zero(j->arena, json_token);
    
    return r;
}

s32 json_parse(json *j) {
    str s = str_skip(j->input, j->c);

    if (j->tokens == 0) {
        j->tokens++;
        j->token = Arena_take_struct_zero(j->arena, json_token);
        Arena_take_struct_zero(j->arena, json_token);
    }
    
    while (s.len > 0) {
        ASSERT(j->p < LCF_JSON_DEPTH);

        if (j->err) {
            break;
        }
    
        char c = *s.str; 
        json_token *t = j->token + j->tokens;
        switch (c) {
            case '{':
            case '[': {
                t->type = (c =='{')? JSON_OBJECT : JSON_ARRAY;
                t->str = s;
                t->line = j->line;
                t->n = 0;
                s32 newp = _json_next_tok(j);
                j->parent[++j->p] = newp;
                s = str_skip(s, 1);
            } break;
            case '}': {
                if (j->p > 0) {
                    json_token *par = j->token + j->parent[j->p];
                    if (par->type != JSON_OBJECT) {
                        j->err = 1;
                    }
                    par->str.len =  s.str+1 - par->str.str;

                    j->p--;
                }
                s = str_skip(s, 1);
            } break;
            case ']': {
                if (j->p > 0) {
                    json_token *par = j->token + j->parent[j->p];
                    if (par->type != JSON_ARRAY) {
                        j->err = 1;
                    }
                    par->str.len = s.str+1 - par->str.str;
                    
                    j->p--;
                }
                s = str_skip(s, 1);
            } break;

            case ':': {
                j->parent[++j->p] = j->tokens-1;
                s = str_skip(s, 1);
            } break;

            case ',': {
                if (j->p >= 0) {
                    if (j->token[j->parent[j->p]].type == JSON_OBJECT) {
                        j->err = 1;
                    }
                    if (j->token[j->parent[j->p]].type != JSON_ARRAY) {
                        j->p--;
                    }
                }
                s = str_skip(s, 1);
            } break;

            /* Strings */
            case '\"':
            case '\'': {
                t->type = (j->token[j->parent[j->p]].type == JSON_KEY)? JSON_STRING : JSON_KEY;
                t->n = 0;

                s = str_skip(s, 1);
                s64 loc = str_char_location(s, c);
                loc = (loc != LCF_STRING_NO_MATCH)? loc : s.len;
                t->str = str_first(s, loc);
                s = str_skip(s, loc+1);
                _json_next_tok(j);
            } break;

            //* Primitives *//
            /* Numbers */
            case '-':
            case '+':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                s32 f = 0;
                s32 i = 0;
                if (c =='+' || c =='-') {
                    i += 1;
                }
                if (*s.str =='0') {
                    if (s.str[1] =='x') {
                        f |= JSON_NUM_HEX; 
                        i += 2;
                    }
                }
                
                s32 found_exp = 0;
                for (; i < s.len; i++) {
                    char c1 = s.str[i];
                    if (c1 == '0'
                        || c1 == '1'
                        || c1 == '2'
                        || c1 == '3'
                        || c1 == '4'
                        || c1 == '5'
                        || c1 == '6'
                        || c1 == '7'
                        || c1 == '8'
                        || c1 == '9'
                    ) {
                        continue;
                    }

                    if (c1 == '.') {
                        if (!((f & JSON_NUM_FLOAT) > 0)) {
                            f |= JSON_NUM_FLOAT;
                            continue;
                        } else {
                            break;
                        }
                    }

                    if (c1 == '}' || c1 == ']' || c1 == ',') {
                        break;
                    }

                    c1 |= 32; // make case insensitive
                    if (f & JSON_NUM_HEX) {
                        if (c1 == 'a'
                        || c1 == 'b'
                        || c1 == 'c'
                        || c1 == 'd'
                        || c1 == 'e'
                        || c1 == 'f'
                        ) {
                            continue;
                        }
                    }

                    if (f & JSON_NUM_HEX) {
                        if (c1 == 'p') {
                            if (!found_exp) {
                                found_exp = true;
                                f |= JSON_NUM_FLOAT;
                                i++;
                                continue;
                            } else {
                                break;
                            }
                        }
                    } else {
                        if (c1 == 'e') {
                            if (!found_exp) {
                                found_exp = true;
                                f |= JSON_NUM_FLOAT;
                                i++;
                                continue;
                            } else {
                                break;
                            }
                        }
                    }
                    break;
                }
                // handle floats like 1.0f
                if (((f & JSON_NUM_FLOAT) > 0) && !found_exp) {
                    if (s.str[i] == 'f') {
                        i++;
                    }
                }
                t->type = JSON_NUM;
                t->n = f;
                t->str = str_first(s, i);
                s = str_skip(s, i);
                _json_next_tok(j);
            } break;

            /* Whitespace */
            case '\n':
                j->line++;
                // fallthrough
            case '\t':
            case '\r':
            case '\v':
            case ' ': {
                // do nothing
                s = str_skip(s, 1);
            } break;
            default: {
                if (str_has_prefix(s, strl("true"))) {
                    t->type = JSON_BOOL;
                    t->n = 1;
                    t->str = str_first(s, 4);
                    t->line = j->line;
                    s = str_skip(s, 4);
                    _json_next_tok(j);
                } else if (str_has_prefix(s, strl("false"))) {
                    t->type = JSON_BOOL;
                    t->n = 0;
                    t->str = str_first(s, 5);
                    t->line = j->line;
                    s = str_skip(s, 5);
                    _json_next_tok(j);
                } else if (str_has_prefix(s, strl("null"))) {
                    t->type = JSON_NULL;
                    t->n = 0;
                    t->str = str_first(s, 4);
                    t->line = j->line;
                    s = str_skip(s, 4);
                    _json_next_tok(j);
                } else if (char_is_alphanum(c)) {
                    t->n = 0;
                    if (j->token[j->parent[j->p]].type == JSON_KEY) {
                        t->type = JSON_STRING;
                        s64 loc = str_char_location(s, ',');
                        loc = (loc != LCF_STRING_NO_MATCH)? loc : s.len;
                        t->str = str_trim_whitespace_back(str_first(s, loc));
                        s = str_skip(s, loc);
                    } else {
                        t->type = JSON_KEY;
                        s64 loc = str_char_location(s, ':');
                        loc = (loc != LCF_STRING_NO_MATCH)? loc : s.len;
                        t->str = str_trim_whitespace_back(str_first(s, loc));
                        s = str_skip(s, loc);
                    }
                    _json_next_tok(j);
                } else {
                    j->err = 1;
                }
            } break;
        }
    }

    j->parent[0] = 0;
    j->p = 0;
    j->c = s.str - j->input.str;
    return j->err;
}

json_token* json_next(json *j, json_token *root, json_token *prev) {
    s32 r = (root)? (s32)(root - j->token) : 0;
    s32 i = 1 + ((prev)? (s32)(prev - j->token) : r);
    
    for (;;) {
        if (i >= j->tokens) {
            return 0;
        }
        s32 p = j->token[i].parent;
        if (p < r) {
            // reached higher node than parent in tree, no more children
            return 0;
        } else if (p == r) {
            // i is next child
            return j->token + i;
        }
        i++;
    }
}

#define json_iter(j, root, i) json_token *i = json_next(j, root, 0); i; i = json_next(j, root, i)

json_token* json_find_key(json *j, json_token *root, str key) {
    ASSERT(!root || root->type == JSON_OBJECT);
    u32 key_hash = str_hash(key, 0);
    for (json_iter(j, root, c)) {
        if (c->type == JSON_KEY && c->n == key_hash) {
            return c + 1;
        }
    }
    return 0;
}

#endif
