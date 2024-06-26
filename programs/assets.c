/* TODO
- tiles in scenes
- other types of assets, aseprite, tileset, etc
- scan all assets from folder and load
-- do this for aseprites, hash of file name is asset hash
-- for objects makes more sense to store hash in asset file, for extra stability
-- likewise for scene, dont want scene hashes to break. 
- arbitrary json for objects? for params and such. can be hashed down super small
-- REF: https://bitsquid.blogspot.com/2015/06/allocation-adventures-1-datacomponent.html
 */

#include "lcf/lcf.h"
#include "lcf/lcf.c"

#include "raylib.h"

#include <stdio.h>

#pragma comment(lib, "raylib.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define MAX_ASSET_LIST 64
#define MAX_ASSET_OBJ 1024
#define MAX_ASSET_ASEPRITE 1024
#define MAX_ASSET_TILESET 64
#define MAX_ASSET_SCENE 2048

#define SCENE_OBJ_CHUNK_SIZE 16
#define ASSET_LIST_SIZE 256

enum AssetTypes {
    INFO = 0,
    ASSET_LIST,
    ASEPRITE,
    OBJ,
    TILESET,
    SCENE,

    ASSET_TYPES
};

read_only s32 ASSET_MAX[ASSET_TYPES] = {
    [ASSET_LIST] = MAX_ASSET_LIST,
    [ASEPRITE] = MAX_ASSET_ASEPRITE,
    [OBJ] = MAX_ASSET_OBJ,
    [TILESET] = MAX_ASSET_TILESET,
    [SCENE] = MAX_ASSET_SCENE,
};

typedef struct Asset {
    u32 hash;
    u16 slot;
    u8 type;
    u8 list;
} Asset;

typedef struct AssetInfo {
    str name;
    str file;
} AssetInfo;

typedef struct Inst {
    u32 hash;
    u16 slot;
    u8 gen;
    u8 real;
} Inst;

typedef struct Obj {
    Inst inst;
    
    Vec2 pos;
    Vec2 origin;
    Vec2 size;
    float angle;
    Color color;
    u64 layermask;
    u64 flags;

    s32 deactivated;
    s32 scene_depth;
    Vec2 scale;
    Vec2 pspeed;
    Vec2 speed;
    f32 accel_lerp;
    f32 angle_speed;
    Vec2 shake;
    Vec2 wave;
    f32 wavefreq;
    
    s32 state;
    s32 timer;
    s32 on_ground;
    s32 jump;
    s32 flip;

    Asset obj;
    Asset ase; // Textures Asset
    Asset scene; // if scene obj, contains child scene

    Inst parent;
    Vec2 parent_offset;

    Inst next;
    Inst prev;
    Inst child;
    s32 children;

    u32 editor_hash;
    s32 editor_lister_score;
} Obj;

Rect ObjHitbox(Obj *e) {
    return (Rect) {
        .x = e->pos.x - e->origin.x,
        .y = e->pos.y - e->origin.y,
        .w = e->size.x,
        .w = e->size.y
    };
}

typedef struct SceneObjChunk {
    struct SceneObjChunk *next;
    // Asset scene;
    s32 objs;
    Obj obj[SCENE_OBJ_CHUNK_SIZE];
} SceneObjChunk;

typedef struct Scene {
    Rect obj_bounds;
    // Rect tile_bounds;
        
    s32 objs;
    SceneObjChunk first;
} Scene;

struct _Assets {
    // Chunking for scenes
    SceneObjChunk *free_obj_chunk;

    // NOTE(lcf): 0 is a null obj/scene/whatever
    Asset obj_handle[MAX_ASSET_OBJ]; 
    AssetInfo obj_info[MAX_ASSET_OBJ];
    Obj obj[MAX_ASSET_OBJ]; s32 objs;
    
