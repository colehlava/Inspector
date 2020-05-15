// main.c

#include <project.h>
#include <stdlib.h>

#define MOTOR_PWM_PERIOD    10
#define RIGHT_MOTOR_FORWARD  0
#define LEFT_MOTOR_FORWARD   1
#define RIGHT_MOTOR_REVERSE  1
#define LEFT_MOTOR_REVERSE   0
#define USB_DEVICE_NUMBER    0
#define USB_OUT_ENDPOINT     1
#define USB_TRANSFER_SIZE    8

static uint8 usb_data[USB_TRANSFER_SIZE];

// Set motor direction
void setMotorDirection(int direction);

// Set motor speed
void setMotorSpeed(int speed);

// Map speed input value [0,99] to PWM compare value [0,MOTOR_PWM_PERIOD]
int16 map2pwm(int16 value);

// main
int main() {
    // Enable global interrupts
    CyGlobalIntEnable;
    
	// Start hardware components
	LCD_Char_Start();
	PWM_Motor_L1_Start();
	PWM_Motor_L2_Start();
    USB1_Start(USB_DEVICE_NUMBER, USB1_5V_OPERATION);
    
    // Block until USB device is configured
    while ( !USB1_GetConfiguration() ) {}

    // Enable OUT endpoint to receive data from host
    USB1_EnableOutEP(USB_OUT_ENDPOINT);
    
    // Configure PWM
    PWM_Motor_L1_WritePeriod(MOTOR_PWM_PERIOD);
    PWM_Motor_L2_WritePeriod(MOTOR_PWM_PERIOD);
    
    LCD_Char_ClearDisplay();
	LCD_Char_PrintString("On");
    
    // Create and zero out array to store incoming USB data
    uint16 bytes_to_receive, bytes_received;
    char forward_state_string[3];
    char speed_state_string[3];
    int forward_state_int, speed_state_int, interpret_usb_success;
    //uint8 usb_data[USB_TRANSFER_SIZE];
    for (int i = 0; i < USB_TRANSFER_SIZE; i++)
        usb_data[i] = 0;
    
    // Main loop
    while (1) {
        // Ensure USB remains configured
        if (USB1_IsConfigurationChanged() != 0u) {
            // Re-enable endpoint on reconfigure
            if (USB1_GetConfiguration() != 0u) {
                USB1_EnableOutEP(USB_OUT_ENDPOINT);
            }
        }
        
        // Check if USB data was received from host
        if (USB1_GetEPState(USB_OUT_ENDPOINT) == USB1_OUT_BUFFER_FULL) {
            // Get number of bytes in buffer
            bytes_to_receive = USB1_GetEPCount(USB_OUT_ENDPOINT);
            
            // Read USB transfer from host
            bytes_received = USB1_ReadOutEP(USB_OUT_ENDPOINT, usb_data, bytes_to_receive);
            
            // Don't adjust motors on failed USB read
            if (bytes_received != USB_TRANSFER_SIZE) continue;
            
            // Interpret received USB data
            interpret_usb_success = 1;
            for (int i = 0; i < bytes_received - 1; i++) {
                if (usb_data[i++] == 'F') {
                    forward_state_string[0] = usb_data[i++];
                    forward_state_string[1] = usb_data[i++];
                    forward_state_string[2] = '\0';
                    forward_state_int = atoi(forward_state_string);
                }
                else {
                    interpret_usb_success = 0;
                    break;
                }
                if (usb_data[i++] != ',') {
                    interpret_usb_success = 0;
                    break;
                }
                if (usb_data[i++] == 'S') {
                    speed_state_string[0] = usb_data[i++];
                    speed_state_string[1] = usb_data[i++];
                    speed_state_string[2] = '\0';
                    speed_state_int = atoi(speed_state_string);
                }
                else {
                    interpret_usb_success = 0;
                    break;
                }
                break;
            }
            
            // Don't adjust motors on failed USB read
            if ( !interpret_usb_success ) continue;
            
            // Display interpreted USB data to OLED
            LCD_Char_ClearDisplay();
            LCD_Char_PrintDecUint16(forward_state_int);
            LCD_Char_PutChar('-');
            LCD_Char_PrintDecUint16(speed_state_int);
            
            // Set motor direction and speed
            setMotorDirection(forward_state_int);
            setMotorSpeed(speed_state_int);
        }
    }
}


// Set motor direction
void setMotorDirection(int direction) {
    if (direction == 1) {
        Motor_Dir_Pin1_Write(LEFT_MOTOR_FORWARD);
        Motor_Dir_Pin2_Write(~LEFT_MOTOR_FORWARD);
    }
    else {
        Motor_Dir_Pin1_Write(~LEFT_MOTOR_FORWARD);
        Motor_Dir_Pin2_Write(LEFT_MOTOR_FORWARD);
    }
}


// Set motor speed
void setMotorSpeed(int speed) {
    int16 scaled_speed = map2pwm(speed);
    PWM_Motor_L1_WriteCompare(scaled_speed);
    PWM_Motor_L2_WriteCompare(scaled_speed);
}


// Map speed input value [0,99] to PWM compare value [0,MOTOR_PWM_PERIOD]
int16 map2pwm(int16 value) {
    return value * MOTOR_PWM_PERIOD / 99;
}
