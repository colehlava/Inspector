// usbcom.c
// Writes motor commands to microcontroller through USB.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <libusb.h>
#include <wiringPiI2C.h>

#define MESSAGE_SIZE      8
#define INPOINT           1 
#define VENDOR_ID    0x04B4
#define PRODUCT_ID   0x8051


// main
int main (int argc, char * argv[]) {
  
  // USB variables
  int usb_return_val;
  int sent_bytes;
  char message[MESSAGE_SIZE];

  // Configure USB
  libusb_init(NULL);            // Initialize the LIBUSB library
  libusb_device_handle* device; // Pointer to USB device data structure

  // Open the USB device
  device = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);

  if (device == NULL) {
    printf("USB device not found\n");
    return -1;
  }

  // Reset the USB device and clear any residual states
  if (libusb_reset_device(device) != 0) {
    printf("USB device reset failed\n");
    return -1;
  } 

  // Set configuration of USB device
  if (libusb_set_configuration(device, 1) != 0) {
    printf("USB set configuration failed\n");
    return -1;
  } 

  // Claim the interface (needed before any I/Os can be issued to the USB device)
  if (libusb_claim_interface(device, 0) != 0) {
    printf("Cannot claim USB interface\n");
    return -1;
  }

  // Message formatting variables
  FILE* button_file;
  FILE* speed_file;
  char button_status_string[4];
  char speed_status_string[4];
  int button_status_int, speed_status_int;

  // Main loop
  while (1) {
      // Open data files
      button_file = fopen("button_status.txt", "r");
      speed_file = fopen("speed.txt", "r");

      // Ensure files exist
      if (button_file == NULL || speed_file == NULL) {
        printf("Failed to open data files.\n");
        return -1;
      }

      // Format message to be sent over USB
      fgets(button_status_string, 4, button_file);
      button_status_int = atoi(button_status_string);

      fgets(speed_status_string, 4, speed_file);
      speed_status_int = atoi(speed_status_string);

      //printf("button_status_string = %s, speed_status_string = %s\n", button_status_string, speed_status_string);
      //printf("button_status = %d, speed_status = %d\n", button_status_int, speed_status_int);
      sprintf(message, "F%02d,S%02d", button_status_int, speed_status_int);
      printf("%s\n", message);

      // Perform OUT transfer (from host to device).
      usb_return_val = libusb_bulk_transfer(device, INPOINT, message, MESSAGE_SIZE, &sent_bytes, 0);

      // Throw error if OUT transfer failed
      if (usb_return_val != 0) {
          perror("OUT transfer failed\n");
          return -1;
      }

      // Delay
      for (int i = 0; i < 200000000; i++);
  }

  // Cleanup
  libusb_close(device);
  if (button_file != NULL)
      fclose(button_file);
  if (speed_file != NULL)
      fclose(speed_file);
  return 0;
}

