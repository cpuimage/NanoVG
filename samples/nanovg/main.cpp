//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#define NANOVG_GLES3_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"
#include "demo.h"
#include "perf.h"
#include "es_util.h"
#include <stdint.h>

#if   defined(__APPLE__)
# include <mach/mach_time.h>
#elif defined(_WIN32)

# include <windows.h>

#else // __linux

# include <time.h>

# ifndef  CLOCK_MONOTONIC //_RAW
#  define CLOCK_MONOTONIC CLOCK_REALTIME
# endif
#endif

static
uint64_t nanotimer() {
    static int ever = 0;
#if defined(__APPLE__)
    static mach_timebase_info_data_t frequency;
    if (!ever) {
        if (mach_timebase_info(&frequency) != KERN_SUCCESS) {
            return 0;
        }
        ever = 1;
    }
    return  (mach_absolute_time() * frequency.numer / frequency.denom);
#elif defined(_WIN32)
    static LARGE_INTEGER frequency;
    if (!ever) {
        QueryPerformanceFrequency(&frequency);
        ever = 1;
    }
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return (t.QuadPart * (uint64_t) 1e9) / frequency.QuadPart;
#else // __linux
    struct timespec t;
    if (!ever) {
        if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
            return 0;
        }
        ever = 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec * (uint64_t) 1e9) + t.tv_nsec;
#endif
}


static double now() {
    static uint64_t epoch = 0;
    if (!epoch) {
        epoch = nanotimer();
    }
    return (nanotimer() - epoch) / 1e9;
};

double calcElapsed(double start, double end) {
    double took = -start;
    return took + end;
}

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

int blowup = 0;
int screenshot = 0;
int premult = 0;
DemoData data;
NVGcontext *vg = NULL;
PerfGraph fps;
double prevt = 0;

typedef struct {
    // title
    const char *title;
    HWND title_hwnd;
    // global variables
} UserData;


int Init(ESContext *esContext) {
    UserData *userData = (UserData *) (esContext->userData);
    initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
    vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
        printf("Could not init nanovg.\n");
        return FALSE;
    }
    if (loadDemoData(vg, &data) == -1)
        return FALSE;
    return TRUE;
}


void Draw(ESContext *esContext) {
    UserData *userData = (UserData *) (esContext->userData);
    double mx, my, t, dt;
    int fbWidth = esContext->width, fbHeight = esContext->height;
    t = now();
    dt = t - prevt;
    prevt = t;
    updateGraph(&fps, dt);
    RECT area;
    GetClientRect(userData->title_hwnd, &area);
    int winWidth = area.right;
    int winHeight = area.bottom;
//  Calculate pixel ration for hi-dpi devices.
    float pxRatio = (float) fbWidth / (float) winWidth;
    POINT pos;
    if (GetCursorPos(&pos)) {
        ScreenToClient(userData->title_hwnd, &pos);
        mx = pos.x;
        my = pos.y;
    }
    // Update and render
    glViewport(0, 0, winWidth, winHeight);
    if (premult)
        glClearColor(0, 0, 0, 0);
    else
        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

    renderDemo(vg, mx, my, winWidth, winHeight, t, blowup, &data);
    renderGraph(vg, 5, 5, &fps);

    nvgEndFrame(vg);

    glEnable(GL_DEPTH_TEST);

    if (screenshot) {
        screenshot = 0;
        saveScreenShot(fbWidth, fbHeight, premult, "dump.png");
    }
}


void KeyProc(ESContext *esContext, unsigned char key, int x, int y) {
    UserData *userData = (UserData *) (esContext->userData);
#define  KEY_P                  80
#define  KEY_S                  83
#define  KEY_SPACE              32
    if (esContext->keyboard.toggle[KEY_SPACE] && esContext->keyboard.state[KEY_SPACE])
        blowup = !blowup;
    if (esContext->keyboard.toggle[KEY_S] && esContext->keyboard.state[KEY_S])
        screenshot = 1;
    if (esContext->keyboard.toggle[KEY_P] && esContext->keyboard.state[KEY_P])
        premult = !premult;
    char str[255] = {0};
    sprintf(str, "%s: \t key:%d \t x:%d \t y:%d", userData->title, key, x, y);
    SetWindowText(userData->title_hwnd, str);
}

void Update(ESContext *esContext, float detlaTime) {
    UserData *userData = (UserData *) (esContext->userData);
    if ((esContext->resolution.x != 0 && esContext->resolution.y != 0) && esContext->resolution.x != esContext->width &&
        esContext->resolution.y != esContext->height) {
        glViewport(0, 0, esContext->resolution.x, esContext->resolution.y);
    }
}

void Shutdown(ESContext *esContext) {
    UserData *userData = (UserData *) (esContext->userData);
    freeDemoData(vg, &data);
    nvgDeleteGLES3(vg);
}

int esMain(ESContext *esContext, int argc, char *argv[]) {
    esContext->userData = malloc(sizeof(UserData));
    const char *title = argv[0];
    esCreateWindow(esContext, title, 1000, 600, ES_WINDOW_RGB);
    UserData *userData = (UserData *) (esContext->userData);
    userData->title = title;
    userData->title_hwnd = FindWindow(NULL, title);
    if (!Init(esContext)) {
        return GL_FALSE;
    }
    glViewport(0, 0, 1000, 600);
    esRegisterKeyFunc(esContext, KeyProc);
    esRegisterDrawFunc(esContext, Draw);
    esRegisterUpdateFunc(esContext, Update);
    esRegisterShutdownFunc(esContext, Shutdown);
    return GL_TRUE;
}
