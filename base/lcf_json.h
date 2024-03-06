#if !defined(LCF_JSON)
#define LCF_STRING "1.0.0"

#ifndef LCF_JSON_DEPTH
#define LCF_JSON_DEPTH 64
#endif

enum JSON_TYPES {
    JSON_UNDEFINED = 0,
    JSON_OBJECT,
    JSON_ARRAY,
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
    s32 parent;
    s32 line;
    s32 n;
    str str;
};
typedef struct json_token json_token;

struct json {
    str input; 
    json_token *token;
    s32 c; // cursor
    s32 tokens;
    s32 tokens_cap;
    s32 line;
    
    // parent stack
    s32 parent[LCF_JSON_DEPTH];
    s32 p;
};
typedef struct json json;

static s32 _json_next_tok(json *j) {
    json_token *t = j->token + j->tokens;
    if (j->p >= 0) {
        j->token[j->parent[j->p]].n++;
        t->parent = j->parent[j->p];
    }
    s32 r = j->tokens++;
    return r;
}

s32 json_parse(json *j) {
    str s = str_skip(j->input, j->c);

    if (j->tokens == 0) {
        j->tokens++;
    }
    
    while (s.len > 0 && j->tokens < j->tokens_cap) {
        ASSERT(j->p < LCF_JSON_DEPTH);
    
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
                if (j->parent[j->p] != newp) {
                    j->parent[++j->p] = newp;
                }
                s = str_skip(s, 1);
            } break;
            case '}':
            case ']': {
                json_token *par;
                do {
                    par = j->token + j->parent[j->p--];
                } while (par->type != JSON_OBJECT && par->type != JSON_ARRAY);
                par->str.len = s.str+1 - par->str.str;
                s = str_skip(s, 1);
            } break;

            case ':': {
                if (j->parent[j->p] != j->tokens) {
                    j->parent[++j->p] = j->tokens;
                }
                s = str_skip(s, 1);
            } break;

            case ',': {
                if (j->p >= 0
                    && j->token[j->parent[j->p]].type != JSON_ARRAY
                    && j->token[j->parent[j->p]].type != JSON_OBJECT) {
                    json_token *par = j->token + j->parent[j->p];
                    par->str.len = s.str - par->str.str;
                    j->p--;
                }
                s = str_skip(s, 1);
            } break;

            /* Strings */
            case '\"':
            case '\'': {
                t->type = JSON_STRING;
                t->n = 0;

                s = str_skip(s, 1);
                s32 loc = str_char_location(s, c);
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

            /* Bool */
            case 't': {
                t->type = JSON_BOOL;
                t->n = 1;
                t->str = str_first(s, 4);
                t->line = j->line;
                s = str_skip(s, 4);
                _json_next_tok(j);
                continue;
            } break;
            case 'f': {
                t->type = JSON_BOOL;
                t->n = 0;
                t->str = str_first(s, 5);
                t->line = j->line;
                s = str_skip(s, 5);
                _json_next_tok(j);
            } break;

            /* Null */
            case 'n': {
                t->type = JSON_NULL;
                t->n = 0;
                t->str = str_first(s, 4);
                t->line = j->line;
                s = str_skip(s, 4);
                _json_next_tok(j);
                continue;
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
                NOTIMPLEMENTED();
            } break;
        }
    }
    j->c = s.str - j->input.str;
    return 0;
}

#endif
