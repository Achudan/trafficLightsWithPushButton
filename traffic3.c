#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <pthread.h>
#include<string.h>
#include<sys/utsname.h>
#include<stdlib.h>

#define LOOP 5
#define MAX_LENGTH 40

// structure of thread with led lights and state
struct thread_args {
  int red;
  int green;
  int yellow;
  int buttonPin;
  int sequence;
  int pushButtonState;
  pthread_mutex_t mutex;
};
//initialize the mutex mutex1
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

//declare two threads, each for each traffic signal
struct thread_args args, args2;

//function to print system details
void displaySystemDetails(){
  struct utsname sysInfo;
  uname(&sysInfo);
  printf("Machine Name : %s\n",sysInfo.machine);
  printf("Node Name : %s\n",sysInfo.nodename);
  printf("System Name : %s\n",sysInfo.sysname);
  printf("Team Member : Hariharan Prabhakar, Achudan Thudukutchi Sadhasivam\n");
}

//function to blink the LED
void blink(int gpio, int seconds){
 for(int i=0;i<seconds;i++){
   printf("Blink ON %d\n",gpio);
   sleep(1);
   printf("BLINK OFF %d\n",gpio);
   sleep(1);
 }
}

//function to turn on and off the LED for the given time
void staticTurnONAndOff(int gpio, int seconds){
  //The sequence of the LED lights are swapped if the pushbutton is enabled for 5 seconds
  if(args2.pushButtonState == 1 || args.pushButtonState == 1){
    //locks the thread with mutex
    pthread_mutex_lock( &mutex1 );

    //reset the push button state
    args2.pushButtonState = 0;
    args.pushButtonState = 0;
    printf("befpre swap %d, %d\n", args.sequence, args2.sequence);

    //swaps the sequence of lights
    args.sequence = args.sequence == 200 ? 100 : 200;
    args2.sequence = args.sequence == 100 ? 200 : 100;
    printf("after swap %d, %d\n", args.sequence, args2.sequence);

    //unlocks the mutex
    pthread_mutex_unlock( &mutex1 );
    return;
  }

  char str[80],str2[80];
  sprintf(str, "/sys/class/gpio/gpio%d/direction", gpio);
  sprintf(str2, "/sys/class/gpio/gpio%d/value", gpio);
  int gpioIOFile = open(str,O_RDWR);
  write(gpioIOFile,"out",3);
  gpioIOFile = open(str2,O_WRONLY);
  write(gpioIOFile,"1",1);
  sleep(seconds);
  write(gpioIOFile,"0",1);
  
}

//turns off the LED connected to the pin gpiopin
void turnOff(int gpioPin){
      char str[80],str2[80];
      sprintf(str, "/sys/class/gpio/gpio%d/direction", gpioPin);
      sprintf(str2, "/sys/class/gpio/gpio%d/value", gpioPin);
      int gpioIOFile = open(str,O_RDWR);
      write(gpioIOFile,"out",3);
      close(gpioIOFile);
      gpioIOFile = open(str2,O_WRONLY);
      write(gpioIOFile,"0",1);
      close(gpioIOFile);
}

// main code to control the work flow of traffic light based on sequence
void *trafficLightDemo(void *_args){
    struct thread_args *args = (struct thread_args *) _args;
    while(1){
       if(args->sequence == 100){
          staticTurnONAndOff(args->green,10);
          staticTurnONAndOff(args->yellow,4);
          staticTurnONAndOff(args->red,14);
       }
       if(args->sequence == 200){
          staticTurnONAndOff(args->red,14);
          staticTurnONAndOff(args->green,10);
          staticTurnONAndOff(args->yellow,4);
       }
    }
}

// void flush_buffer (char *buffer); 

// void *pushButtonListener(){
//   char buffer[MAX_LENGTH + 1];
//   memset (buffer, 0, MAX_LENGTH + 1);
//     while (fgets (buffer, MAX_LENGTH, stdin) != NULL)
//     {
//       pthread_mutex_lock( &mutex1 );
//       int guess = strtol (buffer, NULL, 10);
//       if(guess == 1){
//         args.pushButtonState = 1;
//         printf("Push button enabled %d\n", guess);
//       }
//       else{
//         printf("Wrong button pressed!.. It's okay");
//       }
//       pthread_mutex_unlock( &mutex1 );
//     }
// }

void *pushButtonListener(void *_args){
    struct thread_args *args = (struct thread_args *) _args;
    int counter = 0;
    while (1)
    {
      pthread_mutex_lock( &mutex1 );
      char buttonGPIO[80];
      sprintf(buttonGPIO, "/sys/class/gpio/gpio%d/value", args->buttonPin);
        FILE * f = fopen(buttonGPIO, "r");
        int val=0;
        (void) fscanf(f, "%d", &val);
        printf("button Val : %d\n",val);
        if(val==1){
            counter++;  
            printf("counter : %d\n",counter);
            //check if the push button is pressed for 5 seconds, set the push button to high
            if(counter == 5){
            args->pushButtonState = 1;         
            }
        } else {
            counter = 0;
        }
        fclose(f);
      pthread_mutex_unlock( &mutex1 );
    }
}

//main program to execute traffic light sequence
int main (void)
{   
    //define the pins for each color LEDs for light 1
    args.red = 44;
    args.green = 67;
    args.yellow = 68;
    //sequence 100 represents Green, Yellow and Red
    args.sequence = 100;
    args.buttonPin = 66;
    //the push button is off
    args.pushButtonState = 0;

    //define the pins for each color LEDs for light 2
    args2.red = 20;
    args2.green = 26;
    args2.yellow = 46;
    //sequence 200 represents Red, Yellow and Green
    args2.sequence = 200;
    args.buttonPin = 69;
    //the push button is off
    args2.pushButtonState = 0;
    
    //turn off all the LEDs
    turnOff(args.red);
    turnOff(args.green);
    turnOff(args.yellow);
    turnOff(args2.red);
    turnOff(args2.green);
    turnOff(args2.yellow);
    
    //displays the system and team members detail
    displaySystemDetails();
    pthread_t thread1, thread2, pushThread, pushThread2;

    // make threads
    pthread_create(&thread1, NULL, trafficLightDemo, &args);
    pthread_create(&thread2, NULL, trafficLightDemo, &args2);
    pthread_create(&pushThread, NULL, pushButtonListener, &args);
    // pthread_create(&pushThread2, NULL, pushButtonListener, &args2);
    
    // wait for them to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(pushThread, NULL);
    pthread_join(pushThread2, NULL);

    //program is terminated
    printf("\nLED Blink Program is terminated !!!");
    return 0;

} // end of main

