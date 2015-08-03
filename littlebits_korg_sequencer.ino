const int stateButtonPin = 0;
const int programmerButtonPin = A1;
const int programmerValueButtonPin = A0;

const int digitalBeatPin = 1;
const int analogProgrammerStatePin = 5;
const int analogProgrammerValuePin = 9;

const int programState = 1;
const int sequenceState = 2;
const int sequence2State = 3;
const int metronomeState = 0;


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

int currentSequence[2] = {0, 0};
int currentSequenceAnalog[2];
boolean currentSequenceCanTrack[2];
int currentSequencePosition[2];
int sequences[5][16] = {
  {1},
  {1, 1, 1, 0},
  {1, 0, 1, 0},
  {1, 1, 0, 1, 1},
  {1, 1, 1, 0, 1, 1, 0}
};
int sequencesLength[5] = {1, 4, 4, 5, 7};
int numberOfSequences = 5;


boolean metronomePlaying = false;
boolean metronomeCanTrack = false;
int metronomeMSAnalog;
unsigned long metronomeMS = 1000;
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
      if (stateButtonValue >= 4) {
        stateButtonValue = 0;
      }
      if (stateButtonValue != programState) {
        resetSequencerOnNextProgram = true;
      }
      if (stateButtonValue == metronomeState) {
          metronomeCanTrack = false;
      } else if (stateButtonValue == sequenceState) {
          currentSequenceCanTrack[0] = false;
      } else if (stateButtonValue == sequence2State) {
          currentSequenceCanTrack[1] = false;
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
        case programState:
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
        case metronomeState:
          metronomeToggle();
          break;
      }
    }
  }
  programmerButtonStatus = tmpButtonState;
}

void stateValueHandler() {
   programmerValue = analogRead(programmerValueButtonPin);

  if (stateButtonValue == metronomeState) {
    metronomeSpeed();
  } else if (stateButtonValue == sequenceState) {
    sequenceNumber(0);
  } else if (stateButtonValue == sequence2State) {
    sequenceNumber(1);
  }
}

void metronomeSpeed() {
  int diff = abs(metronomeMSAnalog - programmerValue);
  if (diff < 20) {
    metronomeCanTrack = true;
  }

  if (metronomeCanTrack) {
    metronomeMSAnalog = programmerValue;
    metronomeMS = map(metronomeMSAnalog, 0, 1023, 1000, 20);
  }
}

void sequenceNumber(int number) {
  int diff = abs(currentSequenceAnalog[number] - programmerValue);
  if (diff < 20) {
    currentSequenceCanTrack[number] = true;
  }
  
  if (currentSequenceCanTrack[number]) {
    currentSequenceAnalog[number] = programmerValue;
    currentSequence[number] = map(currentSequenceAnalog[number], 0, 1020, 0, numberOfSequences - 1); 
  }
}

void metronomeToggle() {
  metronomePlaying = !metronomePlaying;
  lastMillis = millis() - metronomeMS;
}

void triggerMetronome() {
  if (metronomePlaying) {
    int diffMillis = millis() - lastMillis;
    if (diffMillis > metronomeMS) {
      if (currentSequencePosition[0] >= sequencesLength[currentSequence[0]]) {
        currentSequencePosition[0] = 0;
      }

      if (sequences[currentSequence[0]][currentSequencePosition[0]]) {
        triggerSequencerBeat();
      }
      currentSequencePosition[0]++;

      if (currentSequencePosition[1] >= sequencesLength[currentSequence[1]]) {
        currentSequencePosition[1] = 0;
      }

      if (sequences[currentSequence[1]][currentSequencePosition[1]]) {
        digitalWrite(digitalBeatPin, HIGH);
        triggerBeatOff = millis() + 10;  
      }
      currentSequencePosition[1]++;      

      lastMillis = millis();
    }
  }
}

void triggerSequencerBeat() {
      

      analogWrite(analogProgrammerValuePin, sequencer[sequencerPlayPosition]);
      sequencerPlayPosition++;
      if (sequencerPlayPosition > sequencerProgramHighestPosition) {
        sequencerPlayPosition = 0;
      }
      
}
  

void triggerMetronomeBeatOff() {
  
  if (triggerBeatOff > 0 && triggerBeatOff < millis()) {
    digitalWrite(digitalBeatPin, LOW); 
    triggerBeatOff = 0;
  }
}
