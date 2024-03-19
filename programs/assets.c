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

#define JSMN_PARENT_LINKS
#include "..\libs\jsmn.h"

jsmntok_t* jsmn_key(char *json, jsmntok_t *root, s32 parent, str key) {
    int children = root[parent].size;
    for (jsmntok_t *t = root + parent+1; children > 0; t++) {
        if (t->parent == parent) {
            children--;

            str tstr = { .str = json + t->start, .len = t->end - t->start };
            if (str_eq(tstr, key)) {
                return t+1;
            }
        }
    }
    return 0;
}

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define MAX_ASSET_LISTS 64
#define MAX_ASSET_OBJ 2048
#define MAX_ASSET_ASEPRITE 2048
#define MAX_ASSET_TILESET 512
#define MAX_ASSET_SCENE 2048

#define SCENE_OBJ_CHUNK_SIZE 128
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
    [ASSET_LIST] = MAX_ASSET_LISTS,
    [ASEPRITE] = MAX_ASSET_ASEPRITE,
    [OBJ] = MAX_ASSET_OBJ,
    [TILESET] = MAX_ASSET_TILESET,
    [SCENE] = MAX_ASSET_SCENE,
};

typedef struct AssetHandle {
    u32 hash;
    u16 slot;
    u8 type;
    u8 list;
} AssetHandle;

typedef struct AssetList {
    struct AssetList *next;
    AssetInfo info;
    s32 assets;
    s32 loaded;
    AssetHandle first;
} AssetList;

typedef struct AssetInfo {
    AssetHandle asset_handle;
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
    Inst id;
    
    Vector2 pos;
    Vector2 origin;
    Vector2 size;
    float angle;
    Color color;
    u64 layermask;
    u64 flags;

    b32 deactivated;
    s32 scene_depth;
    Vector2 scale;
    Vector2 pspeed;
    Vector2 speed;
    f32 accel_lerp;
    f32 angle_speed;
    Vector2 shake;
    Vector2 wave;
    f32 wavefreq;
    
    s32 state;
    s32 timer;
    s32 on_ground;
    s32 jump;
    s32 flip;
    // AnimationState anim;

    AssetInfo info;
    AssetHandle ase; // Textures Asset
    AssetHandle scene; // Child Scene Asset
    
    // Handle parent;
    Vector2 parent_pos;

    // Handle next;
    // Handle prev;
    // Handle first_child;
    // s32 total_children; /* TODO right now only valid when initially spawning */

    u32 editor_hash;
    s32 editor_lister_score;
} Obj;
typedef struct Obj Obj;

struct SceneObjChunk {
    struct SceneObjChunk *next;
    // AssetHandle scene;
    s32 objs;
    Obj obj[SCENE_OBJ_CHUNK_SIZE];
};
typedef struct SceneObjChunk SceneObjChunk;
struct Scene {
    AssetInfo info;
    
    Rectangle obj_bounds;
    // Rectangle tile_bounds;
        
    s32 objs;
    SceneObjChunk first;
};
typedef struct Scene Scene;

struct _Assets {
    // Chunking for scenes
    SceneObjChunk *free_obj_chunk;

    // Tables
    AssetHandle *loaded[ASSET_TYPES];
    AssetList list[MAX_ASSET_INFO]; s32 lists;
    Obj obj[MAX_ASSET_OBJ]; s32 objs;
    Scene scene[MAX_ASSET_SCENE]; s32 scenes;
};
struct _Global {
    Arena *asset_memory;
    struct _Assets assets;
    
    s32 obj_max;
    Obj *obj;
    Obj *free_obj;
    Scene *current_scene;
};
struct _Global *G;

Obj* AssetObj(AssetHandle *handle) {
    // TODO(lcf) this is super stupid for now
    AssetHandle *objs = G->assets.loaded[OBJ];
    Obj* out = G->assets.obj + handle->slot;

    if (out->asset.hash != handle->hash) {
        out = objs;
        for (s32 i = 1; i < G->assets.objs; i++) {
            if (objs[i].hash == handle->hash) {
                out = objs[i].hash;
            }
        }
    }

    if (out != objs) {
        *handle = out->asset;
    }

    return out;
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
    
    SceneObjChunk *c = &scene->first; c->objs = 0;
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
        }
    }

    if (c->next) { // place now unused chunks on free list
        SceneObjChunk *first = c->next
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
    return scene->objs;
}

