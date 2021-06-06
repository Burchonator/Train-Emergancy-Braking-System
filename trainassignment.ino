/*
  Embedded Systems Development Project 2021

  Emergency Train Braking System
  can also be called an automatic train braking system

  Works by detecting a potential hazard and slowing down safely
  to prevent damage and injury to people and objects inside and
  outside of the train.

  Mitchell Ross Burcheri - 220206872
*/

// The following line is optional, but it allows your code to run
// even when not cloud connected
SYSTEM_THREAD(ENABLED);

// defining led pins
const int ledLevel4 = D7; // red led aka risk 4
const int ledLevel3 = D6; // orange led aka risk 3
const int ledLevel2 = D5; // yellow led aka risk 2
const int ledLevel1 = D4; // green led aka risk 1

// defining ultrasonic sensor trigger and echo pins
const int trigPin = D3;
const int echoPin = D2;
// create duration and distance variable for ultrasonic sensor readings
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

// Was going to use a buzzer but it would delay the sensor readings
//const int buzzer = -1; //buzzer. Set buzzer to 9 for arduino pin 9
// I have removed all other buzzer code

// create an array for comparing ultrasonic distance readings
const int arraySize = 15;// array size (CHANGEABLE)
int distanceCheckingArray[arraySize]; // set the size of array

int riskLevel = 0; // variable for recording the level of risk
// 1 for light risk
// 2 for medium risk
// 3 for high risk
// 4 for serious risk
// 5 for has had a collision and was too late to break in time safely

// risk zone ranges (only for testing purposes) (CHANGEABLE)
const int riskRange1 = 160; // 1.6m
const int riskRange2 = 90; // 0.9m
const int riskRange3 = 40; // 0.4m

void setup() {
  // Booting up the system
  Serial.println("Emergancy braking system booting...");
  Serial.begin(9600); // // serial communication at 9600 baudrate
  pinMode(trigPin, OUTPUT); // setting trigger pin as output pin
  pinMode(echoPin, INPUT); // setting echo pin uses as input pin
  pinMode(ledLevel4, OUTPUT); // setting led as output pin
  pinMode(ledLevel3, OUTPUT); // setting led as output pin
  pinMode(ledLevel2, OUTPUT); // setting led as output pin
  pinMode(ledLevel1, OUTPUT); // setting led as output pin
  digitalWrite(ledLevel4, LOW); // setting led to off
  digitalWrite(ledLevel3, LOW); // setting led to off
  digitalWrite(ledLevel2, LOW); // setting led to off
  digitalWrite(ledLevel1, LOW); // setting led to off

  // creating riskLevel and distance as viewable variables on the particle console
  Particle.variable("riskLevel", riskLevel);
  Particle.variable("distance", distance);

  Serial.println("...Running");
  delay(1000); // delay 1 second
}
void loop() {
  scan(); // checks the distance using the ultrasonic sensor
  assignRiskLevel(); // determines the level of risk
  triggerIFTTT(); // runs the code to trigger an IFTTT integration
  delay(100); // 0.1 seconds works well (CHANGEABLE)
}


void scan() { // checks the distance using the ultrasonic sensor
  int i = 0;
  for (i; i < arraySize; ++i) { // run the for loop depending on the arraySize
    digitalWrite(trigPin, LOW); // turn the trig pin off
    delayMicroseconds(2);// wait 2 micro seconds
    digitalWrite(trigPin, HIGH); // turn the trig pin on
    delayMicroseconds(10); // leave trig pin on for 10 micro seconds
    digitalWrite(trigPin, LOW);  // turn the trig pin off
    duration = pulseIn(echoPin, HIGH); // read the sound wave travel time in microseconds using the echo pin
    distance = duration * 0.034 / 2; // calculate the distance by the speed of sound wave divided by 2 (out and back)
    distanceCheckingArray[i] = distance; // add the distance to the array
  }
  int n = sizeof(distanceCheckingArray) / sizeof(distanceCheckingArray[0]); // accurate size of the arry
  quickSort(distanceCheckingArray, 0, n - 1); // perform a quicksort using a recursion function

  // display the array sorted in the serial output (for debugging/testing)
  for (int j = 0; j < arraySize; j++)
  {
    Serial.println(distanceCheckingArray[j]);
  }

  distance = distanceCheckingArray[2]; // make the distance the 3 closest reading removes a bad reading
  // display the actual distance (for debugging/testing)
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}


