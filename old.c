
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"
#include "main.h"
#include "can.h"
#include "dma.h"
#include "i2c.h"
//#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>



void SystemClock_Config(void);
void can_init();
void can_ctrl_loop();

int BlackLine=0; // to count how many black line we seen
int counter=0;
static uint32_t signal_distance=0;// the middle pt of each box btw the right wall
static uint32_t conducting_distance=0;
static uint32_t last_ticks_1 = 0;


//LTSL=lime tracking sensor at left
//LTSM=lime tracking sensor at middle
//LTRL=lime tracking sensor at right
//if(gpio_read(LTSL)==1 WHEN 1 IS DETECTED BLACK

//assume
int signal=2;// for example signal unit at box2
const int conducting=1;// for example conduct unit at 1
const int car_length=211; //just for an example

int f1 = 1;
int f2 = 1;
int f3 = 1;
int SL = 0;
int SM = 0;
int SR = 0;
uint64_t F=0;

void PID_L(float target, float measurement, Motor whichmotor) {
	float input;
    static float prior_error = 0;
    static float ki_total = 0;
    float integral=0;
    float kp = 3;
    float ki = 0;
    float kd = 0.0002;

    float error = target - measurement;
    float proportional = kp * error;

    ki_total += error; // sum of error
    integral = integral + ki * ki_total * 0.001; // previous integral + gain * sampling_time * total_error

    float derivative = kd * ((error - prior_error) / 0.001); // 1ms

    input = proportional + integral + derivative;
    prior_error = error;

    set_motor_current(whichmotor, input);
}

void PID_R(float target, float measurement, Motor whichmotor) {
	float input;
    static float prior_error = 0;
    static float ki_total = 0;
    float integral=0;
    float kp = 3;
    float ki = 0;
    float kd = 0.0002;

    float error = target - measurement;
    float proportional = kp * error;

    ki_total += error; // sum of error
    integral = integral + ki * ki_total * 0.001; // previous integral + gain * sampling_time * total_error

    float derivative = kd * ((error - prior_error) / 0.001); // 1ms

    input = proportional + integral + derivative;
    prior_error = error;

    set_motor_current(whichmotor, input);
}





void backward(){
	PID_L(-50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR0);
	PID_R(-50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR1);
}

void forward(){
	PID_L(50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR0);
	PID_R(50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR1);

}

void stop(){
	PID_L(0, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR0);
	PID_R(0, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR1);
}

void left(){
	PID_L(-50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR0);
	PID_R(50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR1);
}

void right(){
	PID_L(50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR0);
	PID_R(-50, get_motor_feedback(CAN1_MOTOR0).vel_rpm, CAN1_MOTOR1);
}

void catching(){
 led_on(LED2);
}

void releasing(){
	 led_on(LED3);
}



