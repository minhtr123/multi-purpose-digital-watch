#include "mbed.h"
#include "C12832.h"
#include <iostream>
#include <string>


C12832 lcd (D11, D13, D12, D7, D10);
class speaker{
    protected:                                          
    DigitalOut outputSignal;                       
    int status;                                    
    Ticker sounder_f;
public:                                             
    speaker(PinName pin) : outputSignal(pin){off();}    

    void on(void)                                   
    {
        outputSignal = 0;                           
        status = 1;                              
    }

    void off(void)                                  
    {
        outputSignal = 1;                          
        status = 0;                             
    }

    void toggleSpeakerISR(void)                               //Public member function for toggling the speaker
    {
        if (status==1){                                 
            off();                                  
        } else {                                       
            on();
            }
    sounder_f.attach(callback(this, &speaker::toggleSpeakerISR), 1.0/1500);
    }

    int getStatus(void)                            
    {
        return status;                              //Returns whether the speaker is currently on or off
    }
    void detach_function(){
        sounder_f.detach();
    }
};
speaker sounder(D6);
class LED {//begin declaration of class LED
    private:
    DigitalOut red, blue, green; // Declaration of DigitalOut object 
    int status;// status represent the state of the LED
    Ticker flashing;
    public://public declaration
    LED(PinName rd, PinName b, PinName gr) : red(rd), blue(b), green(gr) {off();}//Constructor to provide pin name is assigned to DigitalOut 
    void off (){// member function for when LED is off
        red=1;
        blue=1;
        green=1; // all LEDs are active low so set to 1 would turn them off
        status = 0;// LED is off
    }
    void onBlue(){//// member function for when LED is blue
        blue=0;// pin for blue LED is active low
        status=1;// LED is on
    }
    void onGreen(){// member function for when LED is green
        green=0; // pin for green LED is active low
        status=1;//LED is on
    }
    void offGreen(){
        green=1;
        status=0;
    }
    void toggleISR(){
        if (status==1){
            onGreen();
            status = 0;
        } else {
            offGreen();
            status = 1;
    }
    flashing.attach(callback(this, &LED::toggleISR), 1.0);
    }

    void detach_function(){
        flashing.detach();
    }
};

LED lights(D5,D8,D9);

class Clock
{
private:
int time_hour, time_min, time_secs;
Timer clock_timer;
public:
Clock(int time_hour, int time_min, int time_secs){
    this -> time_hour = time_hour;
    this -> time_min = time_min;
    this -> time_secs = time_secs;

}
void resetClock(){
    clock_timer.reset();
};
void stopClock(){
    clock_timer.stop();
}
void tick(){
    clock_timer.read();
};
void setClock(){
    clock_timer.start();
};
int getHours(){
    if (time_min>=59 && time_secs==59){
    resetClock();
        if (time_hour > 23){
            return 0;
        } else {
            return time_hour++;
        }
    } 
    return time_hour;
}

int getMins()
{   if (time_min > 59){
    return 0;
    }
    if (time_secs==59){
    resetClock();
    return time_min++;
    } 
    return time_min;
}        
int getSecs()
{   
    if(time_secs>=0 && time_secs<=59){
    time_secs=clock_timer.read(); 
    return time_secs;  
} else{
    clock_timer.reset();
    return time_secs;
    }
}
};


class Potentiometer                                 //Begin Potentiometer class definition
{
private:                                            //Private data member declaration
    AnalogIn inputSignal;                           //Declaration of AnalogIn object
    float VDD, currentSampleNorm, currentSampleVolts; //Float variables to speficy the value of VDD and most recent samples

public:                                             // Public declarations
    Potentiometer(PinName pin, float v) : inputSignal(pin), VDD(v) {}   //Constructor - user provided pin name assigned to AnalogIn...
                                                                        //VDD is also provided to determine maximum measurable voltage
    float amplitudeVolts(void)                      //Public member function to measure the amplitude in volts
    {
        return (inputSignal.read()*VDD);            //Scales the 0.0-1.0 value by VDD to read the input in volts
    }
    
    float amplitudeNorm(void)                       //Public member function to measure the normalised amplitude
    {
        return inputSignal.read();                  //Returns the ADC value normalised to range 0.0 - 1.0
    }
    
    void sample(void)                               //Public member function to sample an analogue voltage
    {
        currentSampleNorm = inputSignal.read();       //Stores the current ADC value to the class's data member for normalised values 								(0.0 - 1.0)
        currentSampleVolts = currentSampleNorm * VDD; //Converts the normalised value to the equivalent voltage (0.0 - 3.3 V) and stores 							this information
    }
    
    float getCurrentSampleVolts(void)               //Public member function to return the most recent sample from the potentiometer (in 							volts)
    {
        return currentSampleVolts;                  //Return the contents of the data member currentSampleVolts
    }
    