    Asset scene_handle[MAX_ASSET_SCENE]; 
    AssetInfo scene_info[MAX_ASSET_SCENE];
    Scene scene[MAX_ASSET_SCENE]; s32 scenes;
};
struct _Global {
    Arena *asset_memory;
    struct _Assets assets;

    Arena *world_obj_memory;
    s32 obj_max;
    Obj *obj;
    s32 slot_pos;
    s32 objs;
    Inst free_obj;
    
    Scene *current_scene;
};
struct _Global *G;

Obj* AssetObj(Asset *handle) {
    // TODO(lcf) this is super stupid for now
    // Could be worth changing from arrays to hashmaps as we scale up.
    // Could also not be worth!
    Obj* out = G->assets.obj + handle->slot;

    if (G->assets.obj_handle[handle->slot].hash != handle->hash) {
        out = G->assets.obj;
        
        for (s32 i = 1; i < G->assets.objs; i++) {
            if (G->assets.obj_handle[i].hash == handle->hash) {
                out = G->assets.obj + i;
            }
        }
    }

    *handle = G->assets.obj_handle[(s32)(out - G->assets.obj)];
    return out;
}

AssetInfo* ObjInfo(Obj *o) {
    return G->assets.obj_info + o->obj.slot;
}

Scene* AssetScene(Asset *handle) {
    Scene* out = G->assets.scene + handle->slot;

    if (G->assets.scene_handle[handle->slot].hash != handle->hash) {
        out = G->assets.scene;
        
        for (s32 i = 1; i < G->assets.scenes; i++) {
            if (G->assets.scene_handle[i].hash == handle->hash) {
                out = G->assets.scene + i;
            }
        }
    }

    *handle = G->assets.scene_handle[(s32)(out - G->assets.scene)];
    return out;
}

AssetInfo* SceneInfo(Scene *s) {
    return G->assets.scene_info + (s32)(s - G->assets.scene);
}

Asset* SceneAsset(Scene *s) {
    return G->assets.scene_handle + (s32)(s - G->assets.scene);
}

SceneObjChunk* AllocObjChunk(Arena *a) {
    SceneObjChunk* out = G->assets.free_obj_chunk;
    if (out) {
        G->assets.free_obj_chunk = out->next;
        out->next = 0;
    } else {
        out = Arena_take_zero(a, sizeof(SceneObjChunk));
    }
    return out;
}

s32 CopyWorldToScene(Arena *a, Scene *scene) {
    scene->objs = 0;
    scene->obj_bounds = (Rect){0, 0, 32, 32};

    SceneObjChunk *c = &scene->first; c->objs = 0;

    Vec2 p0 = (Vec2){ f32_HUGE, f32_HUGE};
    Vec2 p1 = (Vec2){-f32_HUGE,-f32_HUGE};
    Obj *o = G->obj;
    Obj *O = G->obj + G->obj_max;
    for (; o < O; o++) {
        if (o->scene_depth == 0) {
            if (c->objs == SCENE_OBJ_CHUNK_SIZE) {
                if (!c->next) {
                    c->next = AllocObjChunk(a);
                }
                c = c->next; c->objs = 0;
                scene->objs += SCENE_OBJ_CHUNK_SIZE;
            }
            memcpy(c->obj + c->objs++, o, sizeof(Obj));

            Rect r = ObjHitbox(o);
            p0.x = MIN(p0.x, r.x); p0.y = MIN(p0.y, r.y);
            p1.x = MAX(p1.x, r.x); p1.y = MAX(p1.y, r.y);
        }
    }

    if (c->next) { // place now unused chunks on free list
        SceneObjChunk *first = c->next;
        SceneObjChunk *last = first; 
        while (last->next) {
            last = last->next;
        }
        if (last) {
            last->next = G->assets.free_obj_chunk;
            G->assets.free_obj_chunk = first;
        }
    }

    scene->objs += c->objs;
    
    if (scene->objs) {
        scene->obj_bounds = RectFromPoints(p0, p1);
    }
    
    return scene->objs;
}

