#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pigpiod_if2.h>

#define WAVES 5
#define DIR 22

struct speed {
    int frequency;
    int steps;
};

void add_string(char* dest, char* src,int start, int length);


int main(int argc, char *argv[]) {
    int pi, wid[WAVES], l_steps[WAVES], reverse = 0;
    float tick1, tick2; //declaring variables
    pi = pigpio_start(0, 0); //connecting to the pigpio daemon
    if (pi < 0) {
        return -1; //closing program if failure to connect
    }

    struct speed speeds[WAVES] = { //various ramp speeds
            {800,  50},
            {960,  100},
            {1000, 200},
            {1200, 300},
            {1300, -1} //-1 to indicate to use this for as many steps as possible
    };

    int t_steps = 500;
    int pin = 16;


    //getting input arguments
    if (argc >= 5) {
        for (int i = 1; i < (argc - 1); i++) { //skip first entry as it is the program name
            if (!strcmp(argv[i], "-p")) {
                pin = atoi(argv[i + 1]);
            }
            if (!strcmp(argv[i], "-s")) {
                t_steps = atoi(argv[i + 1]);
            }
        }
    }

    if(t_steps<0){
        t_steps *= -1;
        reverse = 1;
    }
    
    set_mode(pi, pin, PI_OUTPUT); //setting signal pin as output
    set_mode(pi, DIR, PI_OUTPUT); //setting direction pin as output
    gpio_write(pi,DIR,reverse); //writing direction value 

    char *buff = calloc(WAVES * 14, sizeof(char));

    
    
    int r_steps = t_steps;
    for (int i = 0; i<WAVES; i++) {
        //generating waveforms for each frequency
        wave_add_generic(pi, 2, (gpioPulse_t[])
                {{1 << pin, 0,        250000 / speeds[i].frequency}, //convert given frequency to microseconds
                 {0,        1 << pin, 250000 / speeds[i].frequency}});
        wid[i] = wave_create(pi); // generate one pulse at the given frequency
        //printf("%d\n",500000 / speeds[i].frequency);



        //computing number of steps at each frequency to reach total number of steps
        if (speeds[i].steps==-1){
            l_steps[i] = r_steps;
            r_steps = 0;
        }else if(r_steps >= speeds[i].steps) {
            l_steps[i] = speeds[i].steps;
            r_steps -= speeds[i].steps;
        } else {
            l_steps[i] = r_steps;
            r_steps = 0;
        }
        //printf("%d\n",l_steps[i]);

        //generate ramp-up waveform
        char tempbuff[7] = {255,0,(char)wid[i],255,1,(char)(l_steps[i] & 255),(char)(l_steps[i] >> 8)};
        add_string(buff,tempbuff,7*i,7);
        //for(int i = 0; i<  WAVES * 14; i++){
        //printf("%d,",buff[i]);
    //}
    //printf("\n");
    }
    
    
    //compensation on ramp down if odd number of steps is entered
    if(t_steps % 2){
        l_steps[0]++;
    }
    
    
    //generate ramp-down waveform
    for (int i = 1; i<=WAVES; i++){
        char tempbuff[7] = {255,0,(char)wid[WAVES-i],255,1,(char)(l_steps[WAVES-i] & 255),(char)(l_steps[WAVES-i] >> 8)};
        add_string(buff,tempbuff,7*(WAVES+i-1),7);
        //for(int i = 0; i<  WAVES * 14; i++){
        //printf("%d,",buff[i]);
    //}
    //printf("\n");
    }


    for(int i = 0; i<  WAVES * 14; i++){
        printf("%d ",buff[i]);
    }
    printf("\n");
    
    printf("Transmitting Waveform\n");
    tick1 = get_current_tick(pi);
    wave_chain(pi, buff, WAVES * 14);
    
    while (wave_tx_busy(pi)) usleep(100000);
    tick2 = get_current_tick(pi);
    printf("Time for motor: %.2f\n", (float)(tick2 - tick1)/1000000.0);

    printf("Waveform Transmitted\n");

    for (int i = 0; i<WAVES; i++) {
        wave_delete(pi, wid[i]);
    }
    
    free(buff);
    
    gpio_write(pi,DIR,0); //clearing direction value 
    
    pigpio_stop(pi);
    printf("\nreturn 0\n");
    return 0;
}


void add_string ( char* dest, char* src,int start, int length){
    for(int i=0; i<length; i++){
        *(dest+start+i)=*(src+i);
    }
}