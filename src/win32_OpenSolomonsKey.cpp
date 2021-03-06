/* win32_OpenSolomonsKey.cpp
 Michael Dodis, All rights reserved.
 */
#define OSK_PLATFORM_WIN32
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/GL.h>

#define OSK_PLATFORM_WIN32
#include "OpenSolomonsKey.h"
#define OSK_CLASS_NAME "OSK Class"

#include "gl_funcs.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// TIMING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
global LARGE_INTEGER g_performace_frequency;
global LARGE_INTEGER g_perf_last, g_perf_now = {};
struct Timer {
    LARGE_INTEGER time_last;
    
    double get_elapsed_secs(b32 should_reset = false) {
        LARGE_INTEGER now;
        float delta;
        
        QueryPerformanceCounter(&now);
        
        delta = g_performace_frequency.QuadPart != 0
            ? (float(now.QuadPart - time_last.QuadPart) * 1000.f) / g_performace_frequency.QuadPart
            : 0.f;
        
        assert((delta >= 0.f));
        
        if (should_reset) time_last = now;
        
        return (delta / 1000.f);
    }
    
    void reset(){QueryPerformanceCounter(&time_last);}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// CONTEXT CREATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE(mdodis): Thank you https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
// Modern Opengl is a bitch to init on Windows
typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,const int *attribList);
typedef const char* PFNwglGetExtensionsStringARB(HDC hdc);
typedef BOOL PFNwglSwapIntervalEXT(int interval);
typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;
PFNwglGetExtensionsStringARB* wglGetExtensionsStringARB;
PFNwglSwapIntervalEXT* wglSwapIntervalEXT;
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

b32 wgl_is_extension_supported(char* extname,  char* ext_string) {
    char* csearch = extname;
    char* chay = ext_string;
    
    while(*csearch && *chay)
    {
        if (!(*chay)) return (!(*csearch));
        
        if (*csearch == *chay)
            csearch++;
        else
            csearch = extname;
        
        chay++;
    }
    
    return (!(*csearch) && !(*csearch));
    
}

// Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
// We use a dummy window because you can only set the pixel format for a window once. For the
// real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
// that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
// have a context.
internal void
win32_init_gl_extensions() {
    WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = "Dummy_WGL_djuasiodwa",
    };
    
    if (!RegisterClassA(&window_class))
        inform("Failed to register dummy OpenGL window.");
    
    HWND dummy_window = CreateWindowExA(
                                        0,
                                        window_class.lpszClassName,
                                        "Dummy OpenGL Window",
                                        0,
                                        CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                                        0,0,
                                        window_class.hInstance,
                                        0);
    
    if (!dummy_window)
        inform("Failed to create dummy OpenGL window.");
    
    HDC dummy_dc = GetDC(dummy_window);
    
    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .cDepthBits = 24,
        .cStencilBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
    };
    
    int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format)
        inform("Failed to find a suitable pixel format.");
    
    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd))
        inform("Failed to set the pixel format.");
    
    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context)
        inform("Failed to create a dummy OpenGL rendering context.");
    
    if (!wglMakeCurrent(dummy_dc, dummy_context))
        inform("Failed to activate dummy OpenGL rendering context.");
    
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress("wglChoosePixelFormatARB");
    wglGetExtensionsStringARB = (PFNwglGetExtensionsStringARB*)wglGetProcAddress("wglGetExtensionsStringARB");
    
    if (wglGetExtensionsStringARB) {
        char* ext_string = (char*)wglGetExtensionsStringARB(dummy_dc);
        printf("WGL extensions: %s\n", ext_string);
        
        if (wgl_is_extension_supported("WGL_EXT_swap_control", ext_string)) {
            inform("wglSwapInterval is supported!");
            wglSwapIntervalEXT = (PFNwglSwapIntervalEXT*)wglGetProcAddress("wglSwapIntervalEXT");
            wglSwapIntervalEXT(1);
            
        }
    }
    
    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
}


