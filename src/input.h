/*OSK input.h
Input is defined like everything in resources.h; a #define
where you choose to sepcify a KEYPRESS or KEYDOWN action.
Implementation is left up to the corresponding platform, and
as always configuration is done at compile time.*/
#define KEYMAP \
/*       NAME        X11 key   Win32 key    */\
KEYDOWN (move_right    , XK_Right       , VK_RIGHT    ) \
KEYDOWN (move_left     , XK_Left        , VK_LEFT     ) \
KEYDOWN (move_down     , XK_Down        , VK_DOWN     ) \
KEYPRESS(move_up       , XK_Up          , VK_UP       ) \
KEYPRESS(move_up_menu  , XK_Up          , VK_UP       ) \
KEYPRESS(move_down_menu, XK_Down        , VK_DOWN     ) \
KEYDOWN (fmove_right   , XK_D           , VK_RIGHT    ) \
KEYDOWN (fmove_left    , XK_A           , VK_LEFT     ) \
KEYDOWN (fmove_down    , XK_S           , VK_DOWN     ) \
KEYDOWN (fmove_up      , XK_W           , VK_UP       ) \
KEYPRESS(space_pressed , XK_space       , VK_SPACE    ) \
KEYPRESS(restart       , XK_R           , 'R'         ) \
KEYPRESS(fireball      , XK_X           , 'X'         ) \
KEYPRESS(cast          , XK_Control_L   , VK_CONTROL  ) \
KEYPRESS(sound_down    , XK_9           , '9'         ) \
KEYPRESS(sound_up      , XK_0           , '0'         ) \
KEYPRESS(go_fullscreen , XK_F11         , VK_F11      ) \
KEYPRESS(go_menu       , XK_Escape      , VK_ESCAPE   ) \


#define KEYDOWN(key, ...) b32 key = false;
#define KEYPRESS(key, ...) b32 key[2] = {};
struct InputState
{
    KEYMAP
};
#undef KEYDOWN
#undef KEYPRESS

#define GET_KEYPRESS(name) (b32)(g_input_state.name[1])
#define GET_KEYDOWN(name) (b32)(g_input_state.name)

#if defined(OSK_PLATFORM_X11)

b32 x11_get_key_state(i32 key);

#elif defined(OSK_PLATFORM_WIN32)

b32 win32_get_key_state(i32 key);

#else
#error "No get key state function for compatible platform"
#endif

global InputState g_input_state;