    float getCurrentSampleNorm(void)                //Public member function to return the most recent sample from the potentiometer 								(normalised)
    {
        return currentSampleNorm;                   //Return the contents of the data member currentSampleNorm  
    }
};

const std::string city_array[20] = {"Midway(GMT-11)","Honolulu(GMT-10)","Vancouver(GMT-8)","Edmonton(GMT-7)","MexicoCity(GMT-6)",
                        "Havana(GMT-5)","Caracas(GMT-4)","St Johns(GMT-3:30)","Sao Paulo(GMT-3)","Manchester(GMT)","Lagos(GMT+1)",
                        "Cairo(GMT+2)","Baghdad(GMT+3)","Tehran(GMT+3:30)", "Saratov(GMT+4)","Karachi(GMT+5)","Hanoi(GMT+7)",
                        "Shanghai(GMT+8)","Pyongyang(GMT+9)","Tokyo(GMT+10)"};
int world_hour[20] = {-11, -10, -8, -7, -6, -5, -4, -3, -3, 0, 1, 2, 3, 3, 4, 5, 7, 8, 9, 10};
int world_min [20] = {0, 0, 0, 0, 0, 0, 0, -30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0};
class SamplingPotentiometer : public Potentiometer {
    private:
        float samplingFrequency, samplingPeriod;
        bool status;
        Ticker sampler;
    public:
        SamplingPotentiometer(PinName p, float v, float fs): Potentiometer(p,v), samplingFrequency(fs){
        samplingPeriod=1.0/samplingFrequency;
        };

        void detach_function(){
            sampler.detach();
        }
// hour--------------------------------------
        int get_hour_pot(){
            sample();
            const float hour_gradient = 23.2/3.3;
            return (int) (hour_gradient*getCurrentSampleVolts());
        }

        void give_hour(){
            lcd.locate(15,12);
            lcd.printf("%02d :", get_hour_pot());
            //lcd.fillrect(30, 20, 35, 25, 0);
            sampler.attach(callback(this, &SamplingPotentiometer::give_hour), samplingPeriod);
        }
// min-------------------------------------------

        int get_min_pot(){
            sample();
            const float min_gradient = 59.2/3.3;
            return (int) (min_gradient*getCurrentSampleVolts());
        }

        void give_min(){
            lcd.locate(25,12);
            lcd.printf(" : %02d", get_min_pot());
            //lcd.fillrect(30, 20, 35, 25, 0);
            sampler.attach(callback(this, &SamplingPotentiometer::give_min), samplingPeriod);
        }
// city ------------------------------------------
        int get_city_pot(){
            sample();
            const float city_gradient = 19.5/3.3;
            return (int) (city_gradient*getCurrentSampleVolts());
        }

        void give_city(){
            lcd.locate(37,0);
            lcd.printf(" %s         ", city_array[get_city_pot()]);
            sampler.attach(callback(this, &SamplingPotentiometer::give_city), samplingPeriod);
        }
//-----------------------count down-----------------------
        int get_min4countdownpot(){
            sample();
            const float hour_gradient = 59.2/3.3;
            return (int) (hour_gradient*getCurrentSampleVolts());
        }

        void give_min4countdown(){
            lcd.locate(0,10);
            lcd.printf("   %02d :", get_min4countdownpot());
            //lcd.fillrect(30, 20, 35, 25, 0);
            sampler.attach(callback(this, &SamplingPotentiometer::give_min4countdown), samplingPeriod);
        }

        int get_sec4countdownpot(){
            sample();
            const float hour_gradient = 59.2/3.3;
            return (int) (hour_gradient*getCurrentSampleVolts());
        }

        void give_sec4countdown(){
            lcd.locate(25,10);
            lcd.printf(" : %02d            ", get_sec4countdownpot());
            //lcd.fillrect(30, 20, 35, 25, 0);
            sampler.attach(callback(this, &SamplingPotentiometer::give_sec4countdown), samplingPeriod);
        }
};

int hour_logic(int index, int now_hour){
    if (now_hour+world_hour[index]>=0 && now_hour+world_hour[index]<=23){
    return world_hour[index];
    } else if (now_hour+world_hour[index]<=0) {
        return 24+(now_hour+world_hour[index]-now_hour*2);
    } else if (now_hour+world_hour[index]>=24){
        return -24+(now_hour+world_hour[index]-now_hour);
    } else {return 0;}
}

int min_logic(int index, int now_min){
    if (now_min+world_min[index]>=0 && now_min+world_min[index]<=59){
        return world_min[index];
    } else if (now_min+world_min[index]<=0){
        return -world_min[index];
    } else if (now_min+world_min[index]>=60){
        return world_min[index]-60;
    } else {return 0;}
}


