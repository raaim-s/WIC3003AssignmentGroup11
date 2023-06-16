#include <string>
#include "mbed.h"
#include "C12832.h"
#include "MMA7660.h"
#include "SongPlayer.h"


C12832 lcd(p5, p7, p6, p8, p11);
MMA7660 MMA(p28, p27);
AnalogIn Pot1(p19);
AnalogIn Pot2(p20);
BusIn joy(p15,p12,p13,p16);
DigitalIn click(p14);
SongPlayer spkr(p26);
PwmOut r (p23);
PwmOut g (p24);
PwmOut b (p25);



float opening_note[18]= {1568.0,1396.9,1244.5,1244.5,1396.9,1568.0,1568.0,1568.0,1396.9,
                 1244.5,1396.9,1568.0,1396.9,1244.5,1174.7,1244.5,1244.5, 0.0
                };
float opening_duration[18]= {0.48,0.24,0.72,0.48,0.24,0.48,0.24,0.24,0.24,
                     0.24,0.24,0.24,0.24,0.48,0.24,0.48,0.48, 0.0
                    };

float game_notes[17]={329.63, 311.13, 329.63, 311.13, 329.63, 246.94, 293.66, 261.63, 220, 146.83, 174.61, 220, 246.94,
            174.61, 233.08, 246.94, 261.63};

float game_duration[17]={0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.9,0.3,0.3,0.3,0.9,0.3,0.3,0.3,0.9};

float ending_notes[4]= {146.83, 138.59, 130.8128, 123.4708};

float ending_duration[4]= {0.6, 0.6, 0.6, 1.2};


int controls = 0;
const string display_menu[]   = {"Accelerometer", "Potentiometers", "Joystick"};

void menu(){
    lcd.printf("WELCOME TO FLAPPYSNAKE");
    spkr.PlaySong(opening_note,opening_duration);
    wait(7);
    lcd.cls();
    while(1){
    r = 1.0;
    g = 0.5;
    b = 1.0;
    lcd.locate(0,3);
    lcd.printf("PLEASE SELECT CONTROLS\n");
    lcd.printf("               ");
    lcd.locate(0,10);
    lcd.printf("> ");
    lcd.printf(display_menu[controls].c_str());
    if(joy == 0x4){
        if(controls == 0) controls = 2;
        else controls--;
    } 
    if(joy == 0x8){
        if(controls == 2) controls = 0;
        else controls++;
    }
    if(click){
        break;
    }
    }
}

bool check_collision(int x, int y, int i, int opening){

    if(controls == 0){x+=64;y+=15;}
    if( i <= x && x <= i+4 && 0 <= y && y <= opening-5 ) {return true;}
    if( i <= x && x <= i+4 && opening+5 <= y && y <= 127 ) {return true;}
    else return false;
}

void draw_ball(int x, int y){
    if(controls == 0) {lcd.fillcircle(x+63, y+15, 1, 1);}
    else lcd.fillcircle(x, y, 1, 1);
}


void update_ball(int &x, int &y){
    switch(controls){
        case 0:{ // Accelerometer
            x = (x - MMA.x() * 32.0)/1.5;
            y = (y + MMA.y() * 8.0)/1.5;
            }
            break;
        case 1:{ // Potentiometers
            x = Pot1.read() * 128;
            y = Pot2.read() * 32;
            if(x >= 128) x = 128;
            if(x <= 0) x = 0;
            if(y >= 32) y = 32;
            if(y <= 0) y = 0;
            }
            break;
        case 2:{ //Joystick

            if(joy == 0x4){x=x-2;} //left
            if(joy == 0x8){x=x+2;} //right
            if(joy == 0x1){y=y-2;} //up
            if(joy == 0x2){y=y+2;} //down
            if(x >= 128) x = 128;
            if(x <= 0) x = 0;
            if(y >= 32) y = 32;
            if(y <= 0) y = 0;
            }
            break;
    }

}

void hsv2rgb(float H,float S, float V)
{
    float f,h,p,q,t;
    int i;
    if( S == 0.0) {
        r = 1.0 - V;  // invert pwm !
        g = 1.0 - V;
        b = 1.0 - V;
        return;
    }
    if(H > 360.0) H = 0.0;   // check values
    if(S > 1.0) S = 1.0; 
    if(S < 0.0) S = 0.0;
    if(V > 1.0) V = 1.0;
    if(V < 0.0) V = 0.0;
    h = H / 60.0;
    i = (int) h;
    f = h - i;
    p = V * (1.0 - S);
    q = V * (1.0 - (S * f));
    t = V * (1.0 - (S * (1.0 - f)));

    switch(i) {
        case 0:
            r = 1.0 - V;  // invert pwm !
            g = 1.0 - t;
            b = 1.0 - p;
            break;
        case 1:
            r = 1.0 - q;
            g = 1.0 - V;
            b = 1.0 - p;
            break;
        case 2:
            r = 1.0 - p;
            g = 1.0 - V;
            b = 1.0 - t;
            break;
        case 3:
            r = 1.0 - p;
            g = 1.0 - q;
            b = 1.0 - V;
            break;
        case 4:
            r = 1.0 - t;
            g = 1.0 - p;
            b = 1.0 - V;
            break;
        case 5:
        default:
            r = 1.0 - V;
            g = 1.0 - p;
            b = 1.0 - q;
            break;
    }
}


void game_over(int score){
    lcd.cls();
    lcd.locate(0,3);
    lcd.printf("GAME OVER!\nYOUR FINAL SCORE IS %d!",score);
    r.period(0.001);
    while(1) { //keep led RED
        r = 0.5;
        g = 1.0;
        b = 1.0;
    }
}

int x=63,y=15;

int main(){
    menu();
    int score = 0;
    float h;       //  hue 
    float s,v;   // saturation and  value;
    unsigned short temp;
    s = 1.0;
    spkr.PlaySong(game_notes, game_duration);
    r.period(0.001);
    while(1){
        int opening = rand() % 23 + 5;
        for(int i = 0; i <= 127; i++){
            lcd.cls();
            lcd.locate(0,3);
            lcd.fillrect(i,0,i+4,opening-5,1); //upper box
            lcd.fillrect(i,opening+5,i+4,127,1); //lower box
            update_ball(x, y);
            draw_ball(x, y);
            temp = Pot1.read_u16();
            temp = temp >> 6;        // truncate to 10 Bit
            h = temp * 0.3515625;  // scale to 0 - 360;
            // get Poti 2 fo value
            temp = Pot2.read_u16();
            temp = temp >> 6;
            v = temp * 0.0009765624;  // scale to 0 - 1;
            hsv2rgb(h,s,v);
            if(check_collision(x, y, i, opening)){spkr.PlaySong(ending_notes,ending_duration); wait(3); game_over(score);}
            wait(0.01);
      }
      score +=1;
      if(score%2 == 0){spkr.PlaySong(game_notes, game_duration);}
   }

}

