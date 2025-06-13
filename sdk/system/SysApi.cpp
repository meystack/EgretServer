
#include "SysApi.h"
//#include <sys/time.h>
// #include <string>
// #include <vector>
// #include <signal.h>
//#include <iostream> 
#include <fstream>  

#include <termios.h> 
#include <unistd.h>
#include <fcntl.h>
void Sleep(unsigned int nSec)
{
	struct timeval delay;
	if(nSec > 1000)
	{
		delay.tv_sec=nSec /1000;
		delay.tv_usec = (nSec % 1000) * 1000;			
	}
	else
	{
		delay.tv_sec=0;
		delay.tv_usec = nSec *1000;		
	}
	select(0,0,0,0,&delay);
} 

int _kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF)
	{
	ungetc(ch, stdin);
	return 1;
	}
	return 0;
}
int _kbhit2 (void)
{
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rdfs);
	FD_SET (STDIN_FILENO, &rdfs);

	select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);

}