// Finds inst in the current scene
// Uses:
// just hash -> find first instance of obj with hash
// hash + slot -> find next instance of obj with hash
// hash,slot,gen,real -> find exact instance
Obj* InstObj(Inst *inst) {
    if (!inst->hash) {
        return G->obj;
    }
    
    Obj *o = G->obj + inst->slot;
    if (inst->real) {
        if (!memcmp(inst, &o->inst, sizeof(*inst))) {
            o = G->obj;
        } 
    } else {
        o++; // Increment to skip null inst or slot
        Obj *O = G->obj + G->obj_max;
        while (o->inst.hash != inst->hash) {
            if (++o > O) {
                o = G->obj;
                break;
            }
        }
    }
    *inst = o->inst;
    return o;
}

void SetSlotPos() {
    Obj *o = G->obj + G->slot_pos;
    while (o-- > G->obj) {
        if (InstObj(&o->inst) > G->obj) {
            break;
        }
        G->slot_pos--;
    }
}

Inst InstScene(Scene *scene, Obj *parent);
Inst InstObject(Obj o, Obj *parent) {
    Inst inst = (Inst){0};
    if (o.inst.hash) { // Find slot
        if (G->free_obj.slot) {
            inst = G->free_obj;
            G->free_obj = G->obj[inst.slot].next;
        }
        if (inst.slot == 0 || inst.slot >= G->slot_pos) {
            inst = G->obj[G->slot_pos++].inst;
            G->free_obj.slot = 0;
        }
    }

    if (inst.slot) { // Place instance 
        Obj *obj = G->obj + inst.slot;
        *obj = o;
        inst.hash = o.inst.hash;
        inst.real = 1;
        obj->inst = inst;
        G->objs++;

        if (parent) {
            obj->parent = parent->inst;
            obj->parent_offset = SubV2(o.pos, parent->pos);
            parent->children++;

            // list maintenance 
            {
                Obj *c;
                Obj *n = InstObj(&parent->child);
                do {
                    c = n;
                    n = InstObj(&c->next);
                } while (n > G->obj);

                if (c > G->obj) {
                    c->next = inst;
                    obj->prev = c->inst;
                }
            }
        }

        if (obj->scene.hash) {
            obj->inst.hash = -1;
            InstScene(AssetScene(&obj->scene), obj);
        }
    }

    return inst;
}

void DeleteInst(Inst *inst) {
    if (inst->slot > 0) {
        Obj *o = G->obj + inst->slot;
        if (!memcmp(inst, &o->inst, sizeof(*inst))) {
            // Delete children 
            if (o->children) {
                Obj *c = InstObj(&o->child);
                for (; c > G->obj; ) {
                    c->parent = (Inst){0};
                    c->prev = (Inst){0};
                    Obj *next = InstObj(&c->next);
                    DeleteInst(&c->inst);
                    c = next;
                }
            }

            // Maintain lists
            Obj *prev = InstObj(&o->prev);
            Obj *next = InstObj(&o->next);
            if (prev > G->obj && next > G->obj) { 
                prev->next = next->inst;
                next->prev = prev->inst;
            }
            Obj *parent = InstObj(&o->parent);
            if (parent > G->obj) {
                parent->children--;
                if (parent->child.slot == inst->slot) {
                    parent->child = o->next;
                }
            }

            // Delete
            memset(o, 0, sizeof(*o));
            o->inst.slot = inst->slot;

            // Free list
            Obj *free = G->obj + G->free_obj.slot;
            free->next = o->inst;
            G->free_obj = o->inst;

            SetSlotPos();
        }
        
        *inst = (Inst){0};
    }
}

