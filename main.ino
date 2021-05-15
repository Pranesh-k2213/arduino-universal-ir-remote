// Read connections.txt for guidelines to connect components
//
// Chaging the value of debug to 1 in line 23
// will display values of various variables in LCD module 
// and in serial display which will be help full to debug
// 
// So far this program supports SONY, SAMSUNG, RC6 and NEC protocols
//
// This program can store hex code of 20 buttons (5 buttons for each remote)
//

#include<LiquidCrystal_I2C.h>
#include <IRremote.h>
#include<string.h>

#define EXCLUDE_EXOTIC_PROTOCOLS
#define RECEIVER_PIN 2
#define SENDER_PIN 8
#define inx A0
#define iny A1
#define jbut A2
#define dtime 325
#define debug 0

LiquidCrystal_I2C lcd(0x27, 16, 2);
IRrecv receiver(RECEIVER_PIN);
decode_results results;
IRsend irsend(SENDER_PIN);
int currRem = 0;
int currPos = 0;
int screen = 0;
void display1();
void updatePos(char );
void transmit(unsigned long, int);
void action();
String names[] = {"TV", "SETUP-B", "DVD", "AC", ""};
String buttons[] = {" add ", "Power", " ch+ ", " ch- ", " vl+ ", " vl- ", "temp+", "temp-", " fn+ ", " fn- ", "back", ""};
int notin[11];
int lnotin = 0;

uint8_t leftArrow[8]  = {
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111,
  0b01111,
  0b00111,
  0b00011,
};

uint8_t rightArrow[8] = {
  0b10000,
  0b11000,
  0b11100,
  0b11110,
  0b11111,
  0b11110,
  0b11100,
  0b11000,
};


class Remote {
  public:
  int noKeys = 1;
  int keys[6] = {0, 0, 0, 0, 0, 0};
  unsigned long keyValue[6];
  int type[6];
  void bAdd(int pos) {
    #if debug == 1
      Serial.println("in add func");
      Serial.print("noKeys:");
      Serial.println(noKeys);
    #endif
    if (noKeys <= 5){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("listeing");
      receiver.start();
      for(;;) {
        delay(1);
        if (receiver.decode(&results) && results.value != 0XFFFFFFFF)
            break;
      }
      receiver.stop();
      keys[noKeys] = pos;
      keyValue[noKeys] = results.value;
      type[noKeys] = results.decode_type;
      noKeys += 1;
      #if debug == 1
      Serial.println(results.value, HEX);
      #endif
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("NO");
      lcd.setCursor(0, 1);
      lcd.print("Memory");
      delay(2000);
    }
  }
} remote[4];



void setup() {
 #if debug == 1
  Serial.begin(9600);
 #endif
  pinMode(LED_BUILTIN, OUTPUT);
  receiver.enableIRIn();
  receiver.stop();
  lcd.begin();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, leftArrow);
  lcd.createChar(1, rightArrow);
  lcd.setCursor(0, 0);
#if debug == 1
    remote[0].keys[1] = 5;
    remote[0].noKeys = 2;
#endif
  lcd.print("Uni Remote");
  lcd.setCursor(12, 1);
  lcd.write(0);
  lcd.write(1);
  delay(2000);
  
}



void loop() {
  int x = analogRead(inx);
  int y = analogRead(iny);
  int b = analogRead(jbut);
  if (screen == 0) {
    if(y < 200) {
      currRem = (currRem > 0) ? (currRem-1) : currRem;
      currPos = 0;
      display1();
      delay(dtime);
    } else if (y > 850) {
      currRem = (currRem < 3) ? (currRem+1) : currRem;
      currPos = 0;
      display1();
      delay(dtime);
    } else if (x > 850) {
      updatePos('r');
      display1();
      #if debug == 1
        lcd.setCursor(14, 1);
        lcd.write(1);
      #endif
      delay(dtime);
    } else if (x < 200) {
      updatePos('l');
      display1();
      #if debug == 1
        lcd.setCursor(14, 1);
        lcd.write(0);
      #endif
      delay(dtime);
    } else if (b < 100) {
      action();
      display1();
      delay(dtime);
    }
  } else {
      if(y < 200) {
        updatePos('u');
        display1();
        delay(dtime);
      } else if (y > 850) {
        updatePos('d');
        display1();
        delay(dtime);
      } else if (b<100) {
        action();
        display1();
        delay(dtime);
      }
  } 
}


