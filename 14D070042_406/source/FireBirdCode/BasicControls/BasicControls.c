#define F_CPU 14745600
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"


/*********GLOBAL VARIABLES***********/


const int Distance=1500;   //In cms .This is the distance the bot travels out of the arena to throw the object
const int ProximitySensorTransitionValue=120;
const int WhiteLineTransitionValue= 100;
unsigned char data;                                 //to store received data from UDR1
char i='a';
char j='b';                                  //variable for confirming if the object is really red object we wanted
char capture=0x30;
char break_out_of_all_loops='0';


/*************FUNCTIONS STARTS************************/


/*************FUNCTIONS INITIALIZING PORTS************/


void init_devices(void)
{
	motor_pin_config();
	timer5_init();
	lcd_config();
	adc_init();
	adc_pin_config();
	buzzer_pin_config();
	uart0_init();                       //Initailize UART1 for serial communiaction
	sei();                              //Enables the global interrupts
}
void buzzer_pin_config (void)    //configuring the buzzer
{
	DDRC = DDRC | 0x08;
	PORTC = PORTC & 0xF7;
}
void buzzer_on (void)
{
	PORTC = PORTC | 0x08;
}
void buzzer_off (void)
{
	PORTC = PORTC & 0xF7;
}
void motor_pin_config(void)        //configuring all default pins for enabling motors
{
	DDRA=DDRA | 0x0F;         //pins:D0,D1,D2,D3 are used for controlling the motion of bot....taking OR function because lower nimble will be unaffected by it and any change in upper nimble will easily reflect
	PORTA=PORTA & 0xF0;       //Initially the motor is in switched off state
	DDRL=DDRL | 0x18;         //Pins:L3,L4 are used for enabling motor driver IC left and right channel respectively
	PORTL=PORTL | 0x18;	     //Here we have enabled both right and left channel(Port L is used basically for velocity control using PWM:Pulse Width Modulation)
}
void timer5_init(void)    //Initializing the timer 5 which is used for PWM of the signal given to motors for controlling their speed
{
	TCCR5A = 0xA9;
	TCCR5B = 0x0B;
	TCNT5H = 0xFF;
	TCNT5L = 0x00;
	OCR5AH = 0x00;
	OCR5AL = 0x00;    //Keeping the velocity zero as initial of the left motor
	OCR5BH = 0x00;
	OCR5BL = 0x00;    //Keeping the velocity zero as initial of the right motor
}
void lcd_config(void)               //Configuring LCD
{
	DDRC=DDRC | 0xF7;               //Pin C3 is used for controlling buzzer....pins:c4-c7 are used for controlling DB4-BD7 on LCD....c0 for RS(register select),c1 for RW(Read Write), c3 for EN(Enable)
	PORTC=PORTC & 0x08;
	lcd_set_4bit();                //Doing the job of fully starting the LCD in this function itself instead of doing it in main program
	lcd_init();
}
void adc_pin_config (void)         //Configure ADC Ports
{
	DDRF= 0x00;
	DDRK= 0x00;
}
void adc_init(void)                   //Set Register Values for starting ADC
{
	ADCSRA = 0x86;
	ADCSRB = 0x00;
	ADMUX = 0x20;
	ACSR = 0x80;
}
//Function To Initialize UART0
// desired baud rate:9600
// actual baud rate:9600 (error 0.0%)
// char size: 8 bit
// parity: Disabled
void uart0_init(void)
{
	UCSR0B = 0x00; //disable while setting baud rate
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	UBRR0L = 0x5F; //set baud rate lo
	UBRR0H = 0x00; //set baud rate hi
	UCSR0B = 0x98;
}


/***************FUNCTIONS FOR MOTION CONTROL***********/


