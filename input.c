// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

#include <SDL2/SDL.h>
#include <stdio.h>

#include "input.h"
#include "render.h"
#include "write.h"

#define MAX_CONTROLLERS 4
#define JOYSTICK_INDEX(Joystick) SDL_JoystickInstanceID(Joystick)

SDL_GameController *game_controllers[MAX_CONTROLLERS];

SDL_Joystick* BuiltInJS = 0; 
// Bits for M8 input messages
enum keycodes {
  key_left = 1 << 7,
  key_up = 1 << 6,
  key_down = 1 << 5,
  key_select = 1 << 4,
  key_start = 1 << 3,
  key_right = 1 << 2,
  key_opt = 1 << 1,
  key_edit = 1
};


uint8_t keycode = 0; // value of the pressed key
input_msg_s key = {normal, 0};

// Opens available game controllers and returns the amount of opened controllers
int initialize_game_controllers() {

  int num_joysticks = SDL_NumJoysticks();
  int controller_index = 0;

  SDL_Log("Looking for game controllers\n");
  SDL_Delay(
      1); // Some controllers like XBone wired need a little while to get ready
  // Open all available game controllers
  /*
  for (int i = 0; i < num_joysticks; i++) {
    if (!SDL_IsGameController(i))
      continue;
    if (controller_index >= MAX_CONTROLLERS)
      break;
    game_controllers[controller_index] = SDL_GameControllerOpen(i);
    SDL_Log("Controller %d: %s", controller_index + 1,
            SDL_GameControllerName(game_controllers[controller_index]));
    controller_index++;
  }
  */

  // Read controller mapping database
  //SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

  SDL_JoystickEventState(SDL_ENABLE);
  BuiltInJS = SDL_JoystickOpen(0);
  return controller_index;
}

// Closes all open game controllers
void close_game_controllers() {

  SDL_JoystickClose(0);
}

static input_msg_s handle_normal_keys(SDL_Event *event, uint8_t keyvalue) {
  input_msg_s key = {normal, keyvalue};
  switch (event->key.keysym.scancode) {

  case SDL_SCANCODE_UP:
    key.value = key_up;
    break;

  case SDL_SCANCODE_LEFT:
    key.value = key_left;
    break;

  case SDL_SCANCODE_DOWN:
    key.value = key_down;
    break;

  case SDL_SCANCODE_RIGHT:
    key.value = key_right;
    break;

  case SDL_SCANCODE_LSHIFT:
  case SDL_SCANCODE_A:
    key.value = key_select;
    break;

  case SDL_SCANCODE_SPACE:
  case SDL_SCANCODE_S:
    key.value = key_start;
    break;

  case SDL_SCANCODE_LALT:
  case SDL_SCANCODE_Z:
    key.value = key_opt;
    break;

  case SDL_SCANCODE_LCTRL:
  case SDL_SCANCODE_X:
    key.value = key_edit;
    break;

  case SDL_SCANCODE_DELETE:
    key.value = key_opt | key_edit;
    break;

  case SDL_SCANCODE_R:
    key = (input_msg_s){special, msg_reset_display};
    break;

  default:
    key.value = 0;
    break;
  }
  return key;
}

/*
enum Element JoyButtonsToElements[] = {
	ELEMENT_A,
	ELEMENT_B,
	ELEMENT_Y,
	ELEMENT_X,
	ELEMENT_L1,
	ELEMENT_R1,
	ELEMENT_START,
	ELEMENT_SELECT,
	ELEMENT_L3,
	ELEMENT_R3,
	ELEMENT_L2,
	ELEMENT_R2,
};
*/

static input_msg_s handle_joystick_buttons(SDL_Event *event,
                                                  uint8_t keyvalue) {
  input_msg_s key = {normal, keyvalue};
  /*
  if (event->type == SDL_JOYHATMOTION && event->jhat.hat == 0) {
	if (event->jhat.value & SDL_HAT_UP) key.value = key_up;
	if (event->jhat.value & SDL_HAT_DOWN) key.value = key_down;
	if (event->jhat.value & SDL_HAT_LEFT) key.value = key_left;
	if (event->jhat.value & SDL_HAT_RIGHT) key.value = key_right;
  } else {
  */

  switch (event->jbutton.button) {
	  case 8:
            key.value = key_up;
	    break;
	  case 10:
            key.value = key_left;
	    break;
	  case 11:
            key.value = key_right;
	    break;
	  case 9:
            key.value = key_down;
	    break;
	  case 6:
	    key.type = special;
	    key.value = msg_quit;
	    break;

	  case 3:
	  case 5:
	  case 7:
	    key.value = key_select;
	    break;

	  case 2:
	  case 13:
	    key.value = key_start;
	    break;

	  case 1:
	    key.value = key_edit;
	    break;

	  case 0:
	    key.value = key_opt;
	    break;

	  default:
	    key.value = 0;
	    break;
	  }
  //}
 // printf("%d\n", key.value);

  return key;
}

// Handles SDL input events
void handle_sdl_events() {

  SDL_Event event;

  SDL_PollEvent(&event);

  if (BuiltInJS == NULL) {
	  initialize_game_controllers();
  }

  switch (event.type) {

  // Reinitialize game controllers on controller add/remove/remap
  case SDL_CONTROLLERDEVICEADDED:
  case SDL_CONTROLLERDEVICEREMOVED:
    initialize_game_controllers();
    break;

  // Handle SDL quit events (for example, window close)
  case SDL_QUIT:
    key = (input_msg_s){special, msg_quit};
    break;

  // Keyboard events. Special events are handled within SDL_KEYDOWN.
  case SDL_KEYDOWN:

    // ALT+F4 quits program
    if (event.key.keysym.sym == SDLK_F4 &&
        (event.key.keysym.mod & KMOD_ALT) > 0) {
      key = (input_msg_s){special, msg_quit};
    }


  // Normal keyboard inputs
  case SDL_KEYUP:
    key = handle_normal_keys(&event, 0);

  // Game controller events
  case SDL_JOYBUTTONDOWN:
  case SDL_JOYBUTTONUP:
  case SDL_JOYHATMOTION:
	    key = handle_joystick_buttons(&event, 0);
    break;

  default:
    break;
  }


  if (key.type == normal) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN) 
      keycode |= key.value;
    else if (event.type == SDL_JOYHATMOTION) {
	  uint8_t maskedValue = key.value & 228;
	  keycode &= 27;
	  keycode |= maskedValue;
    } else
      keycode &= ~key.value;
  } else {
    if (event.type == SDL_KEYDOWN)
      keycode = key.value;
    else
      keycode = 0;
  }
}

// Returns the currently pressed keys to main
input_msg_s get_input_msg() {

  key = (input_msg_s){normal, 0};

  // Query for SDL events
  handle_sdl_events();

  return (input_msg_s){key.type, keycode};
}
