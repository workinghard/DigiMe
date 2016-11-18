#include <pebble.h>
#include "helperFunctions.h"


// Helper functions
char* floatToString(char* buffer, int bufferSize, double number) {
  char decimalBuffer[5];

  snprintf(buffer, bufferSize, "%d", (int)number);
  strcat(buffer, ".");

  snprintf(decimalBuffer, 5, "%02d", (int)((double)(number - (int)number) * (double)100));
  strcat(buffer, decimalBuffer);

  return buffer;
}