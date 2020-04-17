/*
eBlocks + eEmptySpace
> associate a pickup within that tile; if the tile
> is occupied by a block, then breaking the block will reveal
> the item. If it's not occupied then we read an extra param to say
> whether or not you have to create & break a block there to reveal it,
> or if it's there at startup


*/


internal Sprite *scene_sprite_add(Sprite *sprite)
{
    fail_unless(sprite, "Passing null sprite to scene_add");
    
    if (sprite->entity.type == ePickup) {
        g_scene.pickup_list.push_back(*sprite);
        return &g_scene.pickup_list.back();
    } else {
        g_scene.spritelist.push_back(*sprite);
        return &g_scene.spritelist.back();
    }
}

internal const char* string_nextline(const char* c)
{
    while (*c != '\n') c++;
    return c + 1;
}

internal const char* string_parse(const char* c, const char* str)
{
    while (*str && *c && *c == *str) {
        c++;
        str++;
    }
    
    if (!(*str))
        return c;
    
    return 0;
}

internal const char*
string_parse_uint(const char* c, u64* out_i)
{
#define IS_DIGIT(x) (x >= '0' && x <= '9')
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
#undef IS_DIGIT
}

internal b32 is_valid_tilemap_object(EntityBaseType type)
{
    return ((u64)type <= (u64)eBlockSolid);
}

internal const char* eat_whitepspace(const char *c) {
    while(*c == ' ' || *c == '\t') c++;
    return c;
}


internal const char *parse_custom(const char *c, fvec2 pos) {
    c = eat_whitepspace(c);
    
    while (*c && *c == ',') {
        u64 object_id;
        c++;
        
        
        c = string_parse_uint(c, &object_id);
        
        Sprite pickup = make_pickup(pos, object_id);
        
        scene_sprite_add(&pickup);
        inform("Under Normal tile: %lld", object_id);
    }
    return c;
}

internal void level_load(char* data)
{
    const char* c = data;
#define OSK_LEVEL_FMT_VERSION "V0.2"
    fail_unless(string_parse(c, OSK_LEVEL_FMT_VERSION), "Version string does not match!");
    
    c += 5;
    inform("%s", "LOADING LEVEL...");
    
    u32 counter_x = 0;
    u32 counter_y = 0;
    
    while (*c) {
        switch(*c) {
            
            case '#': {
                c = string_nextline(c);
            } break;
            
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
                u64 res;
                c = string_parse_uint(c, &res);
                
                fail_unless(res < EntityBaseType_Count,
                            "Entity index does not exist in version"
                            OSK_LEVEL_FMT_VERSION);
                
                if (counter_x >= TILEMAP_COLS)
                {
                    counter_x = 0;
                    counter_y++;
                }
                
                if (is_valid_tilemap_object((EntityBaseType) res)) {
                    g_scene.tilemap[counter_x][counter_y] = (EntityBaseType)res;
                    
                    c = parse_custom(c, fvec2{ (float)counter_x * 64, (float)counter_y * 64});
                } else {
                    Sprite sprite_to_make;
                    fvec2 sprite_initial_pos = fvec2{ (float)counter_x * 64, (float)counter_y * 64};
                    
                    
                    switch((EntityBaseType)res)
                    {
                        case eGoblin:{
                            sprite_to_make = make_goblin(sprite_initial_pos);
                            c = goblin_parse_custom(&sprite_to_make, c);
                        }break;
                        
                        case ePlayerSpawnPoint:{
                            sprite_to_make = make_player(sprite_initial_pos);
                        }break;
                        
                        case eGhost:{
                            sprite_to_make = make_ghost(sprite_initial_pos);
                        }break;
                        
                        default:{
                            warn("sprite type not available for make_");
                        }break;
                    }
                    
                    sprite_to_make.position.y += 64.f - sprite_to_make.collision_box.max_y;
                    scene_sprite_add(&sprite_to_make);
                    
                    g_scene.tilemap[counter_x][counter_y] = eEmptySpace;
                }
                
                counter_x++;
                
            } break;
            
            default:
            {
                c++;
            } break;
        }
    }
    
}

internal void scene_init(const char* level_path)
{
    //assert(level_path);
    char* lvl = platform_load_entire_file(level_path);
    level_load(lvl);
    free(lvl);
    
}

internal void
scene_draw_tilemap()
{
    for(int i = 0; i < 15; ++i )
    {
        for(int j = 0; j < 12; ++j )
        {
            u32 id;
            EntityBaseType type = (EntityBaseType)g_scene.tilemap[i][j];
            
            if (type == eEmptySpace) continue;
            if (type == eBlockSolid) id = 2;
            if (type == eBlockFrail) id = 1;
            
            gl_slow_tilemap_draw(
                                 &GET_TILEMAP_TEXTURE(test),
                                 fvec2{float(i) * 64.f, j * 64.f},
                                 fvec2{64, 64},
                                 0.f,
                                 id);
        }
    }
}


inline u64 scene_get_tile(ivec2 p) {
    
    if (p.x > (TILEMAP_COLS - 1) || p.y > (TILEMAP_ROWS - 1) ||
        p.x < 0 || p.y < 0) return eBlockSolid;
    
    return g_scene.tilemap[p.x][p.y];
}
inline void scene_set_tile(ivec2 p, EntityBaseType t) { g_scene.tilemap[p.x][p.y] = t; }



