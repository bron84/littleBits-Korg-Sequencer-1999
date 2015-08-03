const int stateButtonPin = 0;
const int programmerButtonPin = A1;
const int programmerValueButtonPin = A0;

const int digitalBeatPin = 1;
const int analogProgrammerStatePin = 5;
const int analogProgrammerValuePin = 9;

int stateButtonStatus = 0;
int stateButtonValue = 0;
int lastStateButtonPress;

int programmerButtonStatus = 0;
int programmerButtonValue = 0;
int lastProgrammerButtonPress;

int programmerValue = 0;

int sequencer[64]; 
int sequencerProgramPosition;
int sequencerProgramHighestPosition;
int sequencerPlayPosition;
boolean resetSequencerOnNextProgram = false;

int currentSequence = 0;
int currentSequencePosition = 0;
int sequences[5][16] = {
  {1},
  {1, 1, 1, 0},
  {1, 0, 1, 0},
  {1, 1, 0, 1, 1},
  {1, 1, 1, 0, 1, 1, 0}
};
int sequencesLength[5] = {1, 4, 4, 5, 7};

boolean metronomePlaying = false;
int metronomeMS = 500;
int lastMillis;
int triggerBeatOff;

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
  } else if (stateButtonValue == 2) {
    currentSequence = map(programmerValue, 0, 1020, 0, 3);
  }
}

void metronomeSpeed() {
  metronomeMS = map(programmerValue, 0, 1023, 1000, 100);
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
      if (currentSequencePosition > sequencesLength[currentSequencePosition]) {
        currentSequencePosition = 0;
      }

      if (sequences[currentSequence][currentSequencePosition]) {
        triggerSequencerBeat();
      }
      currentSequencePosition++;

      lastMillis = millis();
    }
  }
}

void triggerSequencerBeat() {
  Serial.println("Seq Beat");
  digitalWrite(digitalBeatPin, HIGH);
  
  analogWrite(analogProgrammerValuePin, sequencer[sequencerPlayPosition]);
  sequencerPlayPosition++;
  if (sequencerPlayPosition > sequencerProgramHighestPosition) {
    sequencerPlayPosition = 0;
  }
  if (triggerBeatOff < 0) {
    triggerBeatOff = millis() + 10;  
  }
  
}

void triggerMetronomeBeatOff() {
  if (triggerBeatOff > -1 && triggerBeatOff < millis()) {
    Serial.println("beat off");
    triggerBeatOff = -1;
    digitalWrite(digitalBeatPin, LOW);
  }
}
