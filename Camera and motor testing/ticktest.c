#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pigpiod_if2.h>

int main(void) {
    int pi; 
    float tick1, tick2; //declaring variables
    pi = pigpio_start(0, 0); //connecting to the pigpio daemon
    if (pi < 0) {
        return -1; //closing program if failure to connect
    }

    char photo_command[50];
    strcpy(photo_command, "gphoto2 --capture-image");

    tick1 = get_current_tick(pi);
    system(photo_command);
    tick2 = get_current_tick(pi);
    printf("Time for photo: %.2f\n", (float)(tick2 - tick1)/1000000.0);

    return 0;
}