void assignRiskLevel() { // determines the level of risk
  if (distance < riskRange3 && riskLevel < 3) { // risk 5
    riskLevel = 5;
    riskLevel5();
  }
  else if (distance >= riskRange1) { // risk 1
    riskLevel = 1;
    riskLevel1();
  }
  else if (distance < riskRange1 && distance >= riskRange2) { // risk 2
    riskLevel = 2;
    riskLevel2();
  }
  else if (distance < riskRange2 && distance >= riskRange3) { // risk 3
    riskLevel = 3;
    riskLevel3();
  }
  else if (distance < riskRange3 && distance >= 0) { // risk 4
    riskLevel = 4;
    riskLevel4();
  }
  else { // if less than 0 turn all LEDs on to show there is a problem
    allOn(); // turn all leds on
  }
}

void riskLevel1() { // risk 1 only turn ledLevel1 on
  digitalWrite(ledLevel4, LOW);
  digitalWrite(ledLevel3, LOW);
  digitalWrite(ledLevel2, LOW);
  digitalWrite(ledLevel1, HIGH);
}
void riskLevel2() { // risk 2 only turn ledLevel2 on
  digitalWrite(ledLevel4, LOW);
  digitalWrite(ledLevel3, LOW);
  digitalWrite(ledLevel2, HIGH);
  digitalWrite(ledLevel1, LOW);
}
void riskLevel3() { // risk 3 only turn ledLevel3 on
  digitalWrite(ledLevel4, LOW);
  digitalWrite(ledLevel3, HIGH);
  digitalWrite(ledLevel2, LOW);
  digitalWrite(ledLevel1, LOW);
}
void riskLevel4() { // risk 4 only turn ledLevel4 on
  digitalWrite(ledLevel4, HIGH);
  digitalWrite(ledLevel3, LOW);
  digitalWrite(ledLevel2, LOW);
  digitalWrite(ledLevel1, LOW);
}
void riskLevel5() { // risk 5 blink ledLevel4 five times
  Serial.println("Risk level 5");
  int i;
  for (i = 0; i < 5; ++i) {
    riskLevel4(); // risk 5 uses ledLevel4
    delay(200);
    allOff(); // turn all leds off
    delay(200);
  }
  riskLevel4(); // keep ledLevel4 on

}
void allOn() { // turn all leds on
  digitalWrite(ledLevel4, HIGH);
  digitalWrite(ledLevel3, HIGH);
  digitalWrite(ledLevel2, HIGH);
  digitalWrite(ledLevel1, HIGH);
}
void allOff() { // turn all leds off
  digitalWrite(ledLevel4, LOW);
  digitalWrite(ledLevel3, LOW);
  digitalWrite(ledLevel2, LOW);
  digitalWrite(ledLevel1, LOW);
}

// Got the quicksort code from https://www.geeksforgeeks.org/selection-sort/
// Used this sort because it is the fastest sort and that is needed for
// fast accurate readings. Especially for a train.
void swap(int* a, int* b) { // takes two values a and b then swaps them
  // the stars change the values input into the function
  int t = *a; // temporary t become a
  *a = *b; // a become b
  *b = t; // b becomes temporary t
}

/* The main function that implements QuickSort
  arr[] --> Array to be sorted,
  low --> Starting index,
  high --> Ending index */
void quickSort(int arr[], int low, int high) {
  if (low < high) {
    /* pi is partitioning index, arr[p] is now
      at right place */
    int pi = partition(arr, low, high);

    // Separately sort elements before
    // partition and after partition
    quickSort(arr, low, pi - 1);
    quickSort(arr, pi + 1, high);
  }
}

/* This function takes last element as pivot, places
  the pivot element at its correct position in sorted
  array, and places all smaller (smaller than pivot)
  to left of pivot and all greater elements to right
  of pivot */
int partition (int arr[], int low, int high) {
  int pivot = arr[high]; // pivot
  int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far

  for (int j = low; j <= high - 1; j++) {
    // If current element is smaller than the pivot
    if (arr[j] < pivot) {
      i++; // increment index of smaller element
      swap(&arr[i], &arr[j]);
    }
  }
  swap(&arr[i + 1], &arr[high]);
  return (i + 1);
}

void triggerIFTTT() { // runs the code to trigger an IFTTT integration
  // publishes events on the particle console
  Particle.publish("Risk Level: ", String(riskLevel), PRIVATE);
  Particle.publish("Distance: ", String(distance), PRIVATE);
  if (riskLevel == 5) {
    // if a risk level of 5 is detected an alert event is sent to the particle console
    Particle.publish("Alert:", "risk5alert", PRIVATE);
    /* from there the IFTTT applet subscribes to the "Alert:" event and when it is set to
      "risk5alert" an email will be sent to the set email address in the applet*/
  }
}