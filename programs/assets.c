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

#define MAX_ASSET_INFO 8192
#define MAX_ASSET_OBJ 2048
#define MAX_ASSET_ASEPRITE 2048
#define MAX_ASSET_TILESET 512
#define MAX_ASSET_SCENE 2048
#define SCENE_OBJ_CHUNK_SIZE 128

enum AssetTypes {
    INFO = 0,
    ASSET_LIST,
    ASEPRITE,
    OBJ,
    TILESET,
    SCENE,

    ASSET_TYPES
};

typedef struct AssetHandle {
    u32 hash;
    u16 type;
    u16 slot;
} AssetHandle;

struct AssetInfo {
    struct AssetInfo *next;
    AssetHandle id;
    str name;
    str file;
};
typedef struct AssetInfo AssetInfo;

struct Obj { 
    u64 bruh; 
    s32 scene_depth;
    AssetHandle scene;
};
typedef struct Obj Obj;

struct SceneObjChunk {
    struct SceneObjChunk *next;
    s32 objs;
    Obj obj[SCENE_OBJ_CHUNK_SIZE];
};
typedef struct SceneObjChunk SceneObjChunk;
struct Scene {
    AssetInfo *info;
    s32 objs;
    SceneObjChunk first;
};
typedef struct Scene Scene;

struct _Assets {
    // Chunking for scenes
    SceneObjChunk *free_obj_chunk;

    
    // Tables
    AssetInfo asset_info[MAX_ASSET_INFO]; s32 asset_infos;
    Obj asset_obj[MAX_ASSET_OBJ]; s32 asset_objs;
    Scene asset_scene[MAX_ASSET_SCENE]; s32 asset_scenes;

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

s32 SaveCurrentSceneAsset() {
    Scene *scene = G->current_scene;
    Obj *obj = G->obj;
    Obj *obj_end = obj + G->obj_max;
    s32 scene_objs = 0;
    
    SceneObjChunk *c = &scene->first; c->objs = 0;
    
    while (obj != obj_end) {
        if (c->objs == SCENE_OBJ_CHUNK_SIZE) {
            if (!c->next) {
                if (G->assets.free_obj_chunk) {
                    c->next = G->assets.free_obj_chunk;
                    G->assets.free_obj_chunk = G->assets.free_obj_chunk->next;
                } else {
                    c->next = Arena_take_zero(G->asset_memory, sizeof(SceneObjChunk));
                }
            }
            c = c->next;
            c->objs = 0;
        }
        
        s32 I = MIN((s32) (obj_end - obj), SCENE_OBJ_CHUNK_SIZE - c->objs);
        for (s32 i = 0; i < I; i++) {
            Obj *o = obj++;
            if (o->scene_depth == 0) {
                memcpy(c->obj + c->objs, o, sizeof(Obj));
                scene_objs++;
                c->objs++;
            }
        }
    }

    scene->objs = scene_objs;
    return scene_objs;
}

char *bruh =  "{\
 'glossary': {\
         'title': 'example glossary',\
		 'GlossDiv': {\
             'title': 'S',\
			 'GlossList': {\
                 'GlossEntry': {\
                     'ID': 'SGML',\
					 'SortAs': 'SGML',\
					 'GlossTerm': 'Standard Generalized Markup Language',\
					 'Acronym': 'SGML',\
					 'True': true, \
					 'False': false, \
					 'Null': null, \
					 'Number': 1234,\
					 'Hex': 0xDD,\
					 'Float': 1.045e+3,\
					 'HexFloat': 0x1ffp+3,\
					 'Abbrev': 'ISO 8879:1986',\
					 'GlossDef': {\
                         'para': 'A meta-markup language, used to create markup languages such as DocBook.',\
						 'GlossSeeAlso': ['GML', 'XML']\
                     },\
					 'GlossSee': 'markup'\
                 }\
             }\
         }\
     }\
}";

char *scene = "{\
x: 0, \
y: 0, \
w: 0, \
h: 0, \
objs: [ \
 1, 2, 3, 4, 5, 6\
], \
\
}";


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

    json_token token[1024];
    json j = (json){
        .input = (str){.str = bruh, .len = strlen(bruh)},
        .token = token,
        .tokens_cap = 1024,
    };
    json_parse(&j);

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