void forward(void)
{
	char PortARestore=0xF0 & PORTA;     //Now PortARestore will contain upper nimble of PORT A and 0000 as lower nimble
	PORTA=0x06 | PortARestore;          //Now this will contain upper nimble of PORT A (unaltered) and lower nimble containing the required command
}
void backward(void)
{
	_delay_ms(50);
	char PortARestore=0xF0 & PORTA;
	PORTA=0x09 | PortARestore;
}
void softRight(void)
{
	char PortARestore=0xF0 & PORTA;
	PORTA=0x04 | PortARestore;
}
void right(void)
{
	char PortARestore=0xF0 & PORTA;
	PORTA=0x05 | PortARestore;
}
void softLeft(void)
{
	char PortARestore=0xF0 & PORTA;
	PORTA=0x02 | PortARestore;
}
void left(void)
{
	char PortARestore=0xF0 & PORTA;
	PORTA=0x0A | PortARestore;
}
void stop(void)
{
	char PortARestore= 0xF0 & PORTA;
	PORTA =0x00 | PortARestore;
}
void velocity_control(unsigned char left_motor,unsigned char right_motor)   //left_motor and right_motor values varies from 0 to 255(i.e. from minimum to maximum)
{
	OCR5AL=(unsigned char)left_motor;
	OCR5BL=(unsigned char)right_motor;
}
unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;
	ADMUX= 0x20 | Ch;
	ADCSRA = ADCSRA | 0x40; //Set start conversion bit
	while((ADCSRA&0x10)==0); //Wait for ADC conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}


/************** LCD PRINTING USING DIRECT ANALOG SIGNAL UPTO THREE DIGITS **********************/


void print_sensor(char row, char column, unsigned char channel)
{
	unsigned int ADC_Value = ADC_Conversion(channel);
	lcd_print(row, column, ADC_Value, 3);
}
unsigned int Sharp_GP2D12_estimation(unsigned char adc_reading)            //Function for estimating Actual Distance for corresponding Analog Signals
{
	float distance;
	unsigned int distanceInt;
	distance = (int)(10.00*(2799.6*(1.00/(pow(adc_reading,1.1546)))));
	distanceInt = (int)distance;
	if(distanceInt>800)
	{
		distanceInt=800;
	}
	return distanceInt;
}
void BasicDrivingProgram(unsigned int SharpIR_sensor1,unsigned int SharpIR_sensor2,unsigned int SharpIR_sensor3,unsigned int proximity_sensor_left_3,unsigned int proximity_sensor_frontleft_4,unsigned int proximity_sensor_frontright_5,unsigned int proximity_sensor_right_6,char* distance_confirmation,unsigned int white_line_sensor_left_1,unsigned int white_line_sensor_middle_2,unsigned int white_line_sensor_right_3 )
{
	if(SharpIR_sensor2<=200)
	{
		*distance_confirmation='1';
	}
	if(proximity_sensor_left_3>ProximitySensorTransitionValue && proximity_sensor_frontleft_4>ProximitySensorTransitionValue && proximity_sensor_frontright_5>ProximitySensorTransitionValue && proximity_sensor_right_6>ProximitySensorTransitionValue)
	{
		forward();
	}
	else if(( proximity_sensor_left_3<=ProximitySensorTransitionValue || proximity_sensor_frontleft_4<=ProximitySensorTransitionValue) && proximity_sensor_frontright_5>ProximitySensorTransitionValue && proximity_sensor_right_6>ProximitySensorTransitionValue )
	{
		left();
	}
	else if(proximity_sensor_left_3>ProximitySensorTransitionValue && proximity_sensor_frontleft_4>ProximitySensorTransitionValue && (proximity_sensor_frontright_5<=ProximitySensorTransitionValue || proximity_sensor_right_6<=ProximitySensorTransitionValue))
	{
		right();
	}
	else
	{
		leftd(43);  //For re-orientation
	}
	if(white_line_sensor_left_1>WhiteLineTransitionValue || white_line_sensor_middle_2>WhiteLineTransitionValue || white_line_sensor_right_3>WhiteLineTransitionValue)
	{
		stop();
		_delay_ms(500);
		backward();
		_delay_ms(500);
		stop();
		_delay_ms(200);
		leftd(180);
		break_out_of_all_loops='1';
	}
}
White_line_sensor_pushing(unsigned int SharpIR_sensor1,unsigned int SharpIR_sensor2,unsigned int SharpIR_sensor3,unsigned int proximity_sensor_left_3,unsigned int proximity_sensor_frontleft_4,unsigned int proximity_sensor_frontright_5,unsigned int proximity_sensor_right_6,unsigned int white_line_sensor_left_1,unsigned int white_line_sensor_middle_2,unsigned int white_line_sensor_right_3,char* white_line_check,char* WLS1_flag,char* WLS2_flag,char* WLS3_flag)
{
	//white_line_check++; statement will be executed and when its value will reach 1 this will be the indication for starting the object pushing mechanism
	char t='0';                 // Dummy Variable required for calling BDP function
	if(white_line_sensor_left_1>WhiteLineTransitionValue)
	{
		*WLS1_flag='1';
	}
	if(white_line_sensor_middle_2>WhiteLineTransitionValue)
	{
		*WLS2_flag='1';
	}
	if(white_line_sensor_right_3>WhiteLineTransitionValue)
	{
		*WLS3_flag='1';
	}
	if(*WLS1_flag=='1' && *WLS2_flag=='1' && *WLS3_flag=='1')
	{
		*white_line_check='1';
	}
	BasicDrivingProgram(SharpIR_sensor1,300,SharpIR_sensor3,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,&t,50,50,50);           //At this point BDP does not need to take care of the black line
	/* HERE WE HAVE GIVEN SOME DEFAULT VALUES TO THE BasicDrivingProgram SO AS ENSURE THAT OUR OBJECTIVE IS FULFILLED */
}


