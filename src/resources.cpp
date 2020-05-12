#if !defined(OSK_RES_H)
#define OSK_RES_H

struct Animation
{
    float duration = 1.f;
    fvec2 start = {0, 0};
    u32 size = 1;
    b32 loop = true;
};

////////////////////////////////
////////////////////////////////
/* Tilemaps
Define all of the tilemaps by associating them with a name, and pass
the extra required information. All of the rendering is done through
tilemaps.
*/
#define ALL_TILEMAPS \
/*          NAME               PATH IN FOLDER                             rows cols*/\
DEF_TILEMAP(TM_essentials      ,"res/essentials.png"                      ,5    ,5  )\
DEF_TILEMAP(dana               ,"res/dana_all.png"                        ,4    ,6  )\
DEF_TILEMAP(TM_fairies         ,"res/fairies.png"                         ,1    ,2  )\
DEF_TILEMAP(tmgoblin           ,"res/goblin_all.png"                      ,4    ,7  )\
DEF_TILEMAP(TM_blueflame       ,"res/blue_flame.png"                      ,2    ,7  )\
DEF_TILEMAP(tmghost            ,"res/ghost_all.png"                       ,2    ,3  )\
DEF_TILEMAP(tmdana_fire        ,"res/dana_fireball.png"                   ,3    ,1  )\
DEF_TILEMAP(TM_effects         ,"res/effects.png"                         ,5    ,4  )\
DEF_TILEMAP(font               ,"res/font.png"                            ,6    ,16 )\
DEF_TILEMAP(misc               ,"res/misc.png"                            ,4    ,4  )\
DEF_TILEMAP(TM_pickups         ,"res/pickups.png"                         ,1    ,19 )\

struct RESTilemap
{
    const char* const name;
    i32 rows, cols;
};
#define DEF_TILEMAP(name, path, rows, cols) TILEMAP_##name,

enum E_TILEMAPS { ALL_TILEMAPS TILEMAP_COUNT };

#undef DEF_TILEMAP
// #define DEF_TILEMAP(name, path, rows, cols) [TILEMAP_##name] = {path, rows, cols},
#define DEF_TILEMAP(name, path, rows, cols) {path, rows, cols},

#define GET_TILEMAP_TEXTURE(name) g_tilemap_textures[TILEMAP_##name]
global GLTilemapTexture g_tilemap_textures[TILEMAP_COUNT];
////////////////////////////////
////////////////////////////////

////////////////////////////////
// Backgrounds

#define ALL_BACKGROUNDS \
B("res/bg/bg_room01.png")\
B("res/bg/bg_room02.png")\
B("res/bg/bg_room03.png")\
B("res/bg/bg_room04.png")\
B("res/bg/bg_room05.png")\
B("res/bg/bg_room06.png")\
B("res/bg/bg_room07.png")\
B("res/bg/bg_room08.png")\
B("res/bg/bg_room09.png")\
B("res/bg/bg_room10.png")\
B("res/bg/bg_room11.png")\
B("res/bg/bg_room12.png")\
B("res/bg/bg_room13.png")\
B("res/bg/bg_room14.png")\
B("res/bg/bg_room15.png")\
B("res/bg/bg_room16.png")\
B("res/bg/bg_room17.png")\
B("res/bg/bg_room18.png")\

struct RESBackground {
    const char *name;
};

RESBackground g_background_files[] {
#define B(name) name,
    ALL_BACKGROUNDS
#undef B
};


