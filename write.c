// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

#include <SDL2/SDL_log.h>
#include <libserialport.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int reset_display(struct sp_port *port){
  SDL_Log("Reset display\n");
  uint8_t buf[2];
  int result;

  buf[0] = 0x45;
  buf[1] = 0x52;
    
  result = sp_blocking_write(port, buf, 2, 5);
  if (result != 2) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Error resetting M8 display, code %d", result);
    return 0;
  }
  return 1;
}

int enable_and_reset_display(struct sp_port *port) {
  uint8_t buf[1];
  int result;

  SDL_Log("Enabling and resetting M8 display\n");

  buf[0] = 0x44;
  result = sp_blocking_write(port, buf, 1, 5);
  if (result != 1) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Error enabling M8 display, code %d", result);
    return 0;
  }

  usleep(500);
  if (!reset_display(port)){
    return 0;
  }

  SDL_Log("M8 Display Enable Successful");
  return 1;
}

int disconnect(struct sp_port *port) {
  char buf[1] = {'D'};
  int result;

  SDL_Log("Disconnecting M8\n");

  result = sp_blocking_write(port, buf, 1, 5);
  if (result != 1) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Error sending disconnect, code %d", result);
    return -1;
  }
  return 1;
}

int send_msg_controller(struct sp_port *port, uint8_t input) {
  char buf[2] = {'C',input};
  size_t nbytes = 2;
  int result;
  result = sp_blocking_write(port, buf, nbytes, 5);
  if (result != nbytes) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Error sending input, code %d", result);
    return -1;
  }
  return 1;

}
