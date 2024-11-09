#include "main.h"
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"

/*
 if receive signal, start moves
 black line+1=0
 if black line =2
//max detect distance=4000<4800
 */
int BlackLine=0; // to count how many black line we seen
int counter;
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

void backward();

void forward();

void stop();

void left();

void right();

void catching();

void releasing();

void pid() {
	SL = f1;
	SM = f2;
	SR = f3;
}


//if receive signal start move
void loop(void) {
	F= gpio_read(TOF_Front);
	SL = gpio_read(LTSL);
	SM = gpio_read(LTSM);
	SR = gpio_read(LTSR);

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
		if (SL == 0 && SM == 0 && SR == 0) //all sense white
				{
			pid();
		}
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
		if (SL == 0 && SM == 0 && SR == 0) //all sense white
				{
			pid();
		}
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
		if (SL == 0 && SM == 0 && SR == 0) //all sense white
				{
			pid();
		}
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



}