Inst InstScene(Scene *scene, Obj *parent) {
    Inst scene_parent = (Inst){0};

    if (scene) {
        // Root scene
        if (!parent) {
            G->objs = 0;
            G->slot_pos = 2;
            scene_parent = (Inst) {
                .hash = -1,
                .slot = 1,
                .gen = 0,
                .real = 1,
            };
            parent = G->obj + 1;
            parent->inst = scene_parent;
        }

        // grab space for all objs
        // NOTE(lcf) this is not necessarily how many objects will be spawned, as there may be child scenes
        s32 next_slot = G->slot_pos;
        G->slot_pos += scene->objs;
        G->objs += scene->objs;
        parent->children += scene->objs;
    
        Obj *prev = parent;
        s32 spawned_objects = 0;
        for (SceneObjChunk *c = &scene->first; c; c = c->next) {
            for (s32 i = 0; i < c->objs; i++) {
                Obj *obj = G->obj + next_slot;
                Inst inst = obj->inst;
                inst.slot = next_slot;
                inst.real = 1;

                *obj = c->obj[i];
                obj->parent = parent->inst;
                obj->inst = inst;

                if (prev == parent) {
                    parent->child = inst;
                    obj->prev = (Inst){0};
                } else {
                    prev->next = inst;
                    obj->prev = prev->inst;
                }
                prev = obj;

                if (inst.hash == 0xFFFFFFFF) {
                    InstScene(AssetScene(&obj->scene), obj);
                }
            }
        }
    }

    return scene_parent;
}

typedef struct Serdes {
    Arena *temp;
    Arena *perm;
    u64 temp_start;
    
    // Ser
    StrList strl;

    // Des
    json json;
    json_token *parent;

    u16 is_writing;
    s32 indent;

} Serdes;

Serdes* des_start(Arena *perm, Arena *temp, str input) {
    u64 temp_start = temp->pos;
    Serdes *serdes = Arena_take(temp, sizeof(Serdes));
    (*serdes) = (Serdes) {
        .temp = temp,
        .temp_start = temp_start,
        .perm = perm,
        .json = (json) {
            .arena = temp,
            .input = input
        },
    };
    json_parse(&serdes->json);
    return serdes;
}

void des_end(Serdes *serdes) {
    Arena_reset(serdes->temp, serdes->temp_start);
}

Serdes* ser_start(Arena *temp) {
    u64 temp_start = temp->pos;
    Serdes* serdes = Arena_take(temp, sizeof(Serdes));
    *serdes = (Serdes){
        .temp = temp,
        .temp_start = temp_start,
        .is_writing = true,
    };
    return serdes;
}

void ser_end(Serdes *serdes, str file) {
    os_WriteFile(file, serdes->strl);
    Arena_reset(serdes->temp, serdes->temp_start);
}

static void serdes_push_parent(Serdes *serdes, json_token *p) {
    serdes->json.parent[++serdes->json.p] = p - serdes->json.token;
    serdes->parent = p;
}

static void serdes_pop_parent(Serdes *serdes) {
    serdes->parent = serdes->json.parent[--serdes->json.p] + serdes->json.token;
    if (serdes->json.p == 0) {
        serdes->parent = 0;
    }
}

static void serdes_print(Serdes *serdes, str s) {
    str indent[12] = {
        strl(""),
        strl("  "),
        strl("    "),
        strl("      "),
        strl("        "),
        strl("          "),
        strl("            "),
        strl("              "),
        strl("                "),
        strl("                  "),
        strl("                    "),
        strl("                      "),
    };
    StrList_push(serdes->temp, &serdes->strl, indent[serdes->indent]);
    StrList_push(serdes->temp, &serdes->strl, s);
}

