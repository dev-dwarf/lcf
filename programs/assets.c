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

#define MAX_ASSET_LIST 64
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
    [ASSET_LIST] = MAX_ASSET_LIST,
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

typedef struct AssetInfo {
    AssetHandle asset_handle;
    str name;
    str file;
} AssetInfo;

typedef struct AssetList {
    AssetInfo info;
    s32 assets;
    s32 loaded;
    AssetHandle *first;
} AssetList;

typedef struct Inst {
    u32 hash;
    u16 slot;
    u8 gen;
    u8 real;
} Inst;

typedef struct Obj {
    Inst id;
    
    Vec2 pos;
    Vec2 origin;
    Vec2 size;
    float angle;
    Color color;
    u64 layermask;
    u64 flags;

    b32 deactivated;
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
    // AnimationState anim;

    AssetInfo info;
    AssetHandle ase; // Textures Asset
    AssetHandle scene; // Child Scene Asset
    
    // Handle parent;
    Vec2 parent_pos;

    // Handle next;
    // Handle prev;
    // Handle first_child;
    // s32 total_children; /* TODO right now only valid when initially spawning */

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
    // AssetHandle scene;
    s32 objs;
    Obj obj[SCENE_OBJ_CHUNK_SIZE];
} SceneObjChunk;

typedef struct Scene {
    AssetInfo info;
    
    Rect obj_bounds;
    // Rect tile_bounds;
        
    s32 objs;
    SceneObjChunk first;
} Scene;

struct _Assets {
    // Chunking for scenes
    SceneObjChunk *free_obj_chunk;

    // Tables
    AssetHandle *loaded[ASSET_TYPES];
    AssetList list[MAX_ASSET_LIST]; s32 lists;
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

    if (out->id.hash != handle->hash) {
        out = G->assets.obj;
        for (s32 i = 1; i < G->assets.objs; i++) {
            if (objs[i].hash == handle->hash) {
                out = G->assets.obj + objs[i].slot;
            }
        }
    }

    if (out != G->assets.obj) {
        *handle = out->info.asset_handle;
    }

    return out;
}