// Finds inst in the current scene
// Uses:
// just hash -> find first instance of obj with hash
// hash + slot -> find next instance of obj with hash
// hash,slot,gen,real -> find exact instance
Obj* InstObj(Inst *inst) {
    ASSERT(inst->hash);
    Obj *o = G->obj + inst->slot;
    if (inst->real) {
        if (!memcmp(inst, o->id, sizeof(Inst))) {
            o = G->obj;
        } 
    } else {
        o++; // Increment to skip null inst or slot
        Obj *O = G->obj + G->obj_max;
        while (o->id.hash != inst->hash) {
            if (++o > O) {
                o = G->obj;
                break;
            }
        }
    }
    o->real = true;
    return o;
}

Inst SpawnObj(Obj *parent, Obj *o) {

}

Inst SpawnScene(Obj *parent, AssetHandle scene_asset) {
    // TODO: handle being root scene
    

    s32 spawned_objects = 0;
    for (SceneObjChunk *c = &scene->first; c; c = c->next)
        for (s32 i = 0; i < c->objs; i++) {
            spawned_objects += SpawnObj(c->obj + i, parent);
        }
    }

    return 
}

struct Serdes {
    Arena *temp;
    Arena *perm;
    
    // Ser
    StrList strl;

    // Des
    json *json;
    json_token *parent;

    s16 version;
    s16 is_writing;
    s32 counter;
};

static void serdes_push_parent(Serdes *serdes, json_token *p) {
    serdes->json->parent[++serdes->json->p] = p - serdes->json->token;
    serdes->parent = p;
}

static void serdes_pop_parent(Serdes *serdes) {
    serdes->parent = serdes->json->parent[--serdes->json->p] + serdes->json->token;
}

static str serdes_str(Serdes *serdes, str key, str v, str def) {
    if (serdes->is_writing) {
        if (!str_eq(v, def)) {
            StrList_push(serdes->temp, serdes->strl, strf(serdes->temp, "\t:%.*s: %.*s,", key, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        v = t? str_copy(serdes->perm, t->str) : def;
    }
    return v;
}
static s64 serdes_s64(Serdes *serdes, str key, s64 v, s64 def) {
    if (serdes->is_writing) {
        if (v != def) {
            StrList_push(serdes->temp, serdes->strl, strf(serdes->temp, "\t%.*s: %lld,", key.len, key.str, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        v = t? str_to_s64(t->str, 0) : def;
    }
    return v;
}
static u64 serdes_u64(Serdes *serdes, str key, u64 v, u64 def) {
    if (serdes->is_writing) {
        if (v != def) {
            StrList_push(serdes->temp, serdes->strl, strf(serdes->temp, "\t%.*s: 0x%08llX,", key.len, key.str, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        v = t? str_to_u64(t->str, 0) : def;
    }
    return v;
}
static f32 serdes_f32(Serdes *serdes, str key, f32 v, f32 def) {
    if (serdes->is_writing) {
        if (v != def) {
            StrList_push(serdes->temp, serdes->strl, strf(serdes->temp, "\t%.*s: %f,", key.len, key.str, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        v = t? str_to_f64(t->str, 0) : def;
    }
    return v;
}
static u64 serdes_color(Serdes *serdes, str key, Color v, Color def) {
    if (serdes->is_writing) {
        if (memcmp(&v, &def, sizeof(Color))) {
            StrList_push(serdes->temp, serdes->strl, strf(serdes->temp, "\t%.*s: 0x%08X,", key.len, key.str, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        if (t) {
            u32 raw = str_to_u64(t->str, 0); 
            memcpy(&v, raw, sizeof(Color));
        } else {
            v = def;
        }
    }
    return v;
}

static void serdes_obj(Serdes *serdes, Obj *obj, Obj *def) {
    o->asset.hash = serdes_u64(serdes, strl("asset_hash"), o->asset.hash, 0);
    o->editor_hash = serdes_u64(serdes, strl("editor_hash"), o->editor_hash, 0);
    o->scene.hash = serdes_u64(serdes, strl("scene_hash"), o->scene.hash, 0);

    if (!def) {
        def = AssetObj(o->asset);
    }
    
    o->pos.x = serdes_f32(serdes, strl("pos_x"), o->pos.x, 0.0);    
    o->pos.y = serdes_f32(serdes, strl("pos_y"), o->pos.y, 0.0);
    o->size.x = serdes_f32(serdes, strl("size_x"), o->size.x, def->size.x);
    o->size.y = serdes_f32(serdes, strl("size_y"), o->size.y, def->size.y);
    o->origin.x = serdes_f32(serdes, strl("origin_x"), o->origin.x, 0.0);
    o->origin.y = serdes_f32(serdes, strl("origin_y"), o->origin.y, 0.0);
    o->scale.x = serdes_f32(serdes, strl("scale_x"), o->scale.x, def->scale.x);
    o->scale.y = serdes_f32(serdes, strl("scale_y"), o->scale.y, def->scale.y);
    o->angle = serdes_f32(serdes, strl("angle"), o->angle, 0.0);
    o->color = serdes_color(serdes, strl("color"), o->color, def->color);
    o->layermask = serdes_u64(serdes, strl("layermask"), o->layermask, def->layermask);
    o->flags = serdes_u64(serdes, strl("flags"), o->flags, def->flags);
}

static void serdes_scene(Serdes *serdes, Scene *s) { 
    if (serdes->is_writing) {
        StrList_push(serdes->temp, serdes->strl, strl("objs: [\n"));
        for (SceneObjChunk *c = &s->first; c; c = c->next) {
            for (s32 i = 0; i < c->objs; i++) {
                Obj *o = c->obj[i];
                StrList_push(serdes->temp, serdes->strl, strl("{\n"));
                serdes_obj(serdes, o, 0);
                StrList_push(serdes->temp, serdes->strl, strl("},\n"));
            }
        }
        StrList_push(serdes->temp, serdes->strl, strl("], \n"));

        serdes_f32(serdes, strl("obj_bounds.x"), s->obj_bounds.x, 0.0);
        serdes_f32(serdes, strl("obj_bounds.y"), s->obj_bounds.y, 0.0);
        serdes_f32(serdes, strl("obj_bounds.width"), s->obj_bounds.width, 32.0);
        serdes_f32(serdes, strl("obj_bounds.height"), s->obj_bounds.height, 32.0);
    } else {
        SceneObjChunk *c = &s->first; c->objs = 0;
        json_token *objs = json_find_key(serdes->json, 0, strl("objs"));
    
        serdes_push_parent(serdes, objs);
        for (json_iter(serdes->json, objs, obj)) {
            serdes_push_parent(serdes, obj)
            serdes_obj(serdes, c->obj + c->objs++, 0);
            serdes_pop_parent(serdes);
        
            if (c->objs == SCENE_OBJ_CHUNK_SIZE) {
                if (!c->next) {
                    c->next = AllocObjChunk(serdes->perm);
                }
                c = c->next; c->objs = 0;
            }
        }
        serdes_pop_parent(serdes);
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "test");
    SetTargetFPS(60);

    // jsmntok_t tok[256];
    // jsmn_parser p; jsmn_init(&p);

    // s32 count = jsmn_parse(&p, scene, strlen(scene), tok, 256);

    // char *types[1 << 4] = {
    //     [JSMN_OBJECT] = "(Object)",
    //     [JSMN_ARRAY] = "(Array)",
    //     [JSMN_STRING] = "(String)",
    //     [JSMN_PRIMITIVE] = "(Primitive)",
    // };
    // for (s32 i = 0; i < count; i++) {
    //     jsmntok_t t = tok[i];
    //     printf("%s, %.*s\n", types[t.type], t.end - t.start, scene+t.start);
    // }

    // jsmntok_t *pt = jsmn_key(scene, tok, 0, strl("objs"));
    // jsmntok_t t = *pt;
    // printf("%s, %.*s\n", types[t.type], t.end - t.start, scene+t.start);

    Arena *a = Arena_create();
    json j = (json){
        .input = str_from_cstring(scene),
        .arena = a,
    };
    json_parse(&j);
    
    json_token *objs = json_find_key(&j, 0, strl("objs")); // 0 is root param
    s32 i = 0;
    for (json_iter(&j, objs, obj)) {
        printf("%d\n", i++);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
		ClearBackground(WHITE);
		
        /* Draw Here */

        DrawFPS(16, 16);
        EndDrawing();
    }
	
    CloseWindow(); 
    return 0;
}
