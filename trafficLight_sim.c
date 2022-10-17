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
  if(gpio == 11 && args2.pushButtonState == 1){
    //locks the thread with mutex
    pthread_mutex_lock( &mutex1 );
    //reset the push button state
    args2.pushButtonState = 0;
    //swaps the sequence of lights
    args.sequence = 200;
    args2.sequence = 100;
    printf("swapping sequence %d, %d\n", args.sequence, args2.sequence);
    //unlocks the mutex
    pthread_mutex_unlock( &mutex1 );
  }

  if(gpio == 1 && args.pushButtonState == 1){
    //locks the thread with mutex
    pthread_mutex_lock( &mutex1 );
    //reset the push button state
    args.pushButtonState = 0;
    //swaps the sequence of lights
    args.sequence = 200;
    args2.sequence = 100;
    printf("swapping sequence %d, %d\n", args.sequence, args2.sequence);
    //unlocks the mutex
    pthread_mutex_unlock( &mutex1 );
  }


  printf("%d\n",gpio);
  sleep(seconds);
}

// main code to control the work flow of traffic light based on sequence
void *trafficLightDemo(void *_args){
    struct thread_args *args = (struct thread_args *) _args;
    while(1){
       if(args->sequence == 100){
          staticTurnONAndOff(args->green,5);
          staticTurnONAndOff(args->yellow,2);
          staticTurnONAndOff(args->red,7);
       }
       if(args->sequence == 200){
          staticTurnONAndOff(args->red,7);
          staticTurnONAndOff(args->green,5);
          staticTurnONAndOff(args->yellow,2);      
       }
    }
}

//keyboard listener to listen to the keyboard input instead of push button
void flush_buffer (char *buffer); 
void *pushButtonListener(){
  char buffer[MAX_LENGTH + 1];
  memset (buffer, 0, MAX_LENGTH + 1);
    while (fgets (buffer, MAX_LENGTH, stdin) != NULL)
    {
      pthread_mutex_lock( &mutex1 );
      int guess = strtol (buffer, NULL, 10);
      if(guess == 1){
        args.pushButtonState = 1;
        printf("Push button enabled%d\n", guess);
      }
      else{
        printf("Wrong button pressed!.. It's okay");
      }
      pthread_mutex_unlock( &mutex1 );
    }
}

//main program to execute traffic light sequence
int main (void)
{   

    //define the pins for each color LEDs for light 1
    args.red = 1;
    args.green = 2;
    args.yellow = 3;
    args.sequence = 100;
    //the push button is off
    args.pushButtonState = 0;

    //define the pins for each color LEDs for light 1
    args2.red = 11;
    args2.green = 22;
    args2.yellow = 33;
    args2.sequence = 200;
    //the push button is off
    args2.pushButtonState = 0;
    
    displaySystemDetails();
    pthread_t thread1, thread2, pushThread;

    // make threads
    pthread_create(&thread1, NULL, trafficLightDemo, &args);
    pthread_create(&thread2, NULL, trafficLightDemo, &args2);
    pthread_create(&pushThread, NULL, pushButtonListener, NULL);
    
    // wait for them to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(pushThread, NULL);

    printf("\nLED Blink Program is terminated !!!");
    return 0;

} // end of main
