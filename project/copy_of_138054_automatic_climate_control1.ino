#include<avr/io.h>
#include<avr/interrupt.h>

#define SET_BIT(PORT,BIT) PORT|=(1<<BIT)
#define CLR_BIT(PORT,BIT) PORT&= ~(1<<BIT)
#define OFF 0
#define ON 1
#define UP 0
#define DOWN 1
#define MAX_TEMPERATURE_VALUE 166
#define MIN_TEMPERATURE_VALUE 145
#define DOOR_PRESSURE_THRESHOLD 500


volatile uint8_t count = 0;
volatile uint16_t ldr_switch,indi_switch;
uint16_t adcRead();
volatile int analog,analog1;

struct{
  volatile unsigned int FLAG_1:1;
  volatile unsigned int FLAG_2:1;
  volatile unsigned int FLAG_3:1;
  volatile unsigned int FLAG_6:1;
  
}FLAG_BIT;

void drl() 
{
		if(FLAG_BIT.FLAG_1==1)
                {
                  CLR_BIT(PORTB,5);	
                  SET_BIT(PORTD,0);
                    
                  CLR_BIT(PORTB,1);	
                  SET_BIT(PORTB,3);
                }
                
	     else if(FLAG_BIT.FLAG_2==1)
                {
                  CLR_BIT(PORTD,0);	
                  SET_BIT(PORTB,4);
                    
                  CLR_BIT(PORTB,3);	
                  SET_BIT(PORTB,2);
                 }
  
         else if(FLAG_BIT.FLAG_3==1)
                 {
                  CLR_BIT(PORTB,4);	
                  SET_BIT(PORTB,5);
                    
                  CLR_BIT(PORTB,2);	
                  SET_BIT(PORTB,1);
             	 }

}
void led_off()
{
		PORTD &=  ~(1<<PD0);
       	PORTB &=   ~(1<<PB5)& ~(1<<PB4)&~(1<<PB3)& ~(1<<PB2)& ~(1<<PB1);        	
}
/**************************Shivam_main**********************/
void lights()
{
  analog=0x41;
             	 indi_switch=adcRead(analog);
                
              
              
              	if(indi_switch >800  && FLAG_BIT.FLAG_6==1)
                {
                	PORTD |=  (1<<PD0);
              		PORTB |=  (1<<PB4)|(1<<PB5);
                } 	
              else if(indi_switch <300 && FLAG_BIT.FLAG_6==1)
                	PORTB |=  (1<<PB3)|(1<<PB2)|(1<<PB1);
                  
              else if (indi_switch <=800 && indi_switch >=300)
              		{
               
             			 
                
               	 analog1=0x40;
             	 ldr_switch=adcRead(analog1);
              			 
                		if(ldr_switch >100)
              			 drl();
                					
				  		else
              			led_off(); 
              		}
              else
              	led_off();
                    	
              
}
 ISR(TIMER0_OVF_vect)
{		
   count++;
   
   if(count >= 0 && count <=50)
   {
     FLAG_BIT.FLAG_1= ~FLAG_BIT.FLAG_1;
    
   }  	    
   else if(count >= 50 && count <=100 )
   {FLAG_BIT.FLAG_2= ~FLAG_BIT.FLAG_2;	
    FLAG_BIT.FLAG_6=1;} 
   else if(count >= 150 && count <=200 )
   {
     FLAG_BIT.FLAG_3= ~FLAG_BIT.FLAG_3;	
     FLAG_BIT.FLAG_6=0;
   }
   else if(count >= 200 )
   count=0;	
   
   
}

void set_timer()
{
   	TCCR0A |= 0X00;  /// normal opt
	TCNT0=0X00; ///define counter
    TCCR0B |=((0<<CS00)|(1<<CS02));
	TIMSK0|=(1<<TOIE0);
}

/*Reading the ADC value*/
uint16_t adcRead(int x)
{
  //PRR &= ~(1<<PRADC); //POWER REDUCTION OFF
  ADMUX = x; //ADC
  ADMUX |= (1<<REFS0); //VREF =5V
  ADCSRA |= (1<<ADEN); //ADC ENABLE
  ADCSRA |= (1<<ADSC); // SINGLE CONVERSION
  while(ADCSRA & (1<<ADSC));
  return(ADC);
}



/*****************************************************/
struct 
{
  //volatile uint8_t Pressure_sensor:1;
  volatile uint8_t Engine_status:1;
  volatile uint8_t AC_button:1;
  volatile uint8_t Automatic_mode:1;
  volatile uint8_t Window_button:1;
  
}Flag;

