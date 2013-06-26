
#include <Servo.h> 
#include <AFMotor.h>

#define ACCEL_STEPS 15
#define LEFT 0
#define MIDDLE 1
#define RIGHT 2

#define HOME 0
#define SERVO 0
#define SERVO_HIGH 60
#define SERVO_LOW 150
#define DEBUG 1
// 0:12;5:32;11:1;2:10;          // test string
// 3:1;7:2;8:2;9:2;10:2;11:2;12:2;
boolean drinkAvaiable = false;
volatile boolean isHome=true, isFirst=true; // whether the string is complete
int drink[13];
int  valve []= {
  23,25,27,29,31,33};

AF_Stepper motor(200, 1);
Servo servo;

void setup()
{
  // initialize serial:
  if(isFirst)
  {
    Serial.begin(9600);
    //int_bfr.reserve(10);
    servo.attach(9);  
    motor.setSpeed(120); // 10 rpm
    motor.release();

    for(int i=0; i<6; i++)
    {
      pinMode(valve[i],OUTPUT);
    }

    //    attachInterrupt(5, setHome, FALLING);
    pinMode(13, OUTPUT);
    digitalWrite(13,LOW);
    pinMode(22, INPUT);

  }

  clearDrink();

  delay(100);
  Serial.println("ready");
  //  servo.write(SERVO_LOW);
  //  delay(100);
}

void loop()
{
  if (drinkAvaiable)
  {
    servo.write(SERVO_LOW);
    slowDown(BACKWARD, false);
    delay(500);
#ifdef DEBUG
    printDrink();
#endif
    for(int i=0; i<7; i++)
    {      
      if(drink[i]==0) continue;
      isHome=false;
      Serial.println(i+1);
      driveTo(i+1);
      Serial.println("there");
      dispense(SERVO, drink[i]);
      Serial.println("filled");
    }
    driveTo(HOME);
    Serial.println(HOME);
    for(int i=7; i<13; i++)
    {
      if(!drink[i]) continue;
      dispense(i, drink[i]);
    }
    Serial.println("done");
    Serial.flush();
    drinkAvaiable = isFirst= false;
    asm volatile (" jmp 0x0");
  }
}

void serialEvent()
{
  String int_bfr = ""; 
  clearDrink();
  uint8_t pos;
  Serial.println("accepted");
  delay(10);
  while (Serial.available()>0)
  {
    char inChar = (char) Serial.read();

    if(isDigit(inChar))
    {
      int_bfr += (char) inChar;
    }
    else if (inChar == '\n')
    {
      drinkAvaiable = true;
      return;
    }
    else
    {
      if(inChar==':')
      {
        pos = int_bfr.toInt();
      }
      else if(inChar==';')
      {
        drink[pos] = int_bfr.toInt();
      }
      int_bfr = "";
    }
    delay(10);

  }
}

void clearDrink()
{
  for(int i=0; i<13; i++)
  {
    drink[i]=0;
  }
}

void printDrink()
{
  Serial.println("----------------------------");
  for(int i=0; i<13; i++)
  {
    Serial.print(i);
    Serial.print(":\t");
    Serial.println(drink[i]);
  }
  Serial.println();
}

//void driveTo(uint8_t pos)
//{
//  static uint8_t actual=HOME;
//  int tmp=pos-actual;
//  if(tmp<0)
//  {
//    tmp *= -1;
//  }
//  delay(1000);
//#ifdef DEBUG
//  Serial.print("Driving ");
//  Serial.print(tmp);
//  Serial.print(" steps to: ");
//  Serial.print(pos);
//  for(int i=0; i<30; i++)
//  {
//    delay(tmp*30);
//    Serial.print('.');
//  }
//#endif
//  actual=pos;
//}

void dispense(uint8_t pos, uint8_t amount)
{
  if(pos==SERVO)
  {
    for(int i=0; i<amount; i+=2)
    {
      for(int val = SERVO_LOW; val>=SERVO_HIGH; val--)     
      {                                
        servo.write(val);  
        delay(10);     
      }
      delay(3000);
      for(int val = SERVO_HIGH; val < SERVO_LOW; val++)
      {
        servo.write(val);               
        delay(10);               
      }
      if(amount>i+2)
        delay(2800);
      delay(200);
    }
  }
  else
  {
    digitalWrite(valve[pos-7],HIGH);
    delay(400*amount);
    digitalWrite(valve[pos-7],LOW);
  }
}

void accelerate(int dir)
{
  for(int i=1;i<=ACCEL_STEPS;i++)
  {
    motor.setSpeed(i*10);
    motor.step(8, dir, SINGLE);
    //      delay(5);
  }
}
void brake(int dir)
{
  for(int i=ACCEL_STEPS; i>0; i--)
  {
    motor.setSpeed(i*10);
    motor.step(8, dir, SINGLE);
    //      delay(5);
  }
  delay(5);
  motor.release();
}
void slowDown(int dir, boolean acc)
{
  int tmp = digitalRead(22);
  if(acc)
  {
    for(int i=ACCEL_STEPS; i>6; i--)
    {
      motor.setSpeed(i*10);
      motor.step(8, dir, SINGLE);
      //      delay(5);
    }
  }
  else
    motor.setSpeed(60);
  while(tmp==HIGH)
  {
    motor.step(2, dir, SINGLE);
    tmp = digitalRead(22);
  }
  isHome = true;
  motor.release();
}
void drive(int mm, int dir)
{
  mm=mm-57;
  float forward_factor=3.6;
  float backward_factor=3.7;
  if(dir==FORWARD)
  {
    motor.step(forward_factor*mm, dir, SINGLE);
  }
  if(dir==BACKWARD)
  {
    motor.step(backward_factor*mm, dir, SINGLE);
  }

}
void driveTo(uint8_t pos)
{
  static int abs_pos=HOME;
  int dist=0;
  switch(pos)
  {
  case HOME: 
    dist=abs_pos-10;
    break;
  case 1: 
    dist=155-abs_pos;
    abs_pos=155;
    break;
  case 2: 
    dist=290-abs_pos;
    abs_pos=290;
    break;
  case 3: 
    dist=425-abs_pos;
    abs_pos=425;
    break;
  case 4: 
    dist=560-abs_pos;
    abs_pos=560;
    break;
  case 5: 
    dist=695-abs_pos;
    abs_pos=695;
    break;
  case 6: 
    dist=830-abs_pos;
    abs_pos=830;
    break;
  case 7: 
    dist=965-abs_pos;
    abs_pos=965;
    break;
  }
  int dir=FORWARD;
  if(pos==HOME)
  {
    if(!isHome)
    {
      dir=BACKWARD;
      accelerate(dir);
      drive(dist, dir);
    }
    //    brake(dir);
    slowDown(dir, true);
  }
  else
  {
    accelerate(dir);
    drive(dist, dir);
    brake(dir);
  }
}
//
//void setHome(void)
//{
//  Serial.println("int");
//  isHome = true;
//  motor.setSpeed(0);
//  motor.release();
//}