static s32 serdes_str(Serdes *serdes, str key, str *v, str def) {
    if (serdes->is_writing) {
        if (!str_eq(*v, def)) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: %.*s,\n", key, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        *v = t? str_copy(serdes->perm, t->str) : def;
    }
    return 0;
}
static s32 serdes_s64(Serdes *serdes, str key, s64 *v, s64 def) {
    if (serdes->is_writing) {
        if (*v != def) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: %lld,\n", key.len, key.str, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        *v = t? str_to_s64(t->str, 0) : def;
    }
    return 0;
}
static s32 serdes_s32(Serdes *serdes, str key, s32 *v, s32 def) {
    if (serdes->is_writing) {
        if (*v != def) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: %d,\n", key.len, key.str, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        *v = t? str_to_s64(t->str, 0) : def;
    }
    return 0;
}
static s32 serdes_u64(Serdes *serdes, str key, u64 *v, u64 def) {
    if (serdes->is_writing) {
        if (*v != def) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: 0x%08llX,\n", key.len, key.str, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        *v = t? str_to_u64(t->str, 0) : def;
    }
    return 0;
}
static s32 serdes_u32(Serdes *serdes, str key, u32 *v, u32 def) {
    if (serdes->is_writing) {
        if (*v != def) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: 0x%08X,\n", key.len, key.str, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        *v = t? str_to_u64(t->str, 0) : def;
    }
    return 0;
}
static s32 serdes_f32(Serdes *serdes, str key, f32 *v, f32 def) {
    if (serdes->is_writing) {
        if (*v != def) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: %g,\n", key.len, key.str, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        *v = t? str_to_f64(t->str, 0) : def;
    }
    return 0;
}
static s32 serdes_color(Serdes *serdes, str key, Color *v, Color def) {
    if (serdes->is_writing) {
        if (memcmp(v, &def, sizeof(*v))) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: 0x%08X,\n", key.len, key.str, *v));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        if (t) {
            u32 raw = str_to_u64(t->str, 0); 
            memcpy(v, &raw, sizeof(Color));
        } else {
            *v = def;
        }
    }
    return 0;
}
static s32 serdes_Vec2(Serdes *serdes, str key, Vec2 *v, Vec2 def) {
    if (serdes->is_writing) {
        if (memcmp(v, &def, sizeof(*v))) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: [%g, %g],\n", key.len, key.str, v->x, v->y));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        if (t) {
            v->x = str_to_f64(t[1].str, 0);
            v->y = str_to_f64(t[2].str, 0);
        } else {
            *v = def;
        }
    }
    return 0;
}
static s32 serdes_Vec3(Serdes *serdes, str key, Vec3 *v, Vec3 def) {
    if (serdes->is_writing) {
        if (memcmp(v, &def, sizeof(*v))) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: [%g, %g, %g],\n", key.len, key.str, v->x, v->y, v->z));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        if (t) {
            v->x = str_to_f64(t[1].str, 0);
            v->y = str_to_f64(t[2].str, 0);
            v->z = str_to_f64(t[3].str, 0);
        } else {
            *v = def;
        }
    }
    return 0;
}
static s32 serdes_Vec4(Serdes *serdes, str key, Vec4 *v, Vec4 def) {
    if (serdes->is_writing) {
        if (memcmp(v, &def, sizeof(*v))) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: [%g, %g, %g, %g],\n", key.len, key.str, v->x, v->y, v->z, v->w));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        if (t) {
            v->x = str_to_f64(t[1].str, 0);
            v->y = str_to_f64(t[2].str, 0);
            v->z = str_to_f64(t[3].str, 0);
            v->w = str_to_f64(t[4].str, 0);
        } else {
            *v = def;
        }
    }
    return 0;
}
static s32 serdes_Rect(Serdes *serdes, str key, Rect *v, Rect def) {
    if (serdes->is_writing) {
        if (memcmp(v, &def, sizeof(*v))) {
            serdes_print(serdes, strf(serdes->temp, "%.*s: [%g, %g, %g, %g],\n", key.len, key.str, v->x, v->y, v->w, v->h));
        }
    } else {
        json_token *t = json_find_key(&serdes->json, serdes->parent, key);
        if (t) {
            v->x = str_to_f64(t[1].str, 0);
            v->y = str_to_f64(t[2].str, 0);
            v->w = str_to_f64(t[3].str, 0);
            v->h = str_to_f64(t[4].str, 0);
        } else {
            *v = def;
        }
    }
    return 0;
}

