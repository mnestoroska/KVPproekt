
// Дефинирање на пиновите
const int pump1Pin = 11; // Потопна пумпа во првиот сад
const int pump2Pin = 9;  // Обична пумпа за кисела течност
const int pump3Pin = 12; // Потопна пумпа за базна течност
const int pump4Pin = 8;  // Обична пумпа за испразнување
const int motorPin = 10; // Мотор за маталка

const int sensorLow1 = A1;  // Сензор за низок водостој во првиот сад
const int sensorOverflow = A2; // Сензор за прелевање во третиот сад
const int sensorLow2 = A3;  // Сензор за низок водостој во третиот сад

bool processRunning = false; // Дали процесот е во тек
unsigned long previousMillis = 0; // Променлива за следење на времето
unsigned long interval = 15000;  // Време на работа на пумпите (10 секунди)

void setup() {
  pinMode(pump1Pin, OUTPUT);
  pinMode(pump2Pin, OUTPUT);
  pinMode(pump3Pin, OUTPUT);
  pinMode(pump4Pin, OUTPUT);
  pinMode(motorPin, OUTPUT);

  pinMode(sensorLow1, INPUT);
  pinMode(sensorOverflow, INPUT);
  pinMode(sensorLow2, INPUT);

  digitalWrite(pump1Pin, HIGH);
  digitalWrite(pump2Pin, LOW);
  digitalWrite(pump3Pin, HIGH);
  digitalWrite(pump4Pin, LOW);
  digitalWrite(motorPin, HIGH);

  Serial.begin(9600);
  Serial.println("System Ready. Waiting for command...");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.equalsIgnoreCase("START") && !processRunning) {
      Serial.println("Starting the process...");
      processRunning = true;
      runProcess();
    } 
    else if (command.equalsIgnoreCase("STOP") && processRunning) {
      Serial.println("Stopping the process...");
      stopProcess();
      processRunning = false;
    } 
    else if (!command.equalsIgnoreCase("START") && !command.equalsIgnoreCase("STOP")) {
      Serial.println("Invalid command. Please type 'START' to begin or 'STOP' to end.");
    }
  }

  if (processRunning) {
    if (checkStopCommand()) return;
  }
}