/**************POSITION CONTROLLED MOVEMENT*************/


void forwardd(int Dist)
{
	forward();
	for(int loop=0;loop<Dist;loop++)
	_delay_ms(6);
	stop();
}
void backwardd(int Dist)
{
	backward();
	for(int loop=0;loop<Dist;loop++)
	_delay_ms(6);
	stop();
}
/* OUR leftd & rightd FUNCTION PERFORMS ROTATION OF ONLY CERTAIN ANGLES, THAT TOO NOT ACCURATELY */
void leftd(int degree)
{
	left();
	if(degree==7)
	_delay_ms(200);
	else if(degree==180)
	_delay_ms(1500);
	else if(degree==5)
	_delay_ms(100);
	else if(degree==43)
	_delay_ms(1000);
	stop();
}
void rightd(int degree)
{
	right();
	if(degree==7)
	_delay_ms(200);
	else if(degree==180)
	_delay_ms(1500);
	else if(degree==5)
	_delay_ms(100);
	else if(degree==43)
	_delay_ms(1000);
	stop();
}


/**************PROGRAM HANDLING XBEE TO AND FORTH SIGNALS***************/


SIGNAL(SIG_USART0_RECV) 		// ISR for receive complete interrupt
{
	data = UDR0; 				//making copy of data from UDR0 in 'data' variable

	if(data==0x38)
	{
		UDR0 = capture;
	}
	if(data==0x31 || data==0x30)
	{
		i=data;
		UDR0 = data; 				//echo data back to PC
	}
	else if(data==0x32 || data==0x33)
	{
		j=data;
		UDR0 = data; 				//echo data back to PC
	}
}

