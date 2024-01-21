#include "displayOptions.h"
void setWarningColor() {
  printf("\033[0;31m"); // Set the text to the color red.
  return;
}

void resetToDefaultColor() {
  printf("\033[0m"); // reset to default color
  return;
}