// Функција за извршување на процесот
void runProcess() {
  if (checkWaterLevels()) {
    Serial.println("Water liquid is too low! Process cannot continue.");
    return;
  }

  Serial.println("Filling the third container...");
  digitalWrite(pump1Pin, LOW);
  
  while (analogRead(sensorOverflow) < 500) {
    if (checkStopCommand() || checkWaterLevels()) {
      digitalWrite(pump1Pin, HIGH);
      return;
    }
  }

  digitalWrite(pump1Pin, HIGH);
  get_pH_value: // Label for jumping back if invalid pH is entered
  Serial.println("Enter pH value (0.0 - 14.0):");

  while (Serial.available() == 0) {
    if (checkStopCommand()) return;
  }

  float pHState = Serial.parseFloat();

  if (pHState < 0.0 || pHState > 14.0) {
    Serial.println("Invalid pH value! Must be between 0.0 and 14.0. Please enter again.");
    goto get_pH_value; 
  }

  Serial.print("pH Value: ");
  Serial.println(pHState);
  
  // Печатење на вредности на сензорите
  int waterLevel1 = analogRead(sensorLow1);
  int waterLevel2 = analogRead(sensorLow2);
  Serial.print("Water Level Sensor 1: ");
  Serial.println(waterLevel1);
  Serial.print("Water Level Sensor 2: ");
  Serial.println(waterLevel2);
  
  if (pHState >= 0 && pHState < 2.2) {
    Serial.println("Adding base to neutralize...");
    digitalWrite(pump3Pin, LOW);  // Додавање базна течност
    delayWithStop(interval-6000);       // Чекање
    digitalWrite(pump3Pin, HIGH); // Исклучување на пумпата
    Mix();
  } 
    else if (pHState >= 2.2 && pHState < 4.5) {
      Serial.println("Adding base to neutralize...");
      digitalWrite(pump3Pin, LOW);  // Додавање базна течност
      delayWithStop(interval-7000);       // Чекање
      digitalWrite(pump3Pin, HIGH); // Исклучување на пумпата
      Mix();
    } 
    else if (pHState >= 4.5 && pHState < 6.7) {
       Serial.println("Adding base to neutralize...");
       digitalWrite(pump3Pin, LOW);  // Додавање базна течност
       delayWithStop(interval-8500);       // Чекање
       digitalWrite(pump3Pin, HIGH); // Исклучување на пумпата
       Mix();
    } 
    else if (pHState >= 7.3 && pHState < 9.5) {
      Serial.println("Adding acid to neutralize...");
      digitalWrite(pump2Pin, HIGH);  // Додавање кисела течност
      delayWithStop(interval);       // Чекање
      digitalWrite(pump2Pin, LOW);   // Исклучување на пумпата
      Mix();
    }
    else if (pHState >= 9.5 && pHState < 11.8) {
      Serial.println("Adding acid to neutralize...");
      digitalWrite(pump2Pin, HIGH);  // Додавање кисела течност
      delayWithStop(interval+1000);       // Чекање
      digitalWrite(pump2Pin, LOW);   // Исклучување на пумпата
      Mix();
    }
    else if (pHState >= 11.8 && pHState < 14) {
      Serial.println("Adding acid to neutralize...");
      digitalWrite(pump2Pin, HIGH);  // Додавање кисела течност
      delayWithStop(interval+2000);       // Чекање
      digitalWrite(pump2Pin, LOW);   // Исклучување на пумпата
      Mix();
      
    } 
    else if (pHState >= 6.7 && pHState < 7.3) {
      Serial.println("pH is neutral. No adjustment needed.");
    } 
    else if (pHState < 0.0 || pHState > 14.0) {
      Serial.println("Invalid pH value. Must be between 0.0 and 14.0.");
      
    }
  // Испразнување на садот без проверка за водостој
  Serial.println("Draining the third container...");
  digitalWrite(pump4Pin, HIGH);
  delay(200000); // Чекање 200 секунди за испразнување
  digitalWrite(pump4Pin, LOW);

  // Ова проверува ако е потребно да се стопира процесот по испразнување на садот
  if (checkWaterLevels()) {
    Serial.println("Low liquid level detected after draining. Please add water and use 'START' to begin again.");
    processRunning = false;
    stopProcess();
  }

  Serial.println("Process completed.");
  processRunning = false;
}
// Функција за запирање на процесот
void stopProcess() {
  digitalWrite(pump1Pin, HIGH);
  digitalWrite(pump2Pin, LOW);
  digitalWrite(pump3Pin, HIGH);
  digitalWrite(pump4Pin, LOW);
  digitalWrite(motorPin, HIGH);
}


// Функција која проверува дали е внесен "STOP"
bool checkStopCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equalsIgnoreCase("STOP")) {
      stopProcess();
      Serial.println("Process stopped.");
      processRunning = false;
      return true;
    }
  }
  return false;
}

// Функција која проверува нивото на водата и го стопира процесот ако е потребно
bool checkWaterLevels() {
  int waterLevel1 = analogRead(sensorLow1);
  int waterLevel2 = analogRead(sensorLow2);

  // Печатење на вредностите за водостој за да се провериме дали се точни
  Serial.print("Water Level Sensor 1: ");
  Serial.println(waterLevel1);
  Serial.print("Water Level Sensor 2: ");
  Serial.println(waterLevel2);

  // Прилагодете ги овие вредности според вашите сензори
  if (waterLevel1 < 400 || waterLevel2 < 400) {
    return true; // Ако било кое ниво на водата е премалку, процесот ќе запре
  }
  return false;
}
// Мешање на раствор
void Mix(){
  Serial.println("Mixing the solution...");
  digitalWrite(motorPin, LOW);  // Вклучување на моторот
  delay(10000);                 // Чекање 10 секунди за мешање
  digitalWrite(motorPin, HIGH); // Исклучување на моторот
  return;
}
// Функција за одложување со проверка за STOP команда
void delayWithStop(unsigned long duration) {
  unsigned long startMillis = millis();
  while (millis() - startMillis < duration) {
    if (checkStopCommand() || checkWaterLevels()) {
      return;
    }
  }
}
