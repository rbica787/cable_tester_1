/* cable tester 1.0
 *  
 * 
 * DESCRIPTION OF THE CIRCUIT:
 * this circuit uses two 74HC595 output shift registers
 * and two 74HC165 input shift registers. It would have
 * all of the components on two stackable shields. The first
 * with two DB25 female solder connectors to pair with 2
 * complementary external adapter harnesses.
 * The second shield would only have a 4x20 i2c lcd screen 
 * and 2 other minor components;
 * namely a 2-way switch, and an active buzzer.

*/

#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <ShiftIn.h>
ShiftIn<2> shift;     //this designates my 165 register quantity

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);


//these are defining the 74HC595 (output register) pins
#define DATA_595 4  //SER in the diagram
#define LATCH_595 5  //RCLK in the diagram
#define CLOCK_595 6 //SRCLK in the diagram



//these are defining the 74HC165 (input register) pins
#define PL_165 7
#define CLK_165 8
#define CE_165 9
#define Q7_165 10



#define number_of_bits 7    //number minus one

#define BUZZER A1

int number_of_circuits_to_test = 16;

int xx = 0;

const int test_type_switch = A0;

boolean correct_bit;

int min595_delay = 10;  // the lowest these can go are the minimum
                       // time specified in the datasheet.
int min165_delay = 5;
          
byte short_byte = 0x1;   // further in the code you will notice that
                         // this hexadecimal value establishes the
                        // first byte sent to the ouputs prior to
                        // the one_at-a-time "for" loop

String double_digit_values[16] = {"2","3","4","5","6","7","8","9","A","B","C","D","E","F","G","H"};
String printed_circuit_number;  
  

///////////////////////////////////////////////////////////
byte good_logicValue[16];  //my short tester comparison arrays;

byte good_math_value[16] = {1, 2, 4, 8, 16, 32, 64, 128, 1, 2, 4, 8, 16, 32, 64, 128};

boolean circuit[16];

boolean all_my_circuits;

int bitWise;

int bitMath;

///////////////////////////////////////////////////////////

byte clear_byte = 0x00;
byte open_byte = 0xFF;

byte incoming_1;   // these are for reading my input data,
byte incoming_2;   // 1 per register


void setup() {

  lcd.init();
  lcd.backlight();
  
  good_logicValue[0] = 0x01;  //00000001

  good_logicValue[1] = 0x02;  //00000010
 
  good_logicValue[2] = 0x04;  //00000100

  good_logicValue[3] = 0x08;  //00001000

  good_logicValue[4] = 0x10;  //00010000

  good_logicValue[5] = 0x20;  //00100000

  good_logicValue[6] = 0x40;  //01000000

  good_logicValue[7] = 0x80;  //10000000

  good_logicValue[8] = 0x01;  //00000001

  good_logicValue[9] = 0x02; //00000010

  good_logicValue[10] = 0x04; //00000100

  good_logicValue[11] = 0x08; //00001000

  good_logicValue[12] = 0x10; //00010000

  good_logicValue[13] = 0x20; //00100000

  good_logicValue[14] = 0x40; //01000000

  good_logicValue[15] = 0x80; //10000000



  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  Serial.begin(9600);

  // this library is pretty much obsolete for what I need,
  //but it is pretty convenient for easily declaring my
  //shift register input pins
  shift.begin(PL_165, CE_165, Q7_165, CLK_165);

  //declaring my 595 output pins
  for (int shiftPins = 4; shiftPins < 8; shiftPins++) {
    pinMode(shiftPins, OUTPUT);
  }

  pinMode(test_type_switch, INPUT);

  digitalWrite(CLOCK_595, LOW);
  digitalWrite(LATCH_595, HIGH);
  digitalWrite(DATA_595, LOW);
}



void loop() {
  //this is an integer that will constantly update each time the loop is run
  int switch_state = digitalRead(test_type_switch);

  if (switch_state)
    open_test();

  else
    short_tester();



}