internal void
win32_init_gl(HDC real_dc) {
    win32_init_gl_extensions();
    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };
    
    int pixel_format;
    UINT num_formats;
    wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats) {
        inform("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
        inform("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC mgl_ctx = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
    if (!mgl_ctx) {
        inform("Failed to create OpenGL 3.3 context.");
    }
    
    if (!wglMakeCurrent(real_dc, mgl_ctx)) {
        inform("Failed to activate OpenGL 3.3 rendering context.");
    }
    
    
    fail_unless(wglSwapIntervalEXT, "wglSwapIntervalEXT");
    fail_unless(wglSwapIntervalEXT(1), "swap interval set failed!");
    
    gl_load();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// AUDIO
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <mmsystem.h> // WIN32_LEAN_AND_MEAN probably doesn't load this
#include <dsound.h>
global LPDIRECTSOUNDBUFFER g_secondary_buffer;
global u64 g_sample_counter = 0;
HANDLE g_dsound_sem;

// Function defs we *need* from audio.cpp
internal void audio_update(const InputState* const istate, u64 samples_to_write);
internal void audio_update_all_sounds();
internal DWORD dsound_cb_audio(void *unused);

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef DIRECT_SOUND_CREATE(FNDirectSoundCreate);

internal void win32_dsound_init(HWND window_handle) {
    // load dsound
    HMODULE dsound_lib = LoadLibraryA("dsound.dll");
    fail_unless(dsound_lib, "dsound failed to load");
    
    FNDirectSoundCreate* dsound_create = (FNDirectSoundCreate*)GetProcAddress(dsound_lib, "DirectSoundCreate");
    fail_unless(dsound_create, "");
    
    // Create the dsound object
    LPDIRECTSOUND dsound;
    if (!SUCCEEDED(dsound_create(0, &dsound, 0))) _exit_with_message("dsound_create");
    if (!SUCCEEDED(dsound->SetCooperativeLevel(window_handle, DSSCL_PRIORITY))) _exit_with_message("SetCooperativeLevel");
    
    WAVEFORMATEX wave_fmt = {};
    wave_fmt.wFormatTag = WAVE_FORMAT_PCM;
    wave_fmt.nChannels = AUDIO_CHANNELS;
    wave_fmt.nSamplesPerSec = AUDIO_SAMPLERATE;
    wave_fmt.wBitsPerSample = AUDIO_BPS;
    wave_fmt.nBlockAlign = (wave_fmt.nChannels * wave_fmt.wBitsPerSample) / 8;
    wave_fmt.nAvgBytesPerSec = wave_fmt.nSamplesPerSec * wave_fmt.nBlockAlign;
    wave_fmt.cbSize = 0;
    
    LPDIRECTSOUNDBUFFER primary_buffer;
    {
        // create primary buffer; It's useless but we have to
        DSBUFFERDESC buffer_desc = {};
        buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        buffer_desc.dwSize = sizeof(DSBUFFERDESC);
        
        if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &primary_buffer, 0)))
            _exit_with_message("CreateSoundBuffer");
        
        if (!SUCCEEDED(primary_buffer->SetFormat(&wave_fmt))) _exit_with_message("SetFormat");
    }
    
    // create secondary buffer (we're using this one)
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = DSBCAPS_GLOBALFOCUS;
    buffer_desc.dwBufferBytes = AUDIO_BUFFER_SIZE;
    buffer_desc.lpwfxFormat = &wave_fmt;
    if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &g_secondary_buffer, 0)))
        _exit_with_message("CreateSoundBuffer secondary");
    
    // Start playing right away
    HRESULT err = g_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
    
    DWORD audio_thread_id;
    HANDLE audio_thread = CreateThread( 0,0, dsound_cb_audio, 0, 0, &audio_thread_id);
    
    g_dsound_sem = CreateSemaphoreA(0,0,1,0);
    
}

