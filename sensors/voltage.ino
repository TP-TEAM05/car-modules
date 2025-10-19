#include "voltage.h"

void initVoltmeter() {
    analogReadResolution(ADC_RES);
}

// Function to calculate and return the input voltage on a specified ADC pin
float readVoltage(int adcPin) {
    // Read the raw ADC value
    int adcValue = analogRead(adcPin);

    // Convert the ADC value to a voltage (scaled by reference voltage and resolution)
    float adcVoltage = (adcValue / (float)MAX_ADC_LEVEL) * V_REF;

    float inputVoltage = INPUT_VOLTAGE(adcVoltage);

    return inputVoltage;
}

// Function to read voltages from all ADC
void readVoltages() {
  adc0voltage = readVoltage(ADC0);
  adc1voltage = readVoltage(ADC1);
  adc2voltage = readVoltage(ADC2);
}