void set_pin()
{
  SET_BIT(DDRC,DDC4);
  SET_BIT(DDRD,DDD0);
  
  SET_BIT(DDRB,PB4);
  SET_BIT(DDRB,PB5);
  SET_BIT(DDRB,PB1);
  SET_BIT(DDRB,PB2); 
  SET_BIT(DDRB,PB3); 
  
  SET_BIT(DDRD,DDD5);
  SET_BIT(DDRD,DDD4);
  SET_BIT(DDRD,DDD7);
  SET_BIT(DDRD,DDD1);
 
  CLR_BIT(DDRD,DDD6);
  CLR_BIT(DDRD,DDD3);
  CLR_BIT(DDRD,DDD2);
  
 //CLR_BIT(DDRD,DDD6);
  CLR_BIT(DDRB,DDB5);
  
  CLR_BIT(PORTD, DDD0);
  CLR_BIT(PORTB, PB4);
  CLR_BIT(PORTB, PB5);
  CLR_BIT(PORTB, PB1); 
  CLR_BIT(PORTB, PB2); 
  CLR_BIT(PORTB, PB3); 
  
  
  CLR_BIT(PORTD,PORTD5);
  CLR_BIT(PORTD,PORTD4);
  CLR_BIT(PORTD,PORTD7);
  CLR_BIT(PORTD,PORTD1);
  CLR_BIT(PORTC,PORTC4);
  CLR_BIT(PIND,PIND3);
  CLR_BIT(PIND,PIND6);
  CLR_BIT(PIND,PIND2);
  //CLR_BIT(PIND,PIND6);
  CLR_BIT(PINB,PINB5);

}

void set_interrupt()
{
  SREG |= (1<<7);
  PCICR |= (1<<PCIE0);
  PCMSK0 |= (1<<PCINT0);
  //PCICR |= (1<<PCIE1);
//  PCMSK0 |= (1<<PCINT12);
  PCICR |= (1<<PCIE2);
  PCMSK2 |= (1<<PCINT22);
  EICRA |= (1<<ISC00);
  EICRA |= (1<<ISC10);
  EIMSK |= ((1<<INT0)|(1<<INT1));
}




uint16_t call_adc()
{
  
  ADMUX |= (1<<REFS0);
  ADCSRA |= (1<<ADEN);
 ADCSRA |= (1<<ADSC);
  while((ADCSRA) & (1<<ADSC));
  return(ADC);
}

void initialize()
{
  Flag.Engine_status=OFF;
  //Flag.Pressure_sensor=OFF;
  Flag.Automatic_mode=OFF;
  Flag.AC_button=OFF;
 // Serial.begin(9600);
  
}

void automatic_climate_control()
{ 
      
      uint16_t cabin_temperature;
   // if(Flag.Pressure_sensor==ON)
   // {
      if(Flag.Automatic_mode==ON)
      {
        ADMUX |=(1<<MUX1);
        ADMUX &= ~((1<<MUX0)|(1<<MUX2)|(1<<MUX3));
        cabin_temperature=call_adc();
        // Serial.println(adc);
        if(cabin_temperature>MAX_TEMPERATURE_VALUE)
        {
          SET_BIT(PORTD,PORTD5);
          if(Flag.Engine_status==ON)
             SET_BIT(PORTD,PORTD4);
        }
        else if(cabin_temperature<MIN_TEMPERATURE_VALUE)
        {
          CLR_BIT(PORTD,PORTD5);
          CLR_BIT(PORTD,PORTD4);
        }
        if(Flag.Engine_status==OFF)
          CLR_BIT(PORTD,PORTD4);
      }
      else
      {
        if(Flag.AC_button==OFF)
        {
           CLR_BIT(PORTD,PORTD5);
           CLR_BIT(PORTD,PORTD4); 
        }
        else
        {
           SET_BIT(PORTD,PORTD5);
           if(Flag.Engine_status==ON)
             SET_BIT(PORTD,PORTD4);
        }
        if(Flag.Engine_status==OFF)
          CLR_BIT(PORTD,PORTD4);
      }
        
   // }
  /*  else
    {
      CLR_BIT(PORTD,PORTD5);
      CLR_BIT(PORTD,PORTD4);
    }*/ 
}

void power_window()
{
 if(Flag.Window_button == UP)
       {
      SET_BIT(PORTD,DDD7);
      CLR_BIT(PORTD,DDD1);
      
      
       }
    else if(Flag.Window_button == DOWN)
    {
      CLR_BIT(PORTD,DDD7);
      SET_BIT(PORTD,DDD1);
      
    } 
}

void door_warning()
{
  uint16_t door_sensor;
  ADMUX &= ~((1<<MUX3)|(1<<MUX1));
  ADMUX |= ((1<<MUX2)|(1<<MUX0));
  door_sensor=call_adc();
  if(door_sensor<DOOR_PRESSURE_THRESHOLD)
     {
        Serial.println("Door is not closed:Warning");
        SET_BIT(PORTC,PORTC4);
     }
      else
      {
        Serial.println("Door is closed:Safe to Drive");
        CLR_BIT(PORTC,PORTC4);
   	  }
}

int main()
{
 
  
  initialize();
  set_pin();
  set_interrupt();
  set_timer();
 
 
  
  
  while(1)
  {
    automatic_climate_control();
      
    power_window();
    
    door_warning();
    
    lights();
     
  }
  
}
ISR(INT0_vect)
{
   Flag.AC_button^=1;
}
ISR(INT1_vect)
{
  Flag.Automatic_mode^=1;
}
ISR(PCINT0_vect)
{
  Flag.Engine_status^=1;
}
/*ISR(PCINT1_vect)
{
  Flag.Engine_status^=1;
}*/
ISR(PCINT2_vect)
{
  Flag.Window_button^=1;
}