//its a continuity test
void open_test() {

     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("CONTINUITY TEST");
     
  
  digitalWrite(LATCH_595, LOW);

  shiftOut(DATA_595, CLOCK_595, MSBFIRST, open_byte);
  shiftOut(DATA_595, CLOCK_595, MSBFIRST, open_byte);

  digitalWrite(LATCH_595, HIGH);

  setup_165s();
  incoming_2 = shiftIn(Q7_165, CLK_165, MSBFIRST);
  incoming_1 = shiftIn(Q7_165, CLK_165, MSBFIRST);
  digitalWrite(CE_165, HIGH); // Disable the clock

  print_byte(incoming_1);
  print_byte(incoming_2);
  Serial.print("\n");


  if ((incoming_1 != open_byte) || (incoming_2 != open_byte))
    digitalWrite(BUZZER, HIGH);
  else
    digitalWrite(BUZZER, LOW);

}


// A function that prints all the 1's and 0's of a byte, so 8 bits +or- 2
void print_byte(byte val)
{
  byte i;
  for (byte i = 0; i <= number_of_bits; i++)
  {
    Serial.print(val >> i & 1, BIN); // Magic bit shift, if you care look up the <<, >>, and & operators
    Serial.print(' ');
  }
}



// This code is intended to trigger the shift register to grab values from it's inputs
void setup_165s()
{
  // Trigger loading the state into the shift register
  digitalWrite(PL_165, LOW);
  delayMicroseconds(min165_delay); // Requires a delay here according to the datasheet timing diagram
  digitalWrite(PL_165, HIGH);
  delayMicroseconds(min165_delay);

  // Required initial states of these two pins according to the datasheet timing diagram
  digitalWrite(CLK_165, HIGH);
  digitalWrite(CE_165, LOW); // Enable the clock

}












//this is one of the main body functions of the project
void short_tester() {



  digitalWrite(BUZZER, LOW);  //this makes sure the buzzer is off
                              //when switching from the continuity test

  //this allows the data to be dropped into the output shift registers
  digitalWrite(LATCH_595, LOW);


  shiftOut(DATA_595, CLOCK_595, MSBFIRST, clear_byte);
  //function that automatically uploads one byte into the output registers
  shiftOut(DATA_595, CLOCK_595, MSBFIRST, short_byte);


  //now pulling the latch to HIGH sends the data out
  digitalWrite(LATCH_595, HIGH);


  setup_165s();
    incoming_2 = shiftIn(Q7_165, CLK_165, MSBFIRST);
    incoming_1 = shiftIn(Q7_165, CLK_165, MSBFIRST);
  digitalWrite(CE_165, HIGH); // Disable the clock
  
//Serial.print("incoming_1 = ");
  print_byte(incoming_1);
//Serial.print("incoming_2 = ");
  print_byte(incoming_2);
  Serial.print("\n");

  bitWise = incoming_2 | incoming_1;
  bitMath = incoming_2 + incoming_1;
  boolean correct_firstBit = bitRead(incoming_1, 0);

 /*
//this is what the compiler is reading for the comparison operation
    Serial.print("(");
    print_byte(bitWise);
    Serial.print(" == ");
    print_byte(good_logicValue[0]);
    Serial.print(")  &&  (");
    Serial.print(bitMath, DEC); 
    Serial.print(" == ");      //purposes
    Serial.print(good_math_value[0]);
    Serial.print(")  &&  ");
    Serial.print(correct_firstBit);
    Serial.print("\n");
*/

 circuit[0] = (bitWise == good_logicValue[0]) && (bitMath == good_math_value[0]) && correct_firstBit;
 
      //this prints the value of incoming circuit[0] for diagnostic purposes
  //Serial.print("circuit[0] = ");
  //Serial.println(circuit[0]);
 
 
    if(bitWise == clear_byte){
        lcd.setCursor(0, 0);
        lcd.print("OPEN    "); 
        lcd.setCursor(0, 1);
        lcd.print(1);
          one_at_a_time();
    }
        else if ((circuit[0] != 1) && (bitWise != clear_byte)){
           lcd.setCursor(0, 2);
           lcd.print("SHORT"); 
           lcd.setCursor(0, 3);
           lcd.print(1);
           one_at_a_time();
        }
            else{
               one_at_a_time();
          }

       all_my_circuits = circuit[0];
for(int q = 1; q < 16; q++){

  /*
        Serial.print("all_my_circuits[");
        Serial.print(q - 1);
        Serial.print("] = ");
          Serial.print(all_my_circuits);
        Serial.print("    circuit[");
        Serial.print(q);
        Serial.print("] = ");
          Serial.print(circuit[q]);
*/
        
  all_my_circuits = all_my_circuits & circuit[q];

 /*
  Serial.print("    loop # =  ");
    Serial.print(q);
      Serial.print("   all_my_circuits = ");
  Serial.println(all_my_circuits);
*/

  
}

  if(xx < 1)
    lcd.clear();
  xx++;

  
  if(all_my_circuits){
    xx = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALL GOOD"); 
}


}














