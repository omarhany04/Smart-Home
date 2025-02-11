#include <Keypad.h>
#include <Servo.h> // Include the Servo library

#define THERMISTORPIN A1        
// resistance of termistor at 25 degrees C
#define THERMISTORNOMINAL 10000      
#define TEMPERATURENOMINAL 25   
// Accuracy - Higher number is bigger
#define NUMSAMPLES 10
// Beta coefficient from datasheet
#define BCOEFFICIENT 3950
// the value of the R1 resistor
#define SERIESRESISTOR 10000   

#define FAN 16
//prepare pole 
uint16_t samples[NUMSAMPLES]; 

float temperature = 0; // Placeholder for temperature value
unsigned long previousMillis = 0; // To store the last time the fan was turned on
unsigned long currentMillis = 0; // To store the current time
bool fanState = false; // To track if the fan is currently ON

Servo myServo;     // Create a servo object
void Check(String text);
void setNewPassword(); // Function to create a new password
float Temprature();

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

const String password = "1234"; // Predefined password
String enteredPass = ""; // String to store the entered password
String uartBuffer= "";
String newPass = "";

// Define the keys on the keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Define the pins for rows and columns
byte rowPins[ROWS] = {2, 10, 4, 5};    // Connect to the row pins
byte colPins[COLS] = {6, 7, 8, 9};    // Connect to the column pins

// Create a Keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600); // Start serial communication
  pinMode(11, OUTPUT); // Set the built-in LED as an output
  pinMode(12, OUTPUT); // Set the built-in LED as an output
  pinMode(13, OUTPUT); // Set the built-in LED as an output
  pinMode(FAN, OUTPUT);
  myServo.attach(3);
  myServo.write(0);
  Serial.println("Program Started");
}

void loop() {
  char key = keypad.getKey(); // Check if a key is pressed
  
  if (key) { // If a key is pressed
    if (key == '#') { // '#' acts as a "clear" key
      enteredPass = ""; // Clear the entered password
      Serial.println("Password cleared.");
    } 
    else if (key == '*') { // '' acts as the "Enter" key
      Check(enteredPass);
      enteredPass="";
    } 
    else if(key == 'A'){    // A is F1
      setNewPassword();
    }
    else { // Add the key to the entered password
      if (enteredPass.length() < 4) { // Limit password length to 4 characters
        enteredPass += key;
        Serial.print("Entered: ");
        Serial.println(enteredPass);
      } else {
        Serial.println("Password length exceeded. Press '*' to clear.");
      }
    }
  }
  
  if (Serial.available() > 0) {
    String uartBuffer = Serial.readString();
    if (uartBuffer.charAt(0) < 'A')
    {
      Serial.println(uartBuffer);
      Check(uartBuffer);
      uartBuffer="";
    }
    else
    {
      uartBuffer.toLowerCase(); // Convert the string to lowercase
      if (uartBuffer == "get temp") {
        Serial.println("The Temperature is: " + String(Temprature()) + "°C");
      }
    }
  }

int analogVoltage = analogRead(A0); // Read the analog value from pin A0
  if (analogVoltage < 400) {
    digitalWrite(13, HIGH); // Turn on LED if voltage is less than 400
  } else {
    digitalWrite(13, LOW); // Turn off LED if voltage is higher 
  }

  currentMillis = millis(); // Update the current time

  // Simulate temperature reading (replace with actual temperature reading code)
  temperature = Temprature();

  // Check if temperature exceeds 50°C
  if (temperature > 50 && !fanState) {
    previousMillis = currentMillis; // Record the time the fan was turned on
    digitalWrite(FAN, HIGH); // Turn the fan ON
    fanState = true; // Update the fan state
    Serial.println("Fan turned ON");
  }

  // Turn off the fan if 30 seconds have passed since it was turned on
  if (fanState && (currentMillis - previousMillis >= 15000)) {
    digitalWrite(FAN, LOW); // Turn the fan OFF
    fanState = false; // Update the fan state
    Serial.println("Fan turned OFF");
  }
  
  
}


void Check(String text)
{
  if (text == password) {
        Serial.println("Correct Password!");
        digitalWrite(11, HIGH);
        delay(350);
        digitalWrite(11, LOW);
        digitalWrite(12, HIGH);
        delay(1700);
        digitalWrite(12, LOW);
      } else {
        Serial.println("Incorrect Password!");
        digitalWrite(11, HIGH);
        delay(100);
        digitalWrite(11, LOW);
        delay(50);
        digitalWrite(11, HIGH);
        delay(100);
        digitalWrite(11, LOW);
      }
}

void setNewPassword(){
  Serial.println("Enter a new 4-digit password:");
  while(newPass.length() < 4){
    char key = keypad.getKey();
    if(key){
      if(key =='#'){   
        newPass = "";
        Serial.println("Input cleared. Enter a new password:");
      }
      else if(key >= '0' && key<= '9'){    // Accept digits only
        newPass += key;
        Serial.print("Entered: ");
        Serial.println(newPass);
      }
      else{
        Serial.println("Only numbers are allowed. Use '#' to clear");
      }
    }
  }

  password = newPass;    // Update the password
  Serial.println("Password updated successfully!");
  Serial.println("New password is: "+ newPass);
}

float Temprature()
{
  uint8_t i;
  float average;
  // saving values from input to pole
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
   // average value of input
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  //resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

 //resistence to temperature
  float temperature;
  temperature = average / THERMISTORNOMINAL; 
  temperature = log(temperature);   
  temperature /= BCOEFFICIENT;                  
  temperature += 1.0 / (TEMPERATURENOMINAL + 273.15); 
  temperature = 1.0 / temperature;                 
  temperature -= 273.15;                        

  return temperature;
}