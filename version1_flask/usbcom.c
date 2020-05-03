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

#define FILTER_SIZE 5
#define INPOINT 0x2


// **************************************************************************************
// main
int main (int argc, char * argv[]) {
  
  // USB variables
  int return_val;
  int sent_bytes;

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


  // Main loop
  while (1) {
      // Perform OUT transfer (from host to device).
      return_val = libusb_bulk_transfer(device, INPOINT, rx_data, TRANSFER_SIZE, &sent_bytes, 0);

      // Throw error if OUT transfer failed
      if (return_val != 0) {
          perror("OUT transfer failed\n");
          return -1;
      }
  }

  libusb_close(device);
  return 0;
}