internal void
win32_dsound_get_bytes_to_output(DWORD* byte_to_lock, DWORD* bytes_to_write) {
    const float target_framerate = .0166f;
    DWORD play_cursor, write_cursor;
    if (!SUCCEEDED(g_secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
        _exit_with_message("GetCurrentPosition failed!");
    
    // how many bytes _should_ be written
    DWORD expected_bytes_per_tick =(DWORD)( float(AUDIO_SAMPLERATE * AUDIO_BYTESPERSAMPLE) * target_framerate);
    expected_bytes_per_tick -= expected_bytes_per_tick % AUDIO_BYTESPERSAMPLE;
    
    // number of extra bytes to write, if we delay a bit
    DWORD safety_bytes = expected_bytes_per_tick * 5;
    
    // where the play cursor is gonna be
    DWORD expected_boundary = play_cursor + expected_bytes_per_tick;
    
    DWORD safe_write_pos = write_cursor;
    if (safe_write_pos < play_cursor)
        safe_write_pos += AUDIO_BUFFER_SIZE; // wrap it around if we're past the write_cursor
    else safe_write_pos += safety_bytes;
    // now we know where to write _to_
    
    DWORD target_cursor;
    if (safe_write_pos < expected_boundary)
        target_cursor = expected_boundary + expected_bytes_per_tick;
    else
        target_cursor = write_cursor + expected_bytes_per_tick + safety_bytes;
    
    target_cursor %= AUDIO_BUFFER_SIZE;
    *byte_to_lock = (g_sample_counter * AUDIO_BYTESPERSAMPLE) % AUDIO_BUFFER_SIZE;
    
    if (*byte_to_lock > target_cursor)
        *bytes_to_write = (AUDIO_BUFFER_SIZE -  *byte_to_lock) + target_cursor;
    else
        *bytes_to_write = target_cursor - *byte_to_lock;
    
}

internal void win32_dsound_copy_to_sound_buffer(DWORD byte_to_lock, DWORD bytes_to_write) {
    
    void* region1; DWORD region1_size;
    void* region2; DWORD region2_size;
    
    if (bytes_to_write == 0) return;
    
    HRESULT error = g_secondary_buffer->Lock( byte_to_lock, bytes_to_write, &region1, &region1_size, &region2, &region2_size, 0);
    
    assert(region1_size + region2_size == bytes_to_write);
    assert(bytes_to_write <= AUDIO_BUFFER_SIZE);
    if (!SUCCEEDED(error)) {
        printf(" \t%d %d\n %x\n", byte_to_lock, bytes_to_write,  error);
        _exit_with_message("Lock failed");
    }
    
    DWORD region1_samples = region1_size / AUDIO_BYTESPERSAMPLE;
    DWORD region2_samples = region2_size / AUDIO_BYTESPERSAMPLE;
    
    i16* sample_in = (i16*)g_audio.buffer;
    i16* sample_out = (i16*) region1;
    for (DWORD sample_idx = 0; sample_idx < region1_samples; sample_idx++) {
        *sample_out++ = *sample_in++;
        *sample_out++ = *sample_in++;
        g_sample_counter++;
    }
    
    sample_out = (i16*) region2;
    for (DWORD sample_idx = 0; sample_idx < region2_samples; sample_idx++) {
        *sample_out++ = *sample_in++;
        *sample_out++ = *sample_in++;
        g_sample_counter++;
    }
    
    fail_unless(SUCCEEDED(g_secondary_buffer->Unlock(region1, region1_size, region2, region2_size)), "");
}

internal DWORD dsound_cb_audio(void *unused) {
    
    i16 *buffer = (i16*)g_audio.buffer;
    
    for(;;) {
        DWORD byte_to_lock, bytes_to_write;
        win32_dsound_get_bytes_to_output(&byte_to_lock, &bytes_to_write);
        
        u64 samples_to_write = bytes_to_write / AUDIO_BYTESPERSAMPLE;
        if (samples_to_write > 0) {
            
            audio_update_all_sounds();
            audio_update(0, samples_to_write);
            
            win32_dsound_copy_to_sound_buffer(byte_to_lock, bytes_to_write);
            
        } else {
            // Wait for our target framerate in ms
            WaitForSingleObjectEx(g_dsound_sem, u64(0.0166f * 1000), FALSE);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// WINDOWING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(mdodis): Win32's DefWindowProc function takes the liberty of
// blocking the whole thread whilst moving the window (via the title bar),
// so we us these two bools to check for that, and NOT update.
// Done using WM_ENTERSIZEMOVE / WM_EXITSIZEMOVE
global b32 is_currently_moving_or_resizing = false;
global b32 was_previously_moving_or_resizing = false;

global HWND     g_wind;
global bool     g_running = true;

Timer g_window_timer;

internal void win32_update_and_render(HDC dc) {
    float delta_time = g_window_timer.get_elapsed_secs(true);
    
    if (was_previously_moving_or_resizing && !is_currently_moving_or_resizing) {
        delta_time = 0.f;
        was_previously_moving_or_resizing = false;
    }
    
    if (delta_time >= 0.9f) delta_time = 0.9f;
    
    //audio_update_all_sounds();
    //audio_update(&g_input_state, samples_to_write);
    
    cb_render(g_input_state, 0, delta_time);
    
    // Wake up the audio thread if something needs to be written _now_
    {
        DWORD byte_to_lock, bytes_to_write;
        win32_dsound_get_bytes_to_output(&byte_to_lock, &bytes_to_write);
        
        u64 samples_to_write = bytes_to_write / AUDIO_BYTESPERSAMPLE;
        if (samples_to_write > 0)
            ReleaseSemaphore(g_dsound_sem, 1, 0);
    }
    
    SwapBuffers(dc);
}

const LONG NORMAL_STYLE = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

internal void toggle_fullscreen(HWND hwnd) {
    
    if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_POPUP) {
        // TODO(mdodis): maybe remember previous configuration?
        SetWindowLongPtrA(hwnd, GWL_STYLE, NORMAL_STYLE);
        SetWindowPos(hwnd, 0, 0, 0, 640, 480, SWP_FRAMECHANGED);
        
    } else {
        int w = GetSystemMetrics(SM_CXSCREEN);
        int h = GetSystemMetrics(SM_CYSCREEN);
        SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
    }
}

internal LRESULT CALLBACK
win32_windproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;
    
    switch(msg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            g_running = false;
        }break;
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        } break;
        
        case WM_SIZE: {
            g_wind_width = LOWORD(lparam);
            g_wind_height =  HIWORD(lparam);
            glViewport(0,0,g_wind_width, g_wind_height);
            
            cb_resize();
            
            OutputDebugStringA("WM_SIZE\n");
            PostMessage(hwnd, WM_PAINT, 0, 0);
        } break;
        
        case WM_KEYDOWN: {
            if (wparam == VK_F11) toggle_fullscreen(hwnd);
        }break;
        
        case WM_ENTERSIZEMOVE:
        case WM_EXITSIZEMOVE: {
            was_previously_moving_or_resizing = is_currently_moving_or_resizing;
            is_currently_moving_or_resizing = msg == WM_ENTERSIZEMOVE;
        } break;
        
        default: {
            result = DefWindowProc(hwnd, msg, wparam, lparam);
        } break;
    }
    return result;
}

internal void
win32_init(HINSTANCE hInstance) {
    // create the actual window
    WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = win32_windproc,
        .hInstance = hInstance,
        .hCursor = LoadCursor(0, IDC_ARROW),
        .hbrBackground = 0,
        .lpszClassName = OSK_CLASS_NAME,
    };
    fail_unless(RegisterClassA(&window_class), "Failed to register class");
    
    RECT rect = {
        .right = 640,
        .bottom = 480
    };
    // DWORD window_style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rect, window_style, false);
    
    g_wind = CreateWindowExA(0, OSK_CLASS_NAME, "Open Solomon's Key", window_style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, 0, 0, hInstance, 0);
    fail_unless(g_wind, "Failed to create window");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// INPUT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
b32 win32_get_key_state(i32 key) {return GetAsyncKeyState(key);}

// Auto-generate the list of keys to update
internal void win32_update_all_keys() {
#define KEYDOWN(name, _X, keysym) g_input_state.name = win32_get_key_state(keysym);
#define KEYPRESS(name, _X, keysym) { \
b32 now = win32_get_key_state(keysym); \
g_input_state.name[1] = (now && !g_input_state.name[0]); \
g_input_state.name[0] = g_input_state.name[1] || now; \
    \
} \
    
    KEYMAP
        
#undef KEYDOWN
#undef KEYPRESS
}

#include <vector>
std::vector<char *> list_maps() {
    std::vector<char *> result;
    WIN32_FIND_DATAA find_data;
    HANDLE find_handle = FindFirstFileA("level_*.osk", &find_data);
    
    if (find_handle != INVALID_HANDLE_VALUE) {
        result.push_back(strdup(find_data.cFileName));
    }
    
    while(FindNextFileA(find_handle, &find_data)) {
        result.push_back(strdup(find_data.cFileName));
    }
    
    return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nShowCmd) {
#if defined(OSK_WIN32_CONSOLE)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif
    win32_init(hInstance);
    HDC dc = GetDC(g_wind);
    
    win32_init_gl(dc);
    cb_init();
    
    win32_dsound_init(g_wind);
    
    ShowWindow(g_wind, SW_SHOW);
    UpdateWindow(g_wind);
    
    QueryPerformanceFrequency(&g_performace_frequency);
    
    g_window_timer.reset();
    while (g_running) {
        MSG message;
        while (PeekMessage(&message, g_wind, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        win32_update_all_keys();
        
        win32_update_and_render(dc);
        
    }
    
    return 0;
}

#undef OSK_CLASS_NAME



#include "OpenSolomonsKey.cpp"
