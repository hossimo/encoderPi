// GPIO test

#define BCM2708_PERI_BASE      0x20000000
#define GPIO_BASE              (BCM2708_PERI_BASE + 0x200000) // GPIO Controller

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int mem_fd;
void *gpio_map;

//IO access
volatile unsigned *gpio;

// globals
static volatile int keepRunning = 1;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

// enums
typedef enum {kWAIT, kRIGHT, kLEFT} encodermode;


// fuction definations
void setup_io();
void printButton(int g);
void intHandler (int dummy);


int main(int argc, char **argv)
{
  int pattern [4] = {0, 1, 3, 2};
  // setup signal handler
  signal (SIGINT, intHandler);

  // Set up gpi pointer for direct register access
  setup_io();

  // Switch GPIO 7..11 to output mode

 /************************************************************************\
  * You are about to change the GPIO settings of your computer.          *
  * Mess this up and it will stop working!                               *
  * It might be a good idea to 'sync' before running this program        *
  * so at least you still have your code changes written to the SD-card! *
 \************************************************************************/

  // Set GPIO pins 7-11 to inputs
  for (int g=7; g<=11; g++)
  {
    INP_GPIO(g); // must use INP_GPIO before we can use OUT_GPIO
  }

  // turn the cursor off
  printf("\n\e[?25l");
  printf("\n");
  printf("GPIO 10│11\n");
  printf("     ──┼──\n");
  fflush(stdout);

  encodermode mode = kWAIT;
  int lastPosition = -1;
  int currentPosition = -1;
  int cycle = -1;
  long counter = 0;

while (keepRunning)
{
  //load up the GPIOs
  int result [2];
  result[0] = GET_GPIO(10) == 0 ? 0 : 1;
  result[1] = GET_GPIO(11) == 0 ? 0 : 1;

  currentPosition = result[0] + (result[1] << 1);

  // if we havent moved then start over
  if (lastPosition == currentPosition)
    continue;

  //not yet sure where we are
  if (mode == kWAIT) {
    for (int i = 0; i <= 3; i++) {
      if (pattern[i] == currentPosition)
        {
          cycle = i;
          break;
        }
    }
  }

  // if the last is +/- 1 place in the pattern then adjust the cycle
  if (lastPosition == pattern[cycle == 3 ? 0 : cycle + 1]) {
    mode = kLEFT;
    cycle = cycle == 0 ? 3 : --cycle;
    counter++;
  }
  else if (lastPosition == pattern[cycle == 0 ? 3 : cycle - 1]) {
    mode = kRIGHT;
    cycle = cycle == 3 ? 0 : ++cycle;
    counter--;
  }
  else // we skipped 1 or more ticks.
    mode = kWAIT;

  char * resultString;
  switch (mode) {
    case kWAIT:
      resultString = "--";
      break;
    case kRIGHT:
      resultString = ">>";
      break;
    case kLEFT:
      resultString = "<<";
      break;
  }

  printf("\e[100D      %d│ %d %s %ld    ",
    result[0],
    result[1],
    resultString,
    counter
  );
  // printf("[%d]l:%d c:%d - %s\n",cycle, lastPosition, currentPosition, resultString);

  lastPosition = currentPosition;
}
// turn the curson back on
printf("\n\n");
printf("\e[?25h");
fflush(stdout) ;

  return 0;

}


//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;


} // setup_io

void printButton(int g)
{
  if (GET_GPIO(g)) // !=0 <-> bit is 1 <- port is HIGH=3.3V
    printf("Button pressed!\n");
  else // port is LOW=0V
    printf("Button released!\n");
}

void intHandler (int dummy) {
  keepRunning = 0;
}