//if receive signal start move
int main() {

	  /* USER CODE BEGIN 1 */

	  /* USER CODE END 1 */

	  /* MCU Configuration--------------------------------------------------------*/

	  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	  HAL_Init();

	  /* USER CODE BEGIN Init */

	  /* USER CODE END Init */

	  /* Configure the system clock */
	  SystemClock_Config();

	  /* USER CODE BEGIN SysInit */

	  /* USER CODE END SysInit */

	  /* Initialize all configured peripherals */
	  MX_GPIO_Init();
	  MX_CAN1_Init();
	  MX_CAN2_Init();
	  MX_SPI1_Init();
	  MX_USART1_UART_Init();
	  MX_I2C2_Init();
	  MX_DMA_Init();
	  MX_USART2_UART_Init();
	  MX_TIM5_Init();
	  /* USER CODE BEGIN 2 */
	  static uint32_t last_tick = 0;

		// we turn off all the led first
		led_off(LED1);
		led_off(LED2);
		led_off(LED3);
		led_off(LED4);
		tft_init(PIN_ON_TOP, BLACK, WHITE, YELLOW, DARK_GREEN);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);

	  /* USER CODE END 2 */

	  /* Infinite loop */
	  /* USER CODE BEGIN WHILE */

		tft_force_clear();


		// main.c - main()

		// all your other init functions
		can_init(); // initialize CAN



	while(1){
		static char rx_buff[99];
		int distance = 0;
		HAL_UART_Receive(&huart1, (uint8_t) &rx_buff, sizeof(rx_buff), 100);
		for (int i = 9; i < 40; i++) {
			if (rx_buff[i] == 'd' && rx_buff[i + 1] == ':') {
				for (int j = 3; j < 7; j++) {
					if (rx_buff[i + j] == ' ') {
						continue;
					}
					distance = distance*10 + (rx_buff[i + j] - '0');
				}
			}
		}
		F = distance;
		SL = gpio_read(Left_Sensor);
		SM = gpio_read(Middle_Sensor);
		SR = gpio_read(Right_Sensor);

		/*float position = 0.0;
		if (SL == 1) {
			position = -1.0; // Left
		} else if (SL == 1) {
			position = 1.0; // Right
		} else if (SM == 1) {
			position = 0.0; // Center
		}
*/
		// Update PID controller
		//update_pid(position);


	//0123 from left to right
	if(signal==0){signal_distance=683+car_length/2;}
	if(signal==1){signal_distance=496+car_length/2;}
	if(signal==2){signal_distance=305+car_length/2;}
	if(signal==3){signal_distance=117+car_length/2;}

	if(conducting==0){conducting_distance=683+car_length/2;}
	if(conducting==1){conducting_distance=496+car_length/2;}
	if(conducting==2){conducting_distance=305+car_length/2;}
	if(conducting==3){conducting_distance=117+car_length/2;}


	if (BlackLine == 0 || BlackLine == 1) {
		counter=0;

		if (SL == 0 && SM == 1 && SR == 0) {
			forward();
		}
		//if (SL == 0 && SM == 0 && SR == 0) //all sense white// ignore
		//else if (SL==1&& SM==0&& SR==1) ignore this case
		else if ((SL == 1 && SM == 0 && SR == 0)
				|| (SL == 1 && SM == 1 && SR == 0)) {
			left();
		} else if ((SL == 0 && SM == 0 && SR == 1)
				|| (SL == 0 && SM == 1 && SR == 1)) {
			right();
		} else if (SL == 1 && SM == 1 && SR == 1) // see the black line
				{
			stop();
			BlackLine += 1;
		} // forward a little bit more before stop
		//f1 = SL;
		//f2 = SM;
		//f3 = SR;
	}
	//case that put in box 2 first(0,1,2,3)
	if(BlackLine==2){// 283 for box 2 only
		if(counter==0){  //turn 90 degree
			if(F>=400-car_length){right();}//400-length of car/2
			else{stop();counter=counter+1;}
		}
		if(counter==1){  //forward
			if(F>signal_distance){forward();}//283 the font sensor btw right wall when go to box2
			else{stop();counter=counter+1;}
		}
		if(counter==2){ //turn 90 degree
			if(F<=595){left();}//645-50 the distance btw front wall and blackline that diffuse
			else{stop();counter=counter+1;}
		}
		if(counter==3){ //turn 90 degree
			if(F>=595){left();}// 645-50 the distance btw front wall and blackline that diffuse
			else{stop();counter=counter+1;}
		}
		if(counter==4){  //forward&catch
			if(F>=275){forward();} //the distance btw front wall and the front of 4box
			else{
				stop();catching();counter=counter+1;
				//if(R>distance_signal){right();}
				//if(R<distance_signal){left();}
				//if(R==distance_signal){forward();}
				}
		}
		if(counter==5){ //back to the dark line that starts diffuse
			if(F<=595){backward();}
			else{stop();counter=counter+1;}
		}
		if(counter==6){	//turn right
			if(F>=signal_distance){right();}
			else{stop();counter=counter+1;}
		}
		if(counter==7){ //return to middle line
			if(F<=400-car_length){backward();}// (800-length of car)/2
			else{stop();counter=counter+1;}
		}
		if(counter==8){
			if(F<=595){left();}
			else{stop();counter=counter+1;}
		}
		if(counter==9){
			if(F>=595){left();}
			else{stop();BlackLine+=1;}
		}
	}

	if(BlackLine==3){ // follow the middle line to go backward
		// if(L>1000) move more 400,then turn right,to the yellow region
		counter=0;
		if (SL == 0 && SM == 1 && SR == 0) {
			backward();
		}
		//if (SL == 0 && SM == 0 && SR == 0) //all sense white

		//else if (SL==1&& SM==0&& SR==1) ignore this case
		else if ((SL == 1 && SM == 0 && SR == 0)
				|| (SL == 1 && SM == 1 && SR == 0)) {
			left();
		} else if ((SL == 0 && SM == 0 && SR == 1)
				|| (SL == 0 && SM == 1 && SR == 1)) {
			right();
		} else if (SL == 1 && SM == 1 && SR == 1) // see the black line
				{
			stop();
			BlackLine += 1;
		} // forward a little bit more before stop
		f1 = SL;
		f2 = SM;
		f3 = SR;

	}
	if(BlackLine==4){
		if((HAL_GetTick() - last_ticks_1)<=100){forward();}
		else{
			if(SM==1 && SR==1){stop(); BlackLine+=1;}
			else{ left();}
		}
	}
	if (BlackLine ==5) { //idk if enter the yellow area will pass through one black line or two, bc there is a thin line before entering the yellow region

		if (SL == 0 && SM == 1 && SR == 0) {
			forward();
		}
		//if (SL == 0 && SM == 0 && SR == 0) //all sense white



		//else if (SL==1&& SM==0&& SR==1) ignore this case
		else if ((SL == 1 && SM == 0 && SR == 0)
				|| (SL == 1 && SM == 1 && SR == 0)) {
			left();
		} else if ((SL == 0 && SM == 0 && SR == 1)
				|| (SL == 0 && SM == 1 && SR == 1)) {
			right();
		} else if (SL == 1 && SM == 1 && SR == 1) // see the black line
				{
			stop();
			BlackLine += 1;
		} // forward a little bit more before stop
		f1 = SL;
		f2 = SM;
		f3 = SR;
	}
	if (BlackLine ==6){
		releasing();
		BlackLine += 1;
	}
	if (BlackLine ==7){
		if(F<(3300-50-400-car_length)){backward();}
		else{stop();BlackLine += 1;}
	}
	if(BlackLine==8){
		if(SL==1 && SR==1){stop();BlackLine += 1;}
		else{left();}
	}
//repeat once more

	 //printing feedback on TFT.
	tft_prints(0, 0, "VEL_L: %5d", get_motor_feedback(CAN1_MOTOR0).vel_rpm);
	tft_prints(0, 1, "VEL_R: %5d", get_motor_feedback(CAN1_MOTOR1).vel_rpm);



	tft_update(100); // update the TFT value every 100 ms
	can_ctrl_loop();
}

}

//farest4000 only
//void loop(void) {
	//B= gpio_read(TOF_Back);
	/*if(B<800){forward();}
	if((R>610){right();}// exceed this limit the car will crash the wall
	if(B<800){forward();}
	if(B>800){
		if(R>distance_signal){right()}
		if(R<distance_signal){left();}
		if(R==distance_signal){forward);}
	}
*/



	/*if (BlackLine == 2) {
		if (signal== 0) {
			if (counter < 2) {
				left();
				if (SM == 1) {
					counter += 1;
				}
			}
			if (counter >= 2) {
				forward();
			}

		}
		if (signal == 1) {
			if (counter < 1) {
				left();
				if (SM == 1) {
					counter += 1;
				}
			}
			if (counter >= 1) {
				forward();
			}
		}
	}
	if (signal == 2) {
		if (counter < 1) {
			right();
			if (SM == 1) {
				counter += 1;
			}
		}
		if (counter >= 1) {
			forward();
		}

	}
	if (signal == 3) {
		if (counter < 2) {
			right();
			if (SM == 1) {
				counter += 1;
			}
		}
		if (counter >= 2) {
			forward();
		}

	}

	*/