//this pushes a byte down the LED chain
//one bit(LED) at a time
void one_at_a_time(){

int x = 0; // a counting variable for reading the correct bit number
          // from the correct byte
     
  
  for (int j = 1; j < number_of_circuits_to_test; j++) {

    digitalWrite(DATA_595, LOW);
    digitalWrite(CLOCK_595, HIGH);
    delay(min595_delay);

    digitalWrite(CLOCK_595, LOW); //the clock pulse without a data pulse creates a "0"
    delay(min595_delay);

    digitalWrite(LATCH_595, LOW);
    delay(min595_delay);

    //now pulling the latch HIGH again send that "0" signal out to the LED's
    digitalWrite(LATCH_595, HIGH);

  setup_165s();
    incoming_2 = shiftIn(Q7_165, CLK_165, MSBFIRST);
    incoming_1 = shiftIn(Q7_165, CLK_165, MSBFIRST);
  digitalWrite(CE_165, HIGH); // Disable the clock

//Serial.print("incoming_1 = ");
  print_byte(incoming_1);
//Serial.print("incoming_2 = ");
  print_byte(incoming_2);
  Serial.print("\n");


     bitWise = incoming_2 | incoming_1;
     bitMath = incoming_2 + incoming_1;


if(x < 7){
                    // Serial.print("correct_bit #");
                   //   Serial.print(j);
                  //  Serial.print(" = ");
  correct_bit = bitRead(incoming_1, j);
}
else{
  int k = j - 8;
                  //   Serial.print("correct_bit #");
                 //   Serial.print(k);
                //  Serial.print(" = ");
  correct_bit = bitRead(incoming_2, k);
}
              //Serial.println(correct_bit);

/*
//this is what the compiler is reading for the comparison operation
    Serial.print("(");
    print_byte(bitWise);
    Serial.print(" == ");
    print_byte(good_logicValue[j]);
    Serial.print(")  &&  (");
    Serial.print(bitMath, DEC); 
    Serial.print(" == ");      //purposes
    Serial.print(good_math_value[j]);
    Serial.print(")  &&  ");
    Serial.print(correct_bit);
    Serial.print("\n");
*/

 circuit[j] = (bitWise == good_logicValue[j]) && (bitMath == good_math_value[j]) && correct_bit;
 
 /*
 //this prints the values of all other incoming circuits for diagnostic purposes
 Serial.print("circuit[");
 Serial.print(j);
 Serial.print("] = ");
 Serial.println(circuit[j]);
*/

   if(j < 9){
    printed_circuit_number = j+1;
   }
   else
    printed_circuit_number = double_digit_values[j];
    
    if(bitWise == clear_byte){
        lcd.setCursor(0, 0);
        lcd.print("OPEN    "); 
        lcd.setCursor(j, 1);
        lcd.print(printed_circuit_number);

    }
    else if((circuit[j] != 1) && (bitWise != clear_byte)){
        lcd.setCursor(0, 2);
        lcd.print("SHORT"); 
        lcd.setCursor(j, 3);
        lcd.print(printed_circuit_number);

        }


    x++;

    
  }
}
