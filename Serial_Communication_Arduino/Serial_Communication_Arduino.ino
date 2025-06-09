// this sample code provided by www.programmingboss.com
void setup() {
  Serial.begin(9600);
}
void loop() {
  float h = 40.0;
  float t= 24.0;
  //Serial.println("Hello Boss");
  Serial.print("{\"Humidity\": ");
  Serial.print(h);
  Serial.print(", \"Temperature\":");
  Serial.print(t);
  Serial.print("}");

  delay(1500);
}