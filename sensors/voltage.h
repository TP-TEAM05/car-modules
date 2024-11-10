// voltage.h

// ADC Channel Definitions
#define ADC0 26
#define ADC1 27
#define ADC2 28

// Reference Voltage and ADC Resolution for RP2040
#define V_REF 3.3          // RP2040 Default Reference Voltage (V_REF)
#define ADC_RES 12         // RP2040 has a 12-bit ADC
#define MAX_ADC_LEVEL ((1 << ADC_RES) - 1) // 4095 for 12-bit

// Voltage Divider Schematic and Definitions
// 
//      V_in
//        |
//       R1 (10K)
//        |
//        +-------> ADCx (ADC input on microcontroller)
//        |
//       R2 (3.3K)
//        |
//       GND
//
// Input Voltage (V_in) = (ADCx_voltage * (R1 + R2)) / R2

#define R1 10000       // 10K ohms
#define R2 3300        // 3.3K ohms

// Macro to calculate the input voltage V_in given the voltage read at ADCx
#define INPUT_VOLTAGE(ADCx_voltage) ((ADCx_voltage) * (R1 + R2) / R2)

// Function declarations
void initVoltmeter();
float readVoltage(int adcPin);
void readVoltages();
