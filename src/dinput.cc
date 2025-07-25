#include "dinput.h"

namespace fallout {



static int gMouseWheelDeltaX = 0;
static int gMouseWheelDeltaY = 0;

// 0x4E0400
bool directInputInit()
{
    // NXDK: Basic controller support FIXME
#ifdef NXDK
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return false;
    };
#endif
    
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0) {
        return false;
    }

    if (!mouseDeviceInit()) {
        goto err;
    }

    if (!keyboardDeviceInit()) {
        goto err;
    }

    return true;

err:

    directInputFree();

    return false;
}

// 0x4E0478
void directInputFree()
{
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

// 0x4E04E8
bool mouseDeviceAcquire()
{
    return true;
}

// 0x4E0514
bool mouseDeviceUnacquire()
{
    return true;
}

// 0x4E053C
bool mouseDeviceGetData(MouseData* mouseState)
{
#ifdef NXDK
    // Handle both real mouse and controller input
    ControllerState controllerState;
    if (dinput_get_controller_state(&controllerState)) {
        // Convert analog input to mouse movement
        // Scale the movement - adjust these values to taste
        const float sensitivity = 15.0f;
        mouseState->x = (int)(controllerState.analogX * sensitivity);
        mouseState->y = (int)(controllerState.analogY * sensitivity);
        mouseState->buttons[0] = controllerState.buttonA;
        mouseState->buttons[1] = controllerState.buttonB;
        mouseState->wheelX = 0;
        mouseState->wheelY = 0;
        return true;
    }
    return false;
#else
    // CE: This function is sometimes called outside loops calling `get_input`
    // and subsequently `GNW95_process_message`, so mouse events might not be
    // handled by SDL yet.
    //
    // TODO: Move mouse events processing into `GNW95_process_message` and
    // update mouse position manually.
    SDL_PumpEvents();

    Uint32 buttons = SDL_GetRelativeMouseState(&(mouseState->x), &(mouseState->y));
    mouseState->buttons[0] = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    mouseState->buttons[1] = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    mouseState->wheelX = gMouseWheelDeltaX;
    mouseState->wheelY = gMouseWheelDeltaY;

    gMouseWheelDeltaX = 0;
    gMouseWheelDeltaY = 0;

    return true;
#endif
}

// 0x4E05A8
bool keyboardDeviceAcquire()
{
    return true;
}

// 0x4E05D4
bool keyboardDeviceUnacquire()
{
    return true;
}

// 0x4E05FC
bool keyboardDeviceReset()
{
    SDL_FlushEvents(SDL_KEYDOWN, SDL_TEXTINPUT);
    return true;
}

#ifdef NXDK
// Add tracking of previous button states
static bool previousButtonStates[SDL_CONTROLLER_BUTTON_MAX] = { false };

// Define default button mappings
const ControllerKeyMapping CONTROLLER_KEY_MAPPINGS[] = {
    // Face buttons (A and B are set to the mouse buttons)
    { SDL_CONTROLLER_BUTTON_X, SDL_SCANCODE_I }, // Inventory
    { SDL_CONTROLLER_BUTTON_Y, SDL_SCANCODE_P }, // Pip-Boy

    // Start/Back
    { SDL_CONTROLLER_BUTTON_BACK, SDL_SCANCODE_C }, // Character Sheet
    { SDL_CONTROLLER_BUTTON_START, SDL_SCANCODE_ESCAPE }, // Options Menu

    // D-Pad
    { SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_SCANCODE_UP }, // Up
    { SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_SCANCODE_DOWN }, // Down
    { SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_SCANCODE_S }, // Skilldex
    { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_SCANCODE_F6 }, // Quick Save

    // White and Black
    { SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_SCANCODE_SPACE }, // (white) End Turn
    { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_SCANCODE_KP_ENTER },// (black) End Combat

    // Thumbstick clicks
    { SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_SCANCODE_HOME }, // Center Camera On Player
    { SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_SCANCODE_A }, // Activate Combat Mode
};

const int CONTROLLER_KEY_MAPPING_COUNT = sizeof(CONTROLLER_KEY_MAPPINGS) / sizeof(CONTROLLER_KEY_MAPPINGS[0]);

// 0x4E0650
bool keyboardDeviceGetData(KeyboardData* keyboardData)
{
    SDL_GameController* controller = SDL_GameControllerOpen(0);
    if (controller == NULL) {
        return false;
    }

    // Check all mapped buttons
    for (int i = 0; i < CONTROLLER_KEY_MAPPING_COUNT; i++) {
        const ControllerKeyMapping* mapping = &CONTROLLER_KEY_MAPPINGS[i];
        bool currentState = SDL_GameControllerGetButton(controller, mapping->button) != 0;
        
        // Only trigger on state changes
        if (currentState != previousButtonStates[mapping->button]) {
            keyboardData->key = mapping->scancode;
            keyboardData->down = currentState ? 1 : 0;
            
            // Store new state
            previousButtonStates[mapping->button] = currentState;
            
            return true;
        }
    }

    return false;
}
#endif

// 0x4E070C
bool mouseDeviceInit()
{
    // NXDK: Find out how to make the ACTUAL mouse work
#ifdef NXDK
    return true;
#else
    return SDL_SetRelativeMouseMode(SDL_TRUE) == 0;
#endif
}

// 0x4E078C
void mouseDeviceFree()
{
}

// 0x4E07B8
bool keyboardDeviceInit()
{
    return true;
}

// 0x4E0874
void keyboardDeviceFree()
{
}

void handleMouseEvent(SDL_Event* event)
{
    // Mouse movement and buttons are accumulated in SDL itself and will be
    // processed later in `mouseDeviceGetData` via `SDL_GetRelativeMouseState`.

    if (event->type == SDL_MOUSEWHEEL) {
        gMouseWheelDeltaX += event->wheel.x;
        gMouseWheelDeltaY += event->wheel.y;
    }
}

#ifdef NXDK
bool dinput_get_controller_state(ControllerState* state)
{
    SDL_GameController* controller = SDL_GameControllerOpen(0);
    if (controller == NULL) {
        return false;
    }

    // Get analog stick values and normalize them to -1.0 to 1.0
    float axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
    float axisY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
    float rightX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
    float rightY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;

    // Apply deadzone
    const float deadzone = 0.2f;
    if (fabs(axisX) < deadzone) axisX = 0;
    if (fabs(axisY) < deadzone) axisY = 0;
    if (fabs(rightX) < deadzone) rightX = 0;
    if (fabs(rightY) < deadzone) rightY = 0;

    state->analogX = axisX;
    state->analogY = axisY;
    state->rightStickX = rightX;
    state->rightStickY = rightY;
    state->buttonA = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) != 0;
    state->buttonB = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) != 0;

    return true;
}
#endif

} // namespace fallout
