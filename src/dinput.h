#ifndef DINPUT_H
#define DINPUT_H

#include <SDL.h>

namespace fallout {

#ifdef NXDK
typedef struct ControllerState {
    float analogX;
    float analogY;
    float rightStickX;  // Right stick X
    float rightStickY;  // Right stick Y
    bool buttonA;
    bool buttonB;
} ControllerState;

typedef struct ControllerKeyMapping {
    SDL_GameControllerButton button;
    SDL_Scancode scancode;
} ControllerKeyMapping;

// Default controller button to keyboard mappings
extern const ControllerKeyMapping CONTROLLER_KEY_MAPPINGS[];
extern const int CONTROLLER_KEY_MAPPING_COUNT;
#endif

typedef struct MouseData {
    int x;
    int y;
    unsigned char buttons[2];
    int wheelX;
    int wheelY;
} MouseData;

typedef struct KeyboardData {
    int key;
    char down;
} KeyboardData;

bool directInputInit();
void directInputFree();
bool mouseDeviceAcquire();
bool mouseDeviceUnacquire();
bool mouseDeviceGetData(MouseData* mouseData);
bool keyboardDeviceAcquire();
bool keyboardDeviceUnacquire();
bool keyboardDeviceReset();
bool keyboardDeviceGetData(KeyboardData* keyboardData);
bool mouseDeviceInit();
void mouseDeviceFree();
bool keyboardDeviceInit();
void keyboardDeviceFree();

void handleMouseEvent(SDL_Event* event);
void handleTouchEvent(SDL_Event* event);

#ifdef NXDK
bool dinput_get_controller_state(ControllerState* state);
#endif
} // namespace fallout

#endif /* DINPUT_H */
