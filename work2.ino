const int btnPin = 2;    // 按鈕接在 D2
const int yellowPin = 11; // 黃燈接在 D11
const int redPin = 10;   // 紅燈接在 D10
const int bluePin = 9;   // 藍燈接在 D9

int pressCount = 0;      // 記錄按按鈕次數（控制黃燈亮度，共 4 級）
unsigned long lastInteractTime = 0; // 記錄上一次按按鈕的時間
bool buttonIsPressed = false;

// 紅色閃爍相關變數
unsigned long lastBlinkTime = 0;
bool redState = false;

// 新增：瘋狂派對模式（連續按 15 下）相關變數
bool isPartyMode = false;
unsigned long partyStartTime = 0;
unsigned long lastPartyFlashTime = 0;
int rapidClickCount = 0;
unsigned long lastSingleClickTime = 0;

void setup() {
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP); // 內建提升電阻，按下去為 LOW
  Serial.begin(9600);
  lastInteractTime = millis();
  randomSeed(analogRead(0)); // 讓隨機亂數每次都不同
}

void loop() {
  unsigned long currentTime = millis();
  int buttonState = digitalRead(btnPin);

  // 1. 偵測按鈕按下
  if (buttonState == LOW && !buttonIsPressed) {
    buttonIsPressed = true;
    
    // 如果目前不在派對模式中，計算連續點擊次數
    if (!isPartyMode) {
      if (currentTime - lastSingleClickTime < 1000) {
        rapidClickCount++; // 1秒內連續按，累積次數
      } else {
        rapidClickCount = 1; // 超過1秒沒按就重新計算連續次數
      }
      lastSingleClickTime = currentTime;

      // 如果連續快速按滿 15 下 -> 觸發派對模式！
      if (rapidClickCount >= 15) {
        isPartyMode = true;
        partyStartTime = currentTime;
        rapidClickCount = 0; // 重置計數
      } else {
        // 一般點擊：增加黃燈亮度級距，並重置 5/10 秒計時器
        pressCount = (pressCount % 4) + 1; 
        lastInteractTime = currentTime;
        
        redState = false;
        digitalWrite(redPin, LOW);
      }
    }
    
    delay(200); // 防彈跳
  }

  // 記錄放開按鈕
  if (buttonState == HIGH && buttonIsPressed) {
    buttonIsPressed = false;
  }

  // 計算距離上次正常互動經過的時間
  unsigned long elapsed = currentTime - lastInteractTime;

  // 2. 狀態判斷與燈光控制
  if (isPartyMode) {
    // **新功能：派對模式（連續按15下觸發，持續 7 秒）**
    if (currentTime - partyStartTime >= 7000) {
      isPartyMode = false;        // 7 秒到，關閉派對模式
      lastInteractTime = currentTime; // 重置計時器，回到正常 0-5 秒狀態
      digitalWrite(yellowPin, LOW);
      digitalWrite(redPin, LOW);
      digitalWrite(bluePin, LOW);
    } else {
      // 每 100 毫秒隨機改變三顆燈的亮度，製造興奮錯亂的霓虹燈效果！
      if (currentTime - lastPartyFlashTime >= 100) {
        lastPartyFlashTime = currentTime;
        analogWrite(yellowPin, random(0, 256));
        analogWrite(redPin, random(0, 256));
        analogWrite(bluePin, random(0, 256));
      }
    }
  }
  else if (elapsed > 10000) {
    // 超過 10 秒：紅燈閃爍，越久閃越快
    digitalWrite(yellowPin, LOW);
    digitalWrite(bluePin, LOW);
    
    unsigned long redTime = elapsed - 10000;
    int blinkInterval = 300 - (redTime / 200); 
    if (blinkInterval < 60) {
      blinkInterval = 60; 
    }

    if (currentTime - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentTime;
      redState = !redState;
      digitalWrite(redPin, redState ? HIGH : LOW);
    }
  } 
  else if (elapsed > 5000) {
    // 超過 5 秒（但在 10 秒內）：變成藍燈
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
    digitalWrite(bluePin, HIGH);
    redState = false;
  } 
  else {
    // 5 秒內（正常狀態）：黃燈亮，按越多次越亮
    digitalWrite(redPin, LOW);
    digitalWrite(bluePin, LOW);
    redState = false;

    int brightness = 60; 
    if (pressCount == 1) brightness = 60;   
    else if (pressCount == 2) brightness = 120; 
    else if (pressCount == 3) brightness = 190; 
    else if (pressCount == 4) brightness = 255; 

    analogWrite(yellowPin, brightness);
  }
}