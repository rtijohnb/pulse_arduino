/*  PulseSensor Starter Project and Signal Tester
    The Best Way to Get Started  With, or See the Raw Signal of, your PulseSensor.com™ & Arduino.
    Here is a link to the tutorial
    https://pulsesensor.com/pages/code-and-guide
    WATCH ME (Tutorial Video):
    https://www.youtube.com/watch?v=RbB8NSRa5X4
*/
#define codeversion 1 //version of the code running (used during initialization)
#define PulseSensorPin 0  // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
#define readingloop  406 //times to read the sensor before averaging it and printing it. 406 == 150 prints/sec (before psudo code addition)
#define numhistoricalreadings 450 //number of samples to store in the buffer
#define thresholdoveravg 150
#define samplespersec 150 //make sure this is accurate, it is used for BPM calculations
#define numhistoricalhbs 10 //number of historical heartbeats to track

int reading = 0;         // holds the incoming raw data. Signal value can range from 0-1024
short historicalbuffer[numhistoricalreadings]; //last 450 samples
short historicalindex = 0; //where to insert reading
int historicalavg = 0; //average of raw samples from the historical buffer
boolean upbeat = true;
short lastbeat = 0;//counter of the number of samples since last HB detected
short historicalhbs[numhistoricalhbs]; //buffer of last 10 HBs, value = number of readings between beats
byte hbindex = 0; //what is the current index to insert new values into the historicalhbs array
byte bpm = 0; //beats per minute
boolean throwawaybeat = true; //the first beat the sensor detects will likely have too high of a reading since the last (non existent) beat, don't use this beat

byte numhbnotdetected = 0; //number of times a hb hasn't been detected in a row (2 seconds x hbnotdetected = seconds where a hb wasn't detected). Used to enable pseudo output
short pseudoraw[112] = {430, 432, 434, 437, 438, 441, 442, 444, 445, 444, 443, 442, 441, 441, 444, 452, 466, 488, 519, 557, 607, 666, 729, 791, 857, 907, 950, 981, 995, 996, 996, 990, 970, 945, 917, 886, 853,
                        815, 776, 739, 703, 666, 635, 604, 576, 549, 525, 503, 483, 464, 449, 432, 420, 410, 403, 398, 395, 397, 402, 409, 417, 426, 437, 449, 461, 473, 484, 494, 503, 511, 518, 523, 526, 527,
                        526, 523, 519, 509, 500, 490, 479, 468, 456, 445, 436, 428, 422, 416, 408, 402, 396, 392, 389, 388, 389, 390, 392, 393, 394, 396, 399, 400, 402, 404, 406, 409, 412, 416, 421, 424, 426, 428
                       };
short pseudoavg[112] = {504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 505, 505, 506, 506, 507, 509, 510, 511, 512, 514, 515, 516, 518, 519, 520, 521, 522,
                        523, 524, 525, 525, 526, 526, 527, 527, 527, 528, 528, 528, 528, 528, 528, 528, 528, 528, 528, 528, 527, 527, 527, 527, 527, 527, 527, 527, 527, 527, 527, 528, 528, 528, 528, 528, 528,
                        528, 529, 529, 529, 529, 528, 528, 528, 527, 526, 525, 524, 523, 522, 521, 520, 519, 517, 516, 513, 512, 510, 508, 508, 507, 507, 507, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506
                       };
short pseudothresh[112] = {654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 654, 655, 655, 656, 656, 657, 659, 660, 661, 662, 664, 665, 666, 668, 669, 670, 671, 672,
                           673, 674, 675, 675, 676, 676, 677, 677, 677, 678, 678, 678, 678, 678, 678, 678, 678, 678, 678, 678, 677, 677, 677, 677, 677, 677, 677, 677, 677, 677, 677, 678, 678, 678, 678, 678, 678,
                           678, 679, 679, 679, 679, 678, 678, 678, 677, 676, 675, 674, 673, 672, 671, 670, 669, 667, 666, 663, 662, 660, 658, 658, 657, 657, 657, 656, 656, 656, 656, 656, 656, 656, 656, 656, 656, 656
                          };

// The SetUp Function:
void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // pin that will blink to your heartbeat
  Serial.begin(115200);        // Set's up Serial Communication at certain speed.

  for (byte i = 0; i < codeversion; i++) { //blink the onboard led for 1 second to show what code version is running
    digitalWrite(LED_BUILTIN, HIGH); //led off
    delay(500);
    digitalWrite(LED_BUILTIN, LOW); //led on
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH); //led off
    delay(500);
  }

  delay(4000); //give the user time to connect to the serial port to read the version
  Serial.print("version ");
  Serial.println(codeversion);
}