typedef enum {initialisation, setTime, worldTime, stopWatch_inactive, stopWatch_active, stopWatch_stop, setCountdown, CountingDown, endCountDown} ProgramState;    //Definition of the enum that refers to the states of the program
ProgramState state;

class stopwatch{
    private:
        Timer stopwatch_time;
        float time;
    public:
        float get_time(){
            time = stopwatch_time.read();
            return time;
        }

        void start_time(){
            stopwatch_time.start();
        }

        void stop_time(){
            stopwatch_time.stop();
        }

        void reset_time(){
            stopwatch_time.reset();
        }
};

class count_down_timer{
    private:
        Timer display_period;
        int save_min, save_secs;
    public:
    count_down_timer(int save_min, int save_secs){
        this -> save_min = save_min;
        this -> save_secs = save_secs;
    }
        int get_countdown_time(){
            return save_min*60+save_secs;
        }

        void stop_and_reset(){
            display_period.stop();
            display_period.reset();
        }

        void display_count_down(int save_min, int save_secs){
            display_period.start();
            lcd.locate(0,0);
            lcd.printf("Count down period    ");

            lcd.locate(15,10);
            lcd.printf("%d / %d s  ", get_countdown_time()-(int)display_period.read(), get_countdown_time());
        }
};

stopwatch watch;
Timeout timeout;
count_down_timer countDown(0,0);

void countdownISR(){
    lights.onGreen();
    sounder.toggleSpeakerISR();
    switch(state){
        case(initialisation):
            state=endCountDown;
            break;
        case(setTime):
            state=endCountDown;
            break;  
        case(worldTime):
            state=endCountDown;
            break;
        case(stopWatch_inactive):
            state=endCountDown;
            break;
        case(stopWatch_active):
            state=endCountDown;
            break; 
        case(stopWatch_stop):
            state=endCountDown;
            break;        
        case(CountingDown):
            state=endCountDown;
            break;
        default:
            state=initialisation;
    }
    lights.detach_function();

}

void fireISR(){
    sounder.detach_function();
    switch(state){
        case (initialisation):
            state = setTime;
            break;
        case (setTime):
            state = initialisation;
            break;

        case(worldTime):
            state=worldTime;
        break;


        case(stopWatch_inactive):
            state=stopWatch_active;
            break;
        case(stopWatch_active):
            state=stopWatch_stop;
            break;
        case(stopWatch_stop):
            state=stopWatch_inactive;
            break;


        case(setCountdown):
            lights.offGreen();
            state=CountingDown;
            timeout.attach(&countdownISR, countDown.get_countdown_time());
            lights.toggleISR();
            break;
        case (CountingDown):
            state=CountingDown;
            break;
        case (endCountDown):
            lights.offGreen();
            state=setCountdown;
            break;

        default:
            state = initialisation;
    }
}

void rightISR(){
    sounder.detach_function();
    switch(state){
        case(initialisation):
            state=worldTime;
            break;
        case(worldTime):
            state=stopWatch_inactive;
            break;

        case(stopWatch_inactive):
            state=setCountdown;
            break;
        case(stopWatch_active):
            state=stopWatch_active;
            break;
        case(stopWatch_stop):
            state=setCountdown;
            break;   
        case(setCountdown):
            state=initialisation;
            break;
        case (CountingDown):
            state=initialisation;
            break; 

        case(endCountDown):
            lights.offGreen();
            state=initialisation;
            break; 

        case(setTime):
            state=setTime;
            break;

        default:
            state = initialisation;
    }       
}

void leftISR(){
        sounder.detach_function();
        switch(state){
        case(initialisation):
            state=setCountdown;
            break;
        case(setCountdown):
            state=stopWatch_inactive;
            break;

        case(stopWatch_inactive):
            state=worldTime;
            break;
        case(worldTime):
            state=initialisation;
            break;  
        case (CountingDown):
            state=stopWatch_inactive;
            break;
        case (endCountDown):
            lights.offGreen();
            state=stopWatch_inactive;
            break;

        case(setTime):
            state=setTime;
            break;

        case(stopWatch_active):
             state=stopWatch_active;
            break;                
        default:
            state = initialisation;
    }       
}



