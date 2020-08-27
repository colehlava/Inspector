// main.c

#include <project.h>
#include <stdlib.h>

// Define to enable debug mode
// #define DEBUG

// Motor constants
#define MOTOR_PWM_PERIOD         10
#define RIGHT_MOTOR_FORWARD       0
#define LEFT_MOTOR_FORWARD        1
#define RIGHT_MOTOR_REVERSE       1
#define LEFT_MOTOR_REVERSE        0

// USB constants
#define USB_DEVICE_NUMBER    0
#define USB_OUT_ENDPOINT     1
#define USB_TRANSFER_SIZE    8

static uint8 usb_data[USB_TRANSFER_SIZE];

// Set motors based on user input
void setMotors(char key, int speed);

// Set motor direction
void setMotorDirection(int L_direction, int R_direction);

// Set motor speed
void setMotorSpeed(int L_speed, int R_speed);

// Map speed input value [0,99] to PWM compare value [0,MOTOR_PWM_PERIOD]
int16 map2pwm(int16 value);

// main
int main() {
    // Enable global interrupts
    CyGlobalIntEnable;
    
	// Start hardware components
	PWM_Motor_L_Start();
	PWM_Motor_R_Start();
    USB1_Start(USB_DEVICE_NUMBER, USB1_5V_OPERATION);
    
    // Block until USB device is configured
    while ( !USB1_GetConfiguration() ) {}

    // Enable OUT endpoint to receive data from host
    USB1_EnableOutEP(USB_OUT_ENDPOINT);
    
    // Configure PWM
    PWM_Motor_L_WritePeriod(MOTOR_PWM_PERIOD);
    PWM_Motor_R_WritePeriod(MOTOR_PWM_PERIOD);
    
    #ifdef DEBUG
        LCD_Char_Start();
        LCD_Char_ClearDisplay();
    	LCD_Char_PrintString("On");
    #endif
    
    // Create and zero out array to store incoming USB data
    uint16 bytes_to_receive, bytes_received;
    char speed_state_string[3];
    int interpret_usb_success, data_index;
    int power_state = 0;
    int speed_value = 50;
    int prev_speed_value = 0;
    char key_input = 'w';
    char prev_key_input = 'q';
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
            
            interpret_usb_success = 1;
            data_index = 0;
            
            // Interpret received USB data
            while (1) {
                // Read power state
                if (usb_data[data_index++] == 'P') {
                    power_state = usb_data[data_index++];
                    power_state -= '0';
                }
                else {
                    interpret_usb_success = 0;
                    break;
                }
                
                // Read speed value
                if (usb_data[data_index++] == 'S') {
                    speed_state_string[0] = usb_data[data_index++];
                    speed_state_string[1] = usb_data[data_index++];
                    speed_state_string[2] = '\0';
                    speed_value = atoi(speed_state_string);
                }
                else {
                    interpret_usb_success = 0;
                    break;
                }
                
                // Read direction input
                if (usb_data[data_index++] == 'D') {
                    key_input = usb_data[data_index];
                }
                else {
                    interpret_usb_success = 0;
                    break;
                }
                break;
            }
            
            // Don't adjust motors on failed USB read
            if ( !interpret_usb_success ) continue;
            
            #ifdef DEBUG
                // Display interpreted USB data to OLED
                LCD_Char_ClearDisplay();
                LCD_Char_PrintDecUint16(power_state);
                LCD_Char_PutChar('-');
                LCD_Char_PrintDecUint16(speed_value);
                LCD_Char_PutChar('-');
                LCD_Char_PutChar(key_input);
            #endif
            
            // Set motor speed and direction
            if (power_state == 0) {
                setMotorSpeed(0, 0);
            }
            else {
                // Only adjust motors on change of input
                if (key_input != prev_key_input || speed_value != prev_speed_value)
                    setMotors(key_input, speed_value);
            }
        }
    }
}


// Set motors based on user input
void setMotors(char key, int speed) {
    // Stop motors while making adjustments
    setMotorSpeed(0, 0);
    
    switch (key) {
        case 'w':
            setMotorDirection(1, 1);
            setMotorSpeed(speed, speed);
            break;
        case 's':
            setMotorDirection(0, 0);
            setMotorSpeed(speed, speed);
            break;
        case 'a':
            setMotorDirection(1, 1);
            setMotorSpeed(speed >> 1, speed);
            break;
        case 'd':
            setMotorDirection(1, 1);
            setMotorSpeed(speed, speed >> 1);
            break;
        case 'f':
            setMotorDirection(0, 1);
            setMotorSpeed(speed, speed);
            break;
        case 'j':
            setMotorDirection(1, 0);
            setMotorSpeed(speed, speed);
            break;
        default:
            break;
    }
}


// Set motor direction
void setMotorDirection(int L_direction, int R_direction) {
    // Left
    if (L_direction == 1) {
        L_Motor_Dir_Pin1_Write(LEFT_MOTOR_FORWARD);
        L_Motor_Dir_Pin2_Write(~LEFT_MOTOR_FORWARD);
    }
    else {
        L_Motor_Dir_Pin1_Write(~LEFT_MOTOR_FORWARD);
        L_Motor_Dir_Pin2_Write(LEFT_MOTOR_FORWARD);
    }
    
    // Right
    if (R_direction == 1) {
        R_Motor_Dir_Pin1_Write(RIGHT_MOTOR_FORWARD);
        R_Motor_Dir_Pin2_Write(~RIGHT_MOTOR_FORWARD);
    }
    else {
        R_Motor_Dir_Pin1_Write(~RIGHT_MOTOR_FORWARD);
        R_Motor_Dir_Pin2_Write(RIGHT_MOTOR_FORWARD);
    }
}


// Set motor speed
void setMotorSpeed(int L_speed, int R_speed) {
    int16 L_scaled_speed = map2pwm(L_speed);
    int16 R_scaled_speed = map2pwm(R_speed);
    
    PWM_Motor_L_WriteCompare(L_scaled_speed);
    PWM_Motor_R_WriteCompare(R_scaled_speed);
}


// Map speed input value [0,99] to PWM compare value [0,MOTOR_PWM_PERIOD]
int16 map2pwm(int16 value) {
    return value * MOTOR_PWM_PERIOD / 99;
}