void display1() {
  if(screen == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(names[currRem]);
    lcd.setCursor(8, 0);
    lcd.write(0);
    lcd.print(buttons[remote[currRem].keys[currPos]]);
    lcd.write(1);
    #if debug == 1
      lcd.setCursor(10, 1);
      lcd.print(currPos);
      lcd.print(" ");
      lcd.print(currRem);
    #endif
    lcd.setCursor(0, 1);
    lcd.print(names[currRem + 1]);
    if (currPos != 0 ) {
      lcd.setCursor(8, 1);
      lcd.print(remote[currRem].keyValue[currPos], HEX);
    }
  } else if (screen == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(buttons[notin[currPos]]);
    lcd.setCursor(0, 1);
    lcd.print(buttons[notin[currPos+1]]);
    lcd.setCursor(6, 0);
    lcd.print("<---");
    
  }
  
}

void updatePos(char a) {
  if (a == 'l') {
    currPos = (currPos > 0)? currPos-1: currPos; 
  } else if (a == 'r') {
    currPos = (currPos < remote[currRem].noKeys-1)? currPos+1 : currPos;
  } else if (a == 'd') {
    currPos = (currPos < lnotin-2)? currPos+1 : currPos;
  } else if (a == 'u') {
    currPos = (currPos >0) ? currPos-1: currPos;
  }
}




void action() {
  if (screen == 0) {
    if (currPos == 0) {
      screen = 1;
      for (int i = currPos; i<=11; i++) {
        bool test = true;
        for(int j = 0; j<6; j++) {
          if (i == remote[currRem].keys[j]) 
            test = false;
        }
        if(test) {
          notin[lnotin] = i;
          ++lnotin;
        }
      }
    } else {
      transmit(remote[currRem].keyValue[currPos], remote[currRem].type[currPos]);
    }
  } else {
    if (buttons[notin[currPos]] == "back") {
      screen = 0;
      lnotin = 0;
      currPos = 0;
    } else {
      screen = 0;
      lnotin = 0;
      int temp = 0;
      for(;temp<11;temp++) {
        if(notin[currPos] == temp)
          break;
      }
      remote[currRem].bAdd(temp);
      currPos = 1;
    }
  }
}

void transmit(unsigned long code, int prot){
  switch (prot) {
    case SONY:
      irsend.sendSony(code, 20);
      #if debug == 1
      Serial.println("Sending sony");
      #endif
      digitalWrite(LED_BUILTIN, HIGH);
      delay(dtime);
      digitalWrite(LED_BUILTIN, LOW);
      break;
    case RC6:
      irsend.sendRC6(code, 20);
      #if debug == 1
      Serial.println("Sending RC6");
      #endif
      digitalWrite(LED_BUILTIN, HIGH);
      delay(dtime);
      digitalWrite(LED_BUILTIN, LOW);
      break;
    case NEC:
      irsend.sendNEC(code, 20);
      #if debug == 1
      Serial.println("Sending NEC");
      #endif
      digitalWrite(LED_BUILTIN, HIGH);
      delay(dtime);
      digitalWrite(LED_BUILTIN, LOW);
      break;
    case SAMSUNG:
      irsend.sendSAMSUNG(code, 20);
      #if debug == 1
      Serial.println("Sending SAMSUNG");
      #endif
      digitalWrite(LED_BUILTIN, HIGH);
      delay(dtime);
      digitalWrite(LED_BUILTIN, LOW);
      break; 
  }
}
