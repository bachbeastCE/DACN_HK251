#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int main (int argc, char* agrv[]){
   int fd;
   struct termios tty;
   char buff[256];

   //open device driver serial

   fd = open("/dev/serial0", O_RDWR | O_NOCTTY | O_SYNC);
   if (fd < 0){
      perror("open");
      return -1;
   }
   if (tcgetattr(fd, &tty) != 0) { 
      perror("tcgetattr"); 
      close(fd); 
      return -1; 
   }  
   
   cfsetospeed(&tty, B115200);
   cfsetispeed(&tty, B115200);

   //8N1 mode
   tty.c_cflag &= ~PARENB; //no parity bit
   tty.c_cflag &= ~CSTOPB; //1 stop bit
   tty.c_cflag &= ~CSIZE; //clear mask
   tty.c_cflag |= CS8; //set data bit = 8;
   tty.c_cflag |= CREAD | CLOCAL;
   //read at least 1byte
   tty.c_cc[VMIN] = 1;
   tty.c_cc[VTIME] = 1; // timeout ~0.1s;

   if((tcsetattr(fd,TCSANOW,&tty)) != 0) // Set the attributes to the termios structur
      printf("\n  ERROR ! in Setting attributes");
   else
      printf("\n  BaudRate = 115200 \n  StopBits = 1 \n  Parity   = none");

   printf("UART READY, WAITING DATA....");

   while(1){
      int n = read(fd, buff, sizeof(buff));
      if (n > 0){
         printf("RX(%d byte): ", n);
         for (int i = 0; i < n; ++i){
	   printf("%c", buff[i]);
         }
         fflush(stdout);
      }
   } 
   close(fd);
   return 0;
   
}