////////////////////////////////
////////////////////////////////
/* ANIMATIONS
You can group animations by "character", and the macro ugliness here
should prevent name collisions, if you input different character names.
*/
#define ALL_CHARACTERS                                                              \
/*            Name,        Tilemap,  Animation Count */                         \
DEF_CHARACTER(test_player, dana,         4,                                     \
/*       Character,    Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(test_player,  Idle,    .1f,      {0,0}, 1,      false)   \
DEF_ANIM(test_player,  Run ,    .1f,      {0,1}, 5,      true )   \
DEF_ANIM(test_player,  Cast,    .05f,     {0,2}, 3,      false)   \
DEF_ANIM(test_player,  Crouch,  .05f,     {0,3}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(Goblin,      tmgoblin,     5,                                     \
/*       Character,    Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Goblin,       Walk,    .2f,      {0,0}, 6,      true )   \
DEF_ANIM(Goblin,       Punch,   .18f,     {0,1}, 4,      false)   \
DEF_ANIM(Goblin,       Chase,   .15f,     {0,2}, 7,      true )   \
DEF_ANIM(Goblin,       Wait,    1.f,      {5,0}, 1,      false)   \
DEF_ANIM(Goblin,       Fall,    .1f,      {0,3}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(Ghost,       tmghost,      2,                                     \
/*       Character,    Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Ghost,        Fly,     .1f,      {0,0}, 3,      true )   \
DEF_ANIM(Ghost,        Punch,   .1f,      {0,1}, 3,      false)   \
)                                                                 \
DEF_CHARACTER(BlueFlame,   TM_blueflame, 2,                                     \
DEF_ANIM(BlueFlame,    Normal,  .1f,      {0,0}, 7,      true)    \
DEF_ANIM(BlueFlame,    Tame,    .1f,      {0,1}, 4,      true)    \
)                                                                 \
DEF_CHARACTER(DFireball,   tmdana_fire,  3,                                     \
/*       Character,    Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(DFireball,    Down,    .1f,      {0,0}, 1,      true )   \
DEF_ANIM(DFireball,    Middle,  .1f,      {0,1}, 1,      true )   \
DEF_ANIM(DFireball,    Up,      .1f,      {0,2}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(Effect,      TM_effects,   5,                                     \
DEF_ANIM(Effect,       Smoke,   .1f,      {0,0}, 4,      false)   \
DEF_ANIM(Effect,       Star,    .1f,      {0,1}, 3,      true )   \
DEF_ANIM(Effect,       Flash,   .1f,      {0,2}, 2,      false)   \
DEF_ANIM(Effect,       Flash2,  .1f,      {0,3}, 3,      false)   \
DEF_ANIM(Effect,       Hit,     .1f,      {0,4}, 4,      false)   \
)                                                                 \
DEF_CHARACTER(Door,        TM_essentials,2,                                     \
DEF_ANIM(Door,         Close,   .1f,      {0,2}, 5,      false)   \
DEF_ANIM(Door,         Open,    .1f,      {0,3}, 5,      false)   \
)                                                                 \
DEF_CHARACTER(Key,         TM_essentials,1,                                     \
DEF_ANIM(Key,          Default, .1f,      {0,4}, 1,      false)   \
)                                                                 \
DEF_CHARACTER(Fairie,      TM_fairies,   1,                                     \
DEF_ANIM(Fairie,       Default, .1f,      {0,0}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(KMirror,     TM_essentials,1,                                     \
DEF_ANIM(KMirror,      Default, .1f,      {3,1}, 1,      false)   \
)                                                                 \

#define DEF_ANIM(character, name, ...) CHARACTER_##character##_anim_##name,
#define DEF_CHARACTER(name, tilemap, anim_count, ...) enum E_##name##_anims{   \
__VA_ARGS__ CHARACTER_##name##_animcount\
}; \


ALL_CHARACTERS


#undef DEF_ANIM
#undef DEF_CHARACTER

#define DEF_ANIM(character, name, ...)
#define DEF_CHARACTER(name, tilemap, anim_count, ...) CHARACTER_##name ,

enum E_CHARACTERS
{
    ALL_CHARACTERS CHARACTER_COUNT
};


#undef DEF_ANIM
#undef DEF_CHARACTER

#define DEF_ANIM(character, name, ...)
#define DEF_CHARACTER(name, tilemap, anim_count, ...) TILEMAP_##tilemap ,

global u64 CHARACTER_TO_TILEMAP[CHARACTER_COUNT] =
{
    ALL_CHARACTERS
};

#undef DEF_ANIM
#undef DEF_CHARACTER

#define DEF_ANIM(character, name, ...) {__VA_ARGS__},
#define DEF_CHARACTER(name, tilemap, anim_count, ...) \
global Animation CHARACTER_##name##_anims[anim_count] = { \
__VA_ARGS__ \
};

ALL_CHARACTERS

#undef DEF_ANIM
#undef DEF_CHARACTER

// Use this with a variable of type ux to hold current animation
#define GET_ANIM_BY_HANDLE(character, hnd) CHARACTER_##character##_anims[hnd]
#define GET_CHAR_ANIM_COUNT(character) CHARACTER_##character##_animcount
#define GET_CHAR_ANIM_HANDLE(character, name) CHARACTER_##character##_anim_##name
#define GET_CHAR_ANIM(character, name) CHARACTER_##character##_anims[CHARACTER_##character##_anim_##name]
#define GET_CHAR_TILEMAP(character) g_tilemap_textures[CHARACTER_TO_TILEMAP[ CHARACTER_##character ]]

#define GET_CHAR_ANIMSET(character) CHARACTER_##character##_anims

// Get the identifier of the animation
#define GET_CHAR_ANIMENUM(character, name) CHARACTER_##character##_anim_##name

////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////

internal u8*
load_image_as_rgba_pixels(const char* const name, i32* out_width, i32* out_height, i32* out_n) {
    int i_w, i_h, i_n;
    unsigned char* data = stbi_load(name, out_width, out_height, out_n, 4);
    
    return data;
}


internal void gl_load_background_texture(long bgn) {
    i32 w,h,n;
    u8 *data = load_image_as_rgba_pixels(g_background_files[bgn].name, &w, &h, &n);
    assert(data);
    gl_update_rgba_texture(data, w, h, g_background_texture_id);
    free(data);
}

internal void load_tilemap_textures()
{
    i32 width, height, bpp;
    u8* data;
    
    const RESTilemap RES_TILEMAPS[TILEMAP_COUNT] =
    {
        ALL_TILEMAPS
    };
#undef DEF_TILEMAP
    
    assert(TILEMAP_COUNT > 0);
    
    for (u32 i = 0 ;i < TILEMAP_COUNT; ++i)
    {
        inform("loading tilemap: %s...",RES_TILEMAPS[i].name);
        data = load_image_as_rgba_pixels(
                                         RES_TILEMAPS[i].name,
                                         &width, &height, &bpp);
        assert(data);
        
        g_tilemap_textures[i] = gl_load_rgba_tilemap(
                                                     data,
                                                     width, height,
                                                     RES_TILEMAPS[i].rows, RES_TILEMAPS[i].cols);
        
    }
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("TMerr: %d\n", err);
        exit(-1);
    }
    
}

////////////////////////////////
// AUDIO
internal RESSound Wave_load_from_file(const char* file);

#define ALL_SOUNDS \
DEF_SOUND(SND_background, "res/bgm1.wav" ) \
DEF_SOUND(SND_jump,       "res/bloop.wav") \
DEF_SOUND(SND_show_key,   "res/audio/key_appear.wav") \
DEF_SOUND(SND_show_player,"res/audio/player_appear.wav") \
DEF_SOUND(SND_hurry,      "res/audio/hurry.wav") \

#define DEF_SOUND(name, path) name,
enum E_ALL_SOUNDS {
    ALL_SOUNDS
        ALL_SOUNDS_COUNT
};
#undef DEF_SOUND

#define DEF_SOUND(name, path) path,
const char *ALL_SOUND_PATHS[ALL_SOUNDS_COUNT] = {
    ALL_SOUNDS
};
#undef DEF_SOUND

RESSound ALL_SOUND_RES[ALL_SOUNDS_COUNT];
#define GET_SOUND(name) ((RESSound *)&ALL_SOUND_RES[name])

internal void load_sound_resources() {
    for (u32 i = 0; i < ALL_SOUNDS_COUNT; ++i) {
        ALL_SOUND_RES[i] = Wave_load_from_file(ALL_SOUND_PATHS[i]);
    }
}

#endif //!OSK_RES_H
