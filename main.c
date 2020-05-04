// main.c

#include <project.h>

#define MOTOR_PWM_PERIOD  10
#define USB_DEVICE_NUMBER  0
#define USB_OUT_ENDPOINT   1
#define USB_TRANSFER_SIZE  8

static uint8 usb_data[USB_TRANSFER_SIZE];

// Map ADC reading to PWM compare value
int16 map2pwm(int16 value);

// main
int main() {
    // Enable global interrupts
    CyGlobalIntEnable;
    
	// Start hardware components
	LCD_Char_Start();
	PWM_Motor_R_Start();
	PWM_Motor_L_Start();
    ADC_SAR_R_Start();
    ADC_SAR_R_StartConvert();
    ADC_SAR_L_Start();
    ADC_SAR_L_StartConvert();    
    USB1_Start(USB_DEVICE_NUMBER, USB1_5V_OPERATION);
    
    // Block until USB device is configured
    while ( !USB1_GetConfiguration() ) {}
    
    LCD_Char_ClearDisplay();
    LCD_Char_PrintString("Beginjamin");

    // Enable OUT endpoint to receive data from host
    USB1_EnableOutEP(USB_OUT_ENDPOINT);
    
    // Configure PWM
    PWM_Motor_R_WritePeriod(MOTOR_PWM_PERIOD);
    PWM_Motor_L_WritePeriod(MOTOR_PWM_PERIOD);
    
    LCD_Char_ClearDisplay();
	LCD_Char_PrintString("Onjamin");
    
    // Create and zero out array to store incoming USB data
    uint16 bytes_to_receive, bytes_received;
    //uint8 usb_data[USB_TRANSFER_SIZE];
    for (int i = 0; i < USB_TRANSFER_SIZE; i++)
        usb_data[i] = 0;
    
    // Main loop
    while (1) {
        // Set motor speed
        /*int16 potValue_R = map2pwm(ADC_SAR_R_GetResult16());
        int16 potValue_L = map2pwm(ADC_SAR_L_GetResult16());
        PWM_Motor_R_WriteCompare(potValue_R);
        PWM_Motor_L_WriteCompare(potValue_L);*/
        
        // Ensure USB remains configured
        if (USB1_IsConfigurationChanged() != 0u) {
            // Re-enable endpoint when device is configured
            if (USB1_GetConfiguration() != 0u) {
                // Enable OUT endpoint to receive data from device
                USB1_EnableOutEP(USB_OUT_ENDPOINT);
            }
        }
        
        // Check if USB data was received from host
        if (USB1_GetEPState(USB_OUT_ENDPOINT) == USB1_OUT_BUFFER_FULL) {
            // Get number of bytes received
            bytes_to_receive = USB1_GetEPCount(USB_OUT_ENDPOINT);
            
            // Read USB transfer from host
            bytes_received = USB1_ReadOutEP(USB_OUT_ENDPOINT, usb_data, bytes_to_receive);
            
            // Display received USB data
            LCD_Char_ClearDisplay();
            for (int i = 0; i < bytes_received; i++)
                LCD_Char_PutChar(usb_data[i]);
        }
        //CyDelay(500);
    }
}


// Map ADC reading to PWM compare value
int16 map2pwm(int16 value) {
    if (value < 0) value = 0;
    return value * MOTOR_PWM_PERIOD / 4095;
}