// Finds first sprite of specific type
// if not found; return 0
internal const Sprite* const scene_get_first_sprite(EntityBaseType type)
{
    for (i32 i = 0; i < g_scene.spritelist.size(); i += 1)
    {
        Sprite* spref = &g_scene.spritelist[i];
        
        if (spref->entity.type == type) return spref;
    }
    return 0;
    
}

// Returns first view-blocking tile in search direction specified
// otherwise, returns {-1, -1};
internal ivec2 scene_get_first_nonempty_tile(ivec2 start_tile, ivec2 end_tile)
{
    i32 xdiff = sgn(end_tile.x - start_tile.x);
    while (start_tile != end_tile)
    {
        if (scene_get_tile(start_tile) != eEmptySpace)
        {
            return start_tile;
        }
        
        start_tile.x += xdiff;
        
        if (start_tile.x < 0 || start_tile.x > 14) return {-1, -1};
    }
    
    
    return ivec2{-1 ,-1};
}


internal void ePlayer_update(Sprite* spref, InputState* istate, float dt);
internal void eGoblin_update(Sprite* spref, InputState* istate, float dt);
internal void eDFireball_update(Sprite* spref, InputState* istate, float dt);
internal void eStarRing_update(Sprite* spref, InputState* istate, float dt);
internal void eGhost_update(Sprite* spref, InputState* istate, float dt);

internal void scene_startup_animation(float dt) {
    
    const int STATE_START = 0;
    const int STATE_SHOW_KEY = 1;
    const int STATE_SHOW_PLAYER = 2;
    const int STATE_DONE = 3;
    
    const float anim_dur = 0.8f;
    static float anim_time = 0.f;
    static Sprite ring_static;
    Sprite *ring;
    
    ring = &ring_static;
    
    const Sprite *const player = scene_get_first_sprite(ePlayer);
    
    
    // placeholder
    const fvec2 DOOR = {64 * 6, 0};
    const fvec2 KEY = {64 * 8, 64 * 6};
    
    if (GET_KEYPRESS(space_pressed)) {
        g_scene.startup_state = 4;
        ring->mark_for_removal = true;
        g_scene.playing = true;
    }
    
    switch (g_scene.startup_state) {
        
        case STATE_START: {
            ring_static = make_starring(DOOR);
            
            g_scene.startup_state = STATE_SHOW_KEY;
        } break;
        
        case STATE_SHOW_KEY: {
            
            ring->update_animation(dt);
            draw(ring);
            
            if (anim_time < anim_dur) {
                ring->position = lerp2(DOOR, KEY, (anim_time/anim_dur));
                anim_time = fclamp(0.f, anim_dur, anim_time + dt);
            } else {
                g_scene.startup_state = STATE_SHOW_PLAYER;
                anim_time = 0.f;
            }
            
        } break;
        
        case STATE_SHOW_PLAYER: {
            
            if (anim_time < anim_dur) {
                ring->position = lerp2(DOOR, player->position, (anim_time/anim_dur));
                anim_time = fclamp(0.f, anim_dur, anim_time + dt);
            } else {
                g_scene.startup_state = 4;
                ring->mark_for_removal = true;
                g_scene.playing = true;
            }
            
            if (anim_time < anim_dur) {
                ring->update_animation(dt);
                float radius = ((anim_dur - anim_time) / anim_dur) * 128.f;
                radius = MAX(radius, 32.f);
                float phase = ((anim_dur - anim_time) / anim_dur) * 90.f;
                
                for (int i = 0; i < 16; ++i) {
                    Sprite tmp = *ring;
                    
                    float angle = (360.f / 16.f) * i;
                    angle += phase;
                    tmp.position += fvec2{cosf(angle * D2R), sinf(angle * D2R)} * radius;
                    
                    draw(&tmp);
                }
            }
            
        } break;
    }
    
    
}

internal void
scene_update(InputState* istate, float dt) {
    
    scene_draw_tilemap();
    
    for (int i = 0; i < g_scene.pickup_list.size(); ++i) {
        draw(&g_scene.pickup_list[i]);
    }
    
    List_Sprite& l = g_scene.spritelist;
    
    // remove marked elements
    auto it = g_scene.spritelist.begin();
    while (it != g_scene.spritelist.end())
    {
        Sprite& spref = (*it);
        if (spref.mark_for_removal)
        {
            it = g_scene.spritelist.erase(it);
        }
        else
        {
            it++;
        }
    }
    
    for (int i = 0; i < l.size(); ++i) {
        Sprite* spref = &l[i];
        spref->update_animation(dt);
        
        switch(spref->entity.type) {
            case ePlayer:{
                ePlayer_update(spref, istate, dt);
            }break;
            
            case eGoblin:{
                eGoblin_update(spref, istate, dt);
            } break;
            
            case eEffect:{
                if (!spref->animation_playing) {
                    spref->mark_for_removal = true;
                }
            }break;
            
            case eDFireball:{
                eDFireball_update(spref, istate, dt);
            }break;
            
            case eGhost:{
                eGhost_update(spref, istate,dt);
            }break;
            
            default:
            break;
        }
        
        
#ifndef NDEBUG
        // Draw the Bounding box sprite
        AABox box = spref->get_transformed_AABox();
        gl_slow_tilemap_draw(
                             &GET_TILEMAP_TEXTURE(test),
                             {box.min_x, box.min_y},
                             {box.max_x - box.min_x, box.max_y - box.min_y},
                             0,5 * 5 + 1,
                             false, false,
                             NRGBA{1.f, 0, 1.f, 0.7f});
#endif
        
        draw(&l[i]);
        
        
    }
    
    
}
