void parseUxUyFromString(char *data, float *uX, float *uY) {
  char *pEnd;

  *uX = strtof(data+1, &pEnd);
  *uY = strtof(pEnd+1, NULL);

  // Serial.print("Parsed: x -> ");
  // Serial.print(*uX);
  // Serial.print(" y -> ");
  // Serial.println(*uY);
}
