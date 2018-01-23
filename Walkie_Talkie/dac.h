/*
 * Header file for the simple MCP4921 DAC
 * driver. Supports writing to the DAC
 * with a gain of 1 and 12 bits of resolution.
 *
 * Authors: Brett Glidden, William Rachuy
 */

#ifndef DAC_H_
#define DAC_H_

void DAC_init();
void DAC_drive(uint16_t level);

#endif /* DAC_H_ */