int main() {
    state = initialisation;

    InterruptIn right(A5);
    InterruptIn left(A4);
    InterruptIn fire(D4);

    fire.rise(&fireISR);
    right.rise(&rightISR);
    left.rise(&leftISR);
    
    SamplingPotentiometer pot_1 (A0,3.3,(23/3.3)*10);
    SamplingPotentiometer pot_2 (A1,3.3,(59/3.3)*10);
    SamplingPotentiometer pot_R (A1, 3.3,(20/3.3)*10);
    SamplingPotentiometer pot_min_countdown(A0,3.3,(59/3.3)*10);
    SamplingPotentiometer pot_sec_countdown(A1,3.3,(59/3.3)*10);    
    int save_second,save_minute, index, now_hour, now_min, save_secs, save_min;
    Clock timeUpdated (0,0,0);

    while(1) {

        switch (state){
            case (initialisation):
                pot_1.detach_function();
                pot_2.detach_function();
                timeUpdated.setClock();
                lcd.fillrect(0,10,70,20,0);
                lcd.locate(0, 0);
                lcd.printf("press Fire to set time              ");
                lcd.locate(0,20);                               
                lcd.printf("%02d:%02d:%02d     ", timeUpdated.getHours(), timeUpdated.getMins(), timeUpdated.getSecs());
                break;
            
            case (setTime):
                lcd.fillrect(80,0,100,20,0);
                lcd.fillrect(0,20,35,30,0);
                lcd.locate(0,0);
                lcd.printf("Set time (HH:MM)");
                // turn pot to give hour
                pot_1.give_hour();
                pot_1.detach_function();

                //turn pot to give minute
                pot_2.give_min();
                pot_2.detach_function();

                timeUpdated = Clock(pot_1.get_hour_pot(), pot_2.get_min_pot(), timeUpdated.getSecs());
                break;
            
            case(worldTime):
                index = pot_R.get_city_pot();
                pot_R.detach_function();
                
                now_hour = timeUpdated.getHours();
                now_min = timeUpdated.getMins();

                lcd.locate(0,0);
                lcd.printf("Location");
                
                pot_R.give_city();
                pot_R.detach_function();
                //world time--------------------------------------
                lcd.locate(0,10);
                if(now_min+world_min[index]>=0 && now_min+world_min[index]<=59){                             
                    lcd.printf("%02d:%02d:%02d    ", timeUpdated.getHours()+hour_logic(index, now_hour), timeUpdated.getMins()+min_logic(index, now_min), timeUpdated.getSecs());
                } else if (now_min+world_min[index]<=0) {
                    lcd.printf("%02d:%02d:%02d    ", timeUpdated.getHours()+hour_logic(index, now_hour)-1, timeUpdated.getMins()+min_logic(index, now_min), timeUpdated.getSecs());
                } else if(now_min+world_min[index]>=59) {
                    lcd.printf("%02d:%02d:%02d    ", timeUpdated.getHours()+hour_logic(index, now_hour)+1, timeUpdated.getMins()+min_logic(index, now_min), timeUpdated.getSecs());
                }
                //--------------home time-------------------------
                lcd.locate(0,20);                               
                lcd.printf("%02d:%02d:%02d    ", timeUpdated.getHours(), timeUpdated.getMins(), timeUpdated.getSecs());
                break;

            case(stopWatch_inactive):
                lcd.fillrect(0,10,50,20,0);
                watch.stop_time();
                watch.reset_time();
                lcd.locate(0,0);
                lcd.printf("Stopwatch: fire to start       ");
                lcd.fillrect(0,10,50,20,0);
                lcd.locate(0,20);
                lcd.printf("Time: %.2f s", watch.get_time()); 
                lcd.fillrect(0,10,70,20,0);
                break;
            
            case(stopWatch_active):
                lcd.fillrect(0,10,35,20,0);
                lcd.locate(0,0);
                lcd.printf("Stopwatch: Active        ");
                watch.start_time();
                lcd.locate(0,20);
                lcd.printf("Time: %.2f s", watch.get_time());
                lights.onBlue(); 
                break;
            
            case(stopWatch_stop):
                lcd.fillrect(0,10,35,20,0);
                lights.off();
                watch.stop_time();
                lcd.locate(0,0);
                lcd.printf("Stopwatch: Inactive      ");
                lcd.locate(0,20);
                lcd.printf("Time: %.2f s", watch.get_time()); 
                break;

            case(setCountdown):
                lcd.locate(0,0);
                lcd.printf("Set countdown period      ");
                lcd.locate(0,20);
                lcd.printf("                           ");
                save_minute=pot_min_countdown.get_min4countdownpot();
                pot_min_countdown.give_min4countdown();
                pot_min_countdown.detach_function(); 

                save_second=pot_sec_countdown.get_sec4countdownpot();               
                pot_sec_countdown.give_sec4countdown();
                pot_sec_countdown.detach_function();
                countDown = count_down_timer(save_minute, save_second);
                break;
            case(CountingDown):
                countDown.display_count_down(save_min, save_secs);
                break;
            case(endCountDown):
                timeout.detach();
                countDown.stop_and_reset();
                lcd.locate(0,0);
                lcd.printf("Count down period           ");
                lcd.locate(0,10);
                lcd.printf("has elapsed!          ");
                lcd.fillrect(0,19,55,30,0);
                break;    
            default:
               state = initialisation; 
        }
    }
}   
