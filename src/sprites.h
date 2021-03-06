/* date = May 13th 2020 10:07 am */
#ifndef SPRITES_H
#define SPRITES_H

struct Sprite {
    GLTilemapTexture const * tilemap = 0;
    fvec2 size = {64, 64};
    fvec2 position = {0,0};
    float rotation = 0.f;
    AABox collision_box = {0,0,64,64};
    ivec2 mirror = {0, 0};
    fvec2 velocity = {0,0};
    b32 enabled = true;
    b32 is_on_air = false;
    b32 mark_for_removal = false;                 // set to true to remove the sprite
    
    // Animation stuff
    b32 animation_playing = true;
    i32 current_frame = 0;
    u32 current_animation = 0;
    float time_accumulator = 0.f;
    Animation* animation_set;
    
    Entity entity;
    
    inline AABox get_transformed_AABox() const {
        return this->collision_box.translate(this->position);
    }
    
    void collide_aabb(AABox* target) {
        fail_unless(target, "");
        
        AABox this_collision = this->get_transformed_AABox();
        
        fvec2 diff;
        b32 collided = aabb_minkowski(&this_collision, target, &diff);
        b32 collided_on_bottom = false;
        if (collided) {
            this->position = this->position - (diff);
            
            // If we are moving up and diff moved us in the Y dir,
            // then negate the collision.
            // (fixes bouncing when hitting corner of a tile)
            if (this->velocity.y < 0 && iabs(diff.y) < 5) {
                this->position.y += diff.y;
                goto finished;
            }
            
            // NOTE(mdodis): if on top?
            if (this_collision.min_y < target->min_y) {
                collided_on_bottom = true;
                if (iabs(diff.y) > 0) {
                    this->is_on_air = false;
                    this->velocity.y = 0;
                }
            }
        }
        
        finished:
        if (!collided_on_bottom && this->velocity.y != 0)
            this->is_on_air = true;
        
    }
    
    
    void update_animation(float dt) {
        
        if (!this->animation_set) return;
        // get animation ref
        Animation* anim_ref;
        anim_ref = &this->animation_set[this->current_animation];
        if (anim_ref->size == 0 || !this->animation_playing)
            return;
        
        // increase time by dt
        if (this->animation_playing) {
            
            this->time_accumulator += dt;
            if (this->time_accumulator >= anim_ref->duration) {
                this->current_frame++;
                if (this->current_frame >= anim_ref->size ) {
                    // if its looping
                    if (anim_ref->loop) {
                        this->current_frame = 0;
                    }else {
                        this->animation_playing = false;
                        this->current_frame--;
                    }
                }
                
                this->time_accumulator = 0.f;
            }
        }
        
    }
    
#define SET_ANIMATION(spr, c, n) (spr)->set_animation_index(GET_CHAR_ANIMENUM(c, n))
    void set_animation_index(u32 anim_idx) {
        fail_unless(this->current_animation >= 0, "set_animation_index, invalid animation index!");
        if (this->current_animation != anim_idx) {
            this->animation_playing = true;
            this->current_animation = anim_idx;
            this->current_frame = 0;
            this->time_accumulator = 0;
        }
        
    }
    
    void move_and_collide(float dt,
                          const float GRAVITY,
                          const float MAX_YSPEED,
                          const float JUMP_STRENGTH,
                          float XSPEED,
                          b32 damage_tiles = false);
    
    void collide_sprite(float dt);
    
    inline float direction() {
        return this->mirror.x ? 1 : -1;
    }
    
    b32 jump(i32 strength)
    {
        
        if (!this->is_on_air)
        {
            this->velocity.y = -strength;
            this->is_on_air = true;
            return true;
        }
        return false;
        
    }
};

inline internal EnemyType get_enemy_type(Sprite *spr) {
    return spr->entity.params[0].as_etype;
}

inline internal bool is_killable_enemy(Sprite *s) {
    EnemyType et = get_enemy_type(s);
    return ((s->entity.type == ET_Enemy) &&
            (et != MT_DragonFire));
}

UPDATE_ENTITY_FUNC(Player_update);
UPDATE_ENTITY_FUNC(Goblin_update);
UPDATE_ENTITY_FUNC(DFireball_update);
UPDATE_ENTITY_FUNC(Ghost_update);
UPDATE_ENTITY_FUNC(BlueFlame_update);
UPDATE_ENTITY_FUNC(DemonHead_update);
UPDATE_ENTITY_FUNC(KMirror_update);
UPDATE_ENTITY_FUNC(PanelMonster_update);
UPDATE_ENTITY_FUNC(PanelMonsterFlame_update);
UPDATE_ENTITY_FUNC(Wyvern_update);
UPDATE_ENTITY_FUNC(Fairie_update);
UPDATE_ENTITY_FUNC(Dragon_update);
UPDATE_ENTITY_FUNC(DragonFire_update);
UPDATE_ENTITY_FUNC(SparkBall_update);
UPDATE_ENTITY_FUNC(Gargoyle_update);

internal void BlueFlame_cast(Sprite* flame);
internal void player_pickup(Sprite *player, Sprite *pickup);
#endif //SPRITES_H