// The Main Loop Function
void loop() {
  reading = 0; //raw pulse level read from the ADC, reseet to 0 prior to reading it (since we use +=)
  for (short i = 0; i < readingloop; i++) { //read the raw ADC level multiple times to slow down overall output to ~150 samples/sec
    reading += analogRead(PulseSensorPin); // Read the PulseSensor's value, add it to the 'reading' value
  }
  reading /= readingloop; //now reading is averaged out across multiple reads, ready to print raw and use to calculate averages and HBs

  //store reading in buffer, and calculate average
  historicalbuffer[historicalindex] = reading; //store reading in history buffer
  historicalindex++; //move to next position in the array
  historicalindex %= numhistoricalreadings; //roll over if necessary
  historicalavg = 0; //reset average
  for (short i = 0; i < numhistoricalreadings; i++) { //calculate the current average
    historicalavg += historicalbuffer[i];
  }
  historicalavg /= numhistoricalreadings; //divide by the number of readings we sumed up

  //detect beat logic: set a threshold that is 'thresholdoveravg' above the average
  //                   if reading is above threshold and starts to fall (compared to 3 samples ago) detect a HB
  short threesamplesago = historicalindex - 3;
  if ( threesamplesago < 0 ) {
    threesamplesago = numhistoricalreadings + threesamplesago;  //wrap around if the value is negative (range: 0 to numhistoricalreadings-1)
  }

  if (upbeat && reading > (historicalavg + thresholdoveravg) //if reading is increasing (or before a HB), and reading is above the threshold
      && reading < historicalbuffer[threesamplesago]) { //and the reading was just lower than 3 samples ago (just started to fall) then peak of HB detected
    if (throwawaybeat) { //don't record the time since last HB, on the first HB we detect (since it will likely be wrong
      throwawaybeat = false; //but record the next one
    } else { //if this is the second (or more) HB in a row, use it
      historicalhbs[hbindex] = lastbeat; //store the number of samples read since last HB was detected
    }
    hbindex++; //increment index
    hbindex %= numhistoricalhbs; //wrap around to 0 if needed
    upbeat = false; //prevent detection of a new HB until we fall under the threshold
    lastbeat = 0; //clear counter for the number of samples until the next HB
    numhbnotdetected = 0; //we detected a hb, clear this counter. used for pseudo output

    //calculate BPM
    short numsamplesbetweenhbs = 0;
    for (byte i = 0; i < numhistoricalhbs; i++) { //add up the number of samples between the last HBs we are recording
      if (historicalhbs[i] == 0) { //if we havent recorded 10 hbs yet
        numsamplesbetweenhbs = 0;  //bail out
        bpm = 0; //we won't have a correct bpm reading
        break;
      } else { //if we have detected 10 heartbeats
        numsamplesbetweenhbs += historicalhbs[i];
      }
    }//for() add up samples since last hb

    numsamplesbetweenhbs /= numhistoricalhbs; //average it out over the last HBs we recorded
    if (numsamplesbetweenhbs != 0) { //divide by 0 check
      bpm = (samplespersec * 60) / (numsamplesbetweenhbs);
    }

  } else if (!upbeat && reading < historicalavg) { //we fell back below the average after a peak
    upbeat = true; //now start looking for the next peak
  }//endif hb detected

  lastbeat++; //how many samples were recorded since last HB
  if (lastbeat > (samplespersec * 2)) { //if we haven't detected a HB in 2 seconds, something is wrong
    lastbeat = 0; //clear lastbeat (so it doesn't roll over, not strictly necessary)
    bpm = 0; //clear bpm
    throwawaybeat = true; //when we detect the first HB, don't use the time since last HB since it will likely be wrong
    historicalhbs[hbindex] = 0; //add a 0 to the array so we will have to wait for 10 good readings before calculating again
    hbindex++; //increment index
    hbindex %= numhistoricalhbs; //wrap around to 0 if needed

    if(numhbnotdetected <= 13){ //if we aren't detecting samples for up to 13x 2sec periods
      numhbnotdetected++; //we didn't detect a hb for a certain amount of time, increment counter, once it reaches 14 we will print pseudo samples
    }
  }
  if (numhbnotdetected <= 13) { //if we are not printing pseudo samples
    Serial.print("raw:");
    Serial.print(reading); //print out this sample
    Serial.print(" avg:");
    Serial.print(historicalavg); //avg
    Serial.print(" thresh:");
    Serial.print(historicalavg + thresholdoveravg); //high threshold
    Serial.print(" bpm:");
    Serial.println(bpm);
    
    if (reading > historicalavg + thresholdoveravg) { //light up the built in LED when the reading is over the threshold
      digitalWrite(LED_BUILTIN, LOW); // turn on led
    } else {
      digitalWrite(LED_BUILTIN, HIGH); // turn off led
    }
  } else { // >= 14 if we didn't detect a hb for 30 sec, used for pseudo output
    //numhbnotdetected 14-126 is pseudo sample (112 samples)
    byte tempindex = numhbnotdetected - 14; //calc ahead of time
    Serial.print("raw:");
    Serial.print(pseudoraw[tempindex]);
    Serial.print(" avg:");
    Serial.print(pseudoavg[tempindex]);
    Serial.print(" thresh:");
    Serial.print(pseudothresh[tempindex]);
    Serial.println(" bpm:80");
    //blink leds quickly
    if( (tempindex/14)%2 ){ //break up the psuedo hb into 8 time segments, blink led
      digitalWrite(LED_BUILTIN, LOW); // turn on led
    }else{
      digitalWrite(LED_BUILTIN, HIGH); // turn off led
    }
        
    numhbnotdetected++; //increment to move to next pseudo sample
    if(numhbnotdetected == 126) { //if we've printed out 150 pseudo samples, loop back and do it again
      numhbnotdetected = 14;//reset counter and start printing the sample again
    }
  }//numhbnotdetected check to see if we should print pseudo HBs

}