/************************* MAIN PROGRAM STARTS ***************************/

 
int main(void)
{
	init_devices();
	velocity_control(248,255);
	while(1)
	{
		capture=0x31;
		//command for capturing the image to be sent through Xbee
		//Image processing
		//Will be having an integer i as an output from the processed program
		if(i=='1')
		{
			stop();
			right();
			_delay_ms(150);
			stop();
			lcd_cursor(1,1);
			lcd_string("IMAGE FAR");
			capture=0x30;
			//Basic Driving Program
			while(1)                                   //will break when the object is finally pushed out and bot has returned
			{
				char distance_confirmation='0';        //For denoting if the red block is less than 20 cms from the bot
				int termination=0;                     //for finding the appropriate point for terminating this loop
				unsigned int SharpIR_sensor1,SharpIR_sensor2,SharpIR_sensor3,proximity_sensor_left_3,proximity_sensor_frontleft_4,proximity_sensor_frontright_5,proximity_sensor_right_6,white_line_sensor_left_1,white_line_sensor_middle_2,white_line_sensor_right_3;
				SharpIR_sensor1=Sharp_GP2D12_estimation(ADC_Conversion(10));
				SharpIR_sensor2=Sharp_GP2D12_estimation(ADC_Conversion(11));
				SharpIR_sensor3=Sharp_GP2D12_estimation(ADC_Conversion(12));
				proximity_sensor_left_3=ADC_Conversion(4);
				proximity_sensor_frontleft_4=ADC_Conversion(5);
				proximity_sensor_frontright_5=ADC_Conversion(7);
				proximity_sensor_right_6=ADC_Conversion(8);
				white_line_sensor_left_1=ADC_Conversion(3);
				white_line_sensor_middle_2=ADC_Conversion(2);
				white_line_sensor_right_3=ADC_Conversion(1);
				//Take some sensory inputs
				//feed the inputs to BDC
				BasicDrivingProgram(SharpIR_sensor1,SharpIR_sensor2,SharpIR_sensor3,proximity_sensor_left_3,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,proximity_sensor_right_6,&distance_confirmation,white_line_sensor_left_1,white_line_sensor_middle_2,white_line_sensor_right_3);
				if(distance_confirmation=='1')
				{
					stop();
					_delay_ms(500);
					capture=0x32;
					lcd_cursor(1,1);
					lcd_string("IMAGE NEAR");
					//command for capturing the image
					//image processing and then continuing
					if(j=='2')
					{
						capture=0x30;
						char white_line_check='0';
						char WLS1_flag='0',WLS2_flag='0',WLS3_flag='0';
						lcd_cursor(1,1);
						lcd_string("ELEMINATING");
						while(1)
						{
							//White line sensor inputs and other sensor inputs
							//WhiteLineSensor and pushing mechanism
							SharpIR_sensor1=Sharp_GP2D12_estimation(ADC_Conversion(10));
							SharpIR_sensor2=Sharp_GP2D12_estimation(ADC_Conversion(11));
							SharpIR_sensor3=Sharp_GP2D12_estimation(ADC_Conversion(12));
							proximity_sensor_left_3=ADC_Conversion(4);
							proximity_sensor_frontleft_4=ADC_Conversion(5);
							proximity_sensor_frontright_5=ADC_Conversion(7);
							proximity_sensor_right_6=ADC_Conversion(8);
							white_line_sensor_left_1=ADC_Conversion(3);
							white_line_sensor_middle_2=ADC_Conversion(2);
							white_line_sensor_right_3=ADC_Conversion(1);
							White_line_sensor_pushing(SharpIR_sensor1,SharpIR_sensor2,SharpIR_sensor3,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,ProximitySensorTransitionValue+10,white_line_sensor_left_1,white_line_sensor_middle_2,white_line_sensor_right_3,&white_line_check,&WLS1_flag,&WLS2_flag,&WLS3_flag);
							if(white_line_check=='1')
							{
								//new mechanism for going forward a particular distance and again coming back
								buzzer_on();
								_delay_ms(100);
								buzzer_off();
								forwardd(Distance);
								rightd(7);
								backwardd(Distance+1000);
								leftd(180);
								termination=1;
								break;
							}
							if(break_out_of_all_loops=='1')
							break;
						}
						j='b';
					}
					else if(j=='3' && distance_confirmation=='2')
					{
						capture=0x30;
						lcd_cursor(1,1);
						lcd_string("REORIENTING J");
						right();
						_delay_ms(1000);
						stop();
						forward();
						_delay_ms(1800);
						stop();
						
						j='b';
						termination=1;
						break;
					}
				}
				else
				{
					continue;
				}
				if(termination==1)
				break;
				if(break_out_of_all_loops=='1')
				break;
			}
			i='a';
		}
		else if(i=='0')
		{
			capture=0x30;
			leftd(7);        //Rotate and then continue
			i='a';
			lcd_cursor(1,1);
			lcd_string("Turning");
			continue;
		}
		break_out_of_all_loops='0';
	}
}