Scene* AssetScene(AssetHandle *handle) {
    // TODO(lcf)
    return 0;
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
    ASSERT(inst->hash);
    Obj *o = G->obj + inst->slot;
    if (inst->real) {
        if (!memcmp(inst, &o->id, sizeof(Inst))) {
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
    *inst = o->id;
    return o;
}

Inst SpawnObj(Obj *parent, Obj *o) {
    // TODO lcf
    return (Inst){0};
}

Inst SpawnScene(Obj *parent, Scene *scene) {
    // TODO: handle being root scene (no parent obj)
    
    s32 spawned_objects = 0;
    for (SceneObjChunk *c = &scene->first; c; c = c->next) {
        for (s32 i = 0; i < c->objs; i++) {
            // Inst inst = SpawnObj(c->obj + i, parent);
            spawned_objects++;
        }
    }

    return (Inst){0};
}

typedef struct Serdes {
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
} Serdes;

static void serdes_push_parent(Serdes *serdes, json_token *p) {
    serdes->json->parent[++serdes->json->p] = p - serdes->json->token;
    serdes->parent = p;
}

static void serdes_pop_parent(Serdes *serdes) {
    serdes->parent = serdes->json->parent[--serdes->json->p] + serdes->json->token;
    if (serdes->json->p == 0) {
        serdes->parent = 0;
    }
}

static str serdes_str(Serdes *serdes, str key, str v, str def) {
    if (serdes->is_writing) {
        if (!str_eq(v, def)) {
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t:%.*s: %.*s,\n", key, v));
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
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: %lld,\n", key.len, key.str, v));
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
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: 0x%08llX,\n", key.len, key.str, v));
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
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: %f,\n", key.len, key.str, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        v = t? str_to_f64(t->str, 0) : def;
    }
    return v;
}
static Color serdes_color(Serdes *serdes, str key, Color v, Color def) {
    if (serdes->is_writing) {
        if (memcmp(&v, &def, sizeof(v))) {
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: 0x%08X,\n", key.len, key.str, v));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        if (t) {
            u32 raw = str_to_u64(t->str, 0); 
            memcpy(&v, &raw, sizeof(Color));
        } else {
            v = def;
        }
    }
    return v;
}
static Vec2 serdes_Vec2(Serdes *serdes, str key, Vec2 v, Vec2 def) {
    if (serdes->is_writing) {
        if (memcmp(&v, &def, sizeof(v))) {
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: [%f, %f],\n", key.len, key.str, v.x, v.y));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        if (t) {
            v.x = str_to_f64(t[1].str, 0);
            v.y = str_to_f64(t[2].str, 0);
        } else {
            v = def;
        }
    }
    return v;
}
static Vec3 serdes_Vec3(Serdes *serdes, str key, Vec3 v, Vec3 def) {
    if (serdes->is_writing) {
        if (memcmp(&v, &def, sizeof(v))) {
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: [%f, %f, %f],\n", key.len, key.str, v.x, v.y, v.z));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        if (t) {
            v.x = str_to_f64(t[1].str, 0);
            v.y = str_to_f64(t[2].str, 0);
            v.z = str_to_f64(t[3].str, 0);
        } else {
            v = def;
        }
    }
    return v;
}
static Vec4 serdes_Vec4(Serdes *serdes, str key, Vec4 v, Vec4 def) {
    if (serdes->is_writing) {
        if (memcmp(&v, &def, sizeof(v))) {
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: [%f, %f, %f, %f],\n", key.len, key.str, v.x, v.y, v.z, v.w));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        if (t) {
            v.x = str_to_f64(t[1].str, 0);
            v.y = str_to_f64(t[2].str, 0);
            v.z = str_to_f64(t[3].str, 0);
            v.w = str_to_f64(t[4].str, 0);
        } else {
            v = def;
        }
    }
    return v;
}
static Rect serdes_Rect(Serdes *serdes, str key, Rect v, Rect def) {
    if (serdes->is_writing) {
        if (memcmp(&v, &def, sizeof(v))) {
            StrList_push(serdes->temp, &serdes->strl, strf(serdes->temp, "\t%.*s: [%f, %f, %f, %f],\n", key.len, key.str, v.x, v.y, v.w, v.h));
        }
    } else {
        json_token *t = json_find_key(serdes->json, serdes->parent, key);
        if (t) {
            v.x = str_to_f64(t[1].str, 0);
            v.y = str_to_f64(t[2].str, 0);
            v.w = str_to_f64(t[3].str, 0);
            v.h = str_to_f64(t[4].str, 0);
        } else {
            v = def;
        }
    }
    return v;
}

static void serdes_obj(Serdes *serdes, Obj *o, Obj *def) {
    o->info.asset_handle.hash = serdes_u64(serdes, strl("asset_hash"), o->info.asset_handle.hash, 0);
    o->editor_hash = serdes_u64(serdes, strl("editor_hash"), o->editor_hash, 0);
    o->scene.hash = serdes_u64(serdes, strl("scene_hash"), o->scene.hash, 0);

    if (!def) {
        def = AssetObj(&o->info.asset_handle);
    }

    o->pos = serdes_Vec2(serdes, strl("pos"), o->pos, (Vec2){0});
    o->size = serdes_Vec2(serdes, strl("size"), o->size, (Vec2){0});
    o->origin = serdes_Vec2(serdes, strl("origin"), o->origin, (Vec2){0});
    o->scale = serdes_Vec2(serdes, strl("scale"), o->scale, (Vec2){0});
    o->angle = serdes_f32(serdes, strl("angle"), o->angle, 0.0);
    o->color = serdes_color(serdes, strl("color"), o->color, def->color);
    o->layermask = serdes_u64(serdes, strl("layermask"), o->layermask, def->layermask);
    o->flags = serdes_u64(serdes, strl("flags"), o->flags, def->flags);
}

static void serdes_scene(Serdes *serdes, Scene *s) { 
    if (serdes->is_writing) {
        StrList_push(serdes->temp, &serdes->strl, strl("objs: [\n"));

        for (SceneObjChunk *c = &s->first; c; c = c->next) {
            for (s32 i = 0; i < c->objs; i++) {
                Obj *o = c->obj + i;
                StrList_push(serdes->temp, &serdes->strl, strl("{\n"));
                serdes_obj(serdes, o, 0);
                StrList_push(serdes->temp, &serdes->strl, strl("},\n"));
            }
        }
        StrList_push(serdes->temp, &serdes->strl, strl("], \n"));
    } else {
        SceneObjChunk *c = &s->first; c->objs = 0;
        json_token *objs = json_find_key(serdes->json, 0, strl("objs"));
    
        serdes_push_parent(serdes, objs);
        for (json_iter(serdes->json, objs, obj)) {
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

    serdes_Rect(serdes, strl("obj_bounds"), s->obj_bounds, (Rect){0, 0, 32, 32});

}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "test");
    SetTargetFPS(60);

    Arena *a = Arena_create();
    G = Arena_take(a, sizeof(*G));

    str scene = os_ReadFile(a, strl("test.dd"));
    
    json j = (json){
        .input = scene,
        .arena = a,
    };

    Serdes serdes = (Serdes){
        .temp = Arena_scratch(),
        .perm = a,
        .json = &j,
    };
    
    json_parse(&j);

    Scene *s = G->assets.scene + 0;
    serdes_scene(&serdes, s);
    
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
