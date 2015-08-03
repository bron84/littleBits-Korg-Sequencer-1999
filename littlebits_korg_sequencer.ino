const int stateButtonPin = 0;
const int programmerButtonPin = A1;
const int programmerValueButtonPin = A0;

const int digitalBeatPin = 1;
const int analogProgrammerStatePin = 5;
const int analogProgrammerValuePin = 9;

int stateButtonStatus = 0;
int stateButtonValue = 0;
unsigned long lastStateButtonPress;

int programmerButtonStatus = 0;
int programmerButtonValue = 0;
unsigned long lastProgrammerButtonPress;

int programmerValue = 0;

int sequencer[64]; 
int sequencerProgramPosition;
int sequencerProgramHighestPosition;
int sequencerPlayPosition;
boolean resetSequencerOnNextProgram = false;

boolean metronomePlaying = false;
boolean metronomeCanTrack = false;
int metronomeMSAnalog;
unsigned long metronomeMS = 500;
unsigned long lastMillis;
unsigned long triggerBeatOff;

void setup() {
  Serial.begin(9600);
  pinMode(stateButtonPin, INPUT);
  pinMode(programmerButtonPin, INPUT);
  pinMode(programmerValueButtonPin, INPUT);

  pinMode(digitalBeatPin, OUTPUT);
  pinMode(analogProgrammerStatePin, OUTPUT);
  pinMode(analogProgrammerValuePin, OUTPUT);
}

void loop() {
  stateValueHandler();
  stateButtonHandler();
  stateProgrammerHandler();
  triggerMetronome();
  triggerMetronomeBeatOff();
}

void stateButtonHandler() {
  int tmpButtonState = digitalRead(stateButtonPin);

  if (stateButtonStatus != tmpButtonState) {
    if (tmpButtonState == HIGH && (millis() - lastStateButtonPress > 100)) {
      lastStateButtonPress = millis();
      
      stateButtonValue++;
      if (stateButtonValue >= 5) {
        stateButtonValue = 0;
      }
      if (stateButtonValue != 1) {
        resetSequencerOnNextProgram = true;
      }
      if (stateButtonValue == 3) {
          metronomeCanTrack = false;
      }
      
    }
  }
  stateButtonStatus = tmpButtonState;
  analogWrite(analogProgrammerStatePin, map(stateButtonValue, 0, 5, 0, 255));
}

void stateProgrammerHandler() {
  int tmpButtonState = digitalRead(programmerButtonPin);

  if (programmerButtonStatus != tmpButtonState) {
    if (tmpButtonState == HIGH && (millis() - lastProgrammerButtonPress > 100)) {
      lastProgrammerButtonPress = millis();
      
      switch (stateButtonValue) {
        case 1:
          if (resetSequencerOnNextProgram) {
            sequencerProgramPosition = 0;
            sequencerProgramHighestPosition = 0;
            resetSequencerOnNextProgram = false;  
          }
          if (sequencerProgramPosition >= 64) {
            break;
          }
        
          sequencer[sequencerProgramPosition] = programmerValue;
          sequencerProgramHighestPosition = sequencerProgramPosition;
          sequencerProgramPosition++;

          break;
        case 3:
          metronomeToggle();
          break;
      }
    }
  }
  programmerButtonStatus = tmpButtonState;
}

void stateValueHandler() {
   programmerValue = analogRead(programmerValueButtonPin);

  if (stateButtonValue == 3) {
    metronomeSpeed();
  }   
}

void metronomeSpeed() {
  int diff = abs(metronomeMSAnalog - programmerValue);
  if (diff < 20) {
    metronomeCanTrack = true;
  }

  if (metronomeCanTrack) {
    metronomeMSAnalog = programmerValue;
    metronomeMS = map(metronomeMSAnalog, 0, 1023, 1000, 100);
  }
}

void metronomeToggle() {
  Serial.println("toggle metronome");
  metronomePlaying = !metronomePlaying;
  lastMillis = millis() - metronomeMS;
}

void triggerMetronome() {
  if (metronomePlaying) {
    int diffMillis = millis() - lastMillis;
    if (diffMillis > metronomeMS) {
      digitalWrite(digitalBeatPin, HIGH);

      analogWrite(analogProgrammerValuePin, sequencer[sequencerPlayPosition]);
      sequencerPlayPosition++;
      if (sequencerPlayPosition > sequencerProgramHighestPosition) {
        sequencerPlayPosition = 0;
      }
      triggerBeatOff = millis() + 10;
      lastMillis = millis();
    }
  }
}

void triggerMetronomeBeatOff() {
  
  if (triggerBeatOff > 0 && triggerBeatOff < millis()) {
    digitalWrite(digitalBeatPin, LOW); 
    triggerBeatOff = 0;
  }
}
