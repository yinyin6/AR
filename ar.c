#include "main.h"
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"

/*
 if receive signal, start moves
 black line+1=0
 if black line =2
//max detect distance=4000<4800
 */
int BlackLine = 0; // to count how many black line we ssen
int counter=0;
//LTSL=lime tracking sensor at left
//LTSM=lime tracking sensor at middle
//LTRL=lime tracking sensor at right
//if(gpio_read(LTSL)==1 WHEN 1 IS DETECTED BLACK
const int signal =0;// for example signal unit at 0
const int conduct=1;// for example conduct unit at 1

int f1 = 1;
int f2 = 1;
int f3 = 1;
int SL = 1;
int SM = 1;
int SR = 1;

void forward();
void stop();
void left();
void right();
void pid() {
	SL = f1;
	SM = f2;
	SR = f3;
}


//farest4000 only
void loop(void) {


	R= gpio_read(TOF_Right);
	B= gpio_read(TOF_Back);
	if(B<800){forward();}
	if((R>610){right();}// exceed this limit the car will crash the wall
	if(B<800){forward();}
	if(B>800){
		if(R>distance_signal){right()}
		if(R<distance_signal){left();}
		if(R==distance_signal){forward);}
	}




}




























//if receive signal start move
void loop(void) {
	SL = gpio_read(LTSL);
	SM = gpio_read(LTSM);
	SR = gpio_read(LTSR);

	if (BlackLine == 0 || BlackLine == 1) {

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
	if(BlackLine==2){
		if(counter==0){  //turn 90 degree
			if(F<=400){stop();counter=counter+1;}
			else{right();}
		}
		if(counter==1){  //forward
			if(F>283){forward();}
			else{stop();counter=counter+1;}
		}
		if(counter==2){ //turn 90 degree
			if(R>44+145+44 && F<=600){left();}// 44+145+44 the distance signal to the right wall for box 3
			else{stop();counter=counter+1;}
		}
		if(counter==3){  //forward&catch
			if(B<=335-50){stop();catch();counter=counter+1;}
			else{
				if(R>distance_signal){right();}
				if(R<distance_signal){left();}
				if(R==distance_signal){forward();}
				}
		}
		if(counter==4){ //backward
			if(B<655-50+70){backward();}
			else{
				if(B<=distance_signal){stop();conter=counter+1;}
				else{left();}
				}
			}
		if(counter==5){ //return to middle line
			if(B>400){forward();}
			else{
				if(R>400){right();}
				else{stop();BlackLine+=1; counter=0;}
				}
			}
		}

	if(BlackLine==3){
		// if(L>1000) move more 400,then turn right,to the yellow region
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





}