s32 serdes_obj(Serdes *serdes, Obj *o, Obj *def) {
    serdes_u32(serdes, strl("obj"), &o->obj.hash, 0);
    serdes_u32(serdes, strl("inst"), &o->editor_hash, 0);
    serdes_u32(serdes, strl("scene_hash"), &o->scene.hash, 0);

    if (!def) {
        u32 hash = o->obj.hash;
        def = AssetObj(&o->obj);
        o->obj.hash = hash;
    }

    serdes_Vec2(serdes, strl("pos"), &o->pos, (Vec2){0});
    serdes_Vec2(serdes, strl("size"), &o->size, (Vec2){0});
    serdes_Vec2(serdes, strl("origin"), &o->origin, (Vec2){0});
    serdes_Vec2(serdes, strl("scale"), &o->scale, (Vec2){0});
    serdes_f32(serdes, strl("angle"), &o->angle, 0.0);
    serdes_color(serdes, strl("color"), &o->color, def->color);
    serdes_u64(serdes, strl("layermask"), &o->layermask, def->layermask);
    serdes_u64(serdes, strl("flags"), &o->flags, def->flags);
    return 0;
}

s32 serdes_scene(Serdes *serdes, Scene *s) { 
    Asset *ass = SceneAsset(s);
    serdes_u32(serdes, strl("scene"), &ass->hash, -1);

    if (serdes->is_writing) {
        serdes_print(serdes, strl("objs: [\n")); 
        for (SceneObjChunk *c = &s->first; c; c = c->next) {
            for (s32 i = 0; i < c->objs; i++) {
                Obj *o = c->obj + i;
                serdes_print(serdes, strl("{\n"));  
                serdes->indent++;
                serdes_obj(serdes, o, 0);
                serdes->indent--;
                serdes_print(serdes, strl("},\n")); 
            }
        }
        serdes_print(serdes, strl("],\n"));
    } else {
        SceneObjChunk *c = &s->first; c->objs = 0;
        json_token *objs = json_find_key(&serdes->json, 0, strl("objs"));
    
        serdes_push_parent(serdes, objs);
        for (json_iter(&serdes->json, objs, obj)) {
            if (c->objs == SCENE_OBJ_CHUNK_SIZE) {
                if (!c->next) {
                    c->next = AllocObjChunk(serdes->perm);
                }
                if (c->next) {
                    s->objs += SCENE_OBJ_CHUNK_SIZE;
                    c->next->objs = 0;
                }
                c = c->next; 
            }
            
            serdes_push_parent(serdes, obj);
            serdes_obj(serdes, c->obj + c->objs++, 0);
            serdes_pop_parent(serdes);
        }
        serdes_pop_parent(serdes);

        if (c) {
            s->objs += c->objs;
        }
    }

    serdes_Rect(serdes, strl("obj_bounds"), &s->obj_bounds, (Rect){0, 0, 32, 32});
    return 0;
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "test");
    SetTargetFPS(60);

    Arena *a = Arena_create();
    G = Arena_take(a, sizeof(*G));

    str scene_json = os_ReadFile(a, strl("test.dd"));
    Scene *s = G->assets.scene + 1;
    {
        Serdes *des = des_start(a, Arena_scratch(), scene_json);
        serdes_scene(des, s);
        des_end(des);
    }
   
    {
        Serdes *ser = ser_start(Arena_scratch());
        serdes_scene(ser, s);
        ser_end(ser, strl("test_ser.dd"));
    }
    
  //   while (!WindowShouldClose()) {
  //       BeginDrawing();
		// ClearBackground(WHITE);
		
  //       /* Draw Here */

  //       DrawFPS(16, 16);
  //       EndDrawing();
  //   }
	
    CloseWindow(); 
    return 0;
}
