#include "plib/gnw/dxinput.h"
#include <hal/debug.h>
#include <SDL.h>
#include "xboxkrnl/xboxkrnl.h"

namespace fallout {

static SDL_Joystick* gController = nullptr;
static bool gControllerConnected = false;

static int gMouseWheelDeltaX = 0;
static int gMouseWheelDeltaY = 0;

// Smoothed velocity accumulators
static float velocityX = 0.0f;
static float velocityY = 0.0f;

// Settings
constexpr int DEADZONE = 8000;
constexpr float SENSITIVITY = 0.0075f;
constexpr float FRICTION = 0.75f;
constexpr float MAX_VELOCITY = 12.0f;

static bool dxinput_mouse_init()
{
    // DbgPrint("dxinput_mouse_init: opening controller as joystick\n");

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) != 0) {
        // DbgPrint("dxinput_mouse_init: SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_JoystickEventState(SDL_ENABLE);

    if (SDL_NumJoysticks() > 0) {
        gController = SDL_JoystickOpen(0);
        if (gController) {
            gControllerConnected = true;
            // DbgPrint("dxinput_mouse_init: controller connected and opened\n");
        } else {
            // DbgPrint("dxinput_mouse_init: SDL_JoystickOpen failed: %s\n", SDL_GetError());
            return false;
        }
    } else {
        // DbgPrint("dxinput_mouse_init: no joystick detected\n");
        return false;
    }

    return true;
}

static void dxinput_mouse_exit()
{
    // DbgPrint("dxinput_mouse_exit called\n");
    if (gController) {
        SDL_JoystickClose(gController);
        gController = nullptr;
    }
    gControllerConnected = false;
}

static bool dxinput_keyboard_init() {
    // DbgPrint("dxinput_keyboard_init called\n");
    return true;
}

static void dxinput_keyboard_exit() {
    // DbgPrint("dxinput_keyboard_exit called\n");
}

bool dxinput_init()
{
    // DbgPrint("dxinput_init: initializing input subsystems\n");

    if (!dxinput_mouse_init()) {
        // DbgPrint("dxinput_init: dxinput_mouse_init failed\n");
        return false;
    }

    if (!dxinput_keyboard_init()) {
        // DbgPrint("dxinput_init: dxinput_keyboard_init failed\n");
        dxinput_mouse_exit();
        return false;
    }

    return true;
}

void dxinput_exit()
{
    // DbgPrint("dxinput_exit: shutting down input subsystems\n");
    dxinput_mouse_exit();
    dxinput_keyboard_exit();
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);
}

bool dxinput_get_mouse_state(MouseData* mouseState)
{
    SDL_JoystickUpdate();

    if (!gControllerConnected || !gController) {
        // DbgPrint("dxinput_get_mouse_state: no controller connected\n");
        return false;
    }

    int16_t axisX = SDL_JoystickGetAxis(gController, 0); // Left stick X
    int16_t axisY = SDL_JoystickGetAxis(gController, 1); // Left stick Y

    float dx = 0.0f;
    float dy = 0.0f;

    if (abs(axisX) > DEADZONE) {
        dx = axisX * SENSITIVITY;
        if (dx > MAX_VELOCITY) dx = MAX_VELOCITY;
        if (dx < -MAX_VELOCITY) dx = -MAX_VELOCITY;
    }

    if (abs(axisY) > DEADZONE) {
        dy = axisY * SENSITIVITY;
        if (dy > MAX_VELOCITY) dy = MAX_VELOCITY;
        if (dy < -MAX_VELOCITY) dy = -MAX_VELOCITY;
    }

    // Output relative deltas just like SDL_GetRelativeMouseState
    mouseState->x = static_cast<int>(dx);
    mouseState->y = static_cast<int>(dy);

    // Map controller buttons to mouse buttons
    mouseState->buttons[0] = SDL_JoystickGetButton(gController, 0); // A button → Left click
    mouseState->buttons[1] = SDL_JoystickGetButton(gController, 1); // B button → Right click

    // Pass through scroll wheel state
    mouseState->wheelX = gMouseWheelDeltaX;
    mouseState->wheelY = gMouseWheelDeltaY;

    // Reset wheel deltas after reporting
    gMouseWheelDeltaX = 0;
    gMouseWheelDeltaY = 0;

    SDL_WarpMouseInWindow(nullptr, mouseState->x, mouseState->y);

    return true;
}



bool dxinput_acquire_mouse() { return true; }
bool dxinput_unacquire_mouse() { return true; }
bool dxinput_acquire_keyboard() { return true; }
bool dxinput_unacquire_keyboard() { return true; }

bool dxinput_flush_keyboard_buffer() {
    SDL_FlushEvents(SDL_KEYDOWN, SDL_TEXTINPUT);
    return true;
}

bool dxinput_read_keyboard_buffer(KeyboardData* keyboardData) {
    // DbgPrint("dxinput_read_keyboard_buffer called\n");
    return true;
}

void handleMouseEvent(SDL_Event* event)
{
    if (event->type == SDL_MOUSEWHEEL) {
        gMouseWheelDeltaX += event->wheel.x;
        gMouseWheelDeltaY += event->wheel.y;
    }
}

} // namespace fallout


