/* SAI Engine - by Matt Allen */
// Added github.com repository for this to keep track of changes 11/20/14

/* Constantly tries to read file "/var/tmp/a4m/socat_output_injector_fifo" for buffered message
 * and then outputs to write file "/var/tmp/a4m/socat_smart_injector_fifo" for response from pseudo
 * smart injector 
 *
 * INPUT:  /var/tmp/a4m/socat_output_injector_fifo
 * OUTPUT: /tmp/inj_rx file populated with SP response to be read by delivery
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <fstream>
#define RXFILE1		"/var/tmp/a4m/socat_smart_injector_input_data_file1"
#define RXFILE2		"/var/tmp/a4m/socat_smart_injector_input_data_file2"
#define PIPE_FIFO1   	"/var/tmp/a4m/socat_output_smart_injector_fifo1"
#define PIPE_FIFO2   	"/var/tmp/a4m/socat_output_smart_injector_fifo2"
#define debugger { printf("%s:%d\n",__FILE__,__LINE__);}
#define MAX_BUF 20
#define MAX_BUF_LEN 17

//  File stream for crash log
  std::ofstream logger("log.txt");

//void log( std::string file, int line, std::string message = "" )
void log( std::string message = "" )
{
	//Write message to file
	logger << __FILE__<< "[" << __LINE__ << "]: " << message << std::endl;
} 

//#define DEBUG 0
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
	printf("USAGE:  ./SAI [option]\n");
	printf("1.  FIFO #1\n");
	printf("2.  FIFO #2\n");
	printf("3.  BOTH FIFOs (not implemented yet)\n");
	return(1);
  }

  char buffer[MAX_BUF];
  char response[MAX_BUF];
  char tempBuf[40];
  char address_buf[3];

  FILE * pFile;
  long lSize;
  size_t result;
  double NRT = 0.0;
  double NRT2 = 0.0;
  double NRT3 = 0.0;
  int pipe, buf_size, inj_address;

  // SETTING UP WHICH FIFO TO USE FOR COMMs TESTING
  log("***STARTING UP SAI BLACK BOX***");
  if ((atoi(argv[1])) == 1)
  {
  	printf("INCOMING FIFO -> %s\n", PIPE_FIFO1);
  	printf("OUTGOING FILE -> %s\n", RXFILE1);
	log("FIFO -- /var/tmp/a4m/socat_output_smart_injector_fifo1");
  }
  else if ((atoi(argv[1])) == 2)
  {

  	printf("INCOMING FIFO -> %s\n", PIPE_FIFO2);
  	printf("OUTGOING FILE -> %s\n", RXFILE2);
	log("FIFO -- /var/tmp/a4m/socat_output_smart_injector_fifo1");
  }

  memset(buffer, 0, MAX_BUF);

  while(1)
  {
	//Read in inj_tx to buffer
	if ((atoi(argv[1])) == 1)
		if ( (pipe = open( PIPE_FIFO1 , O_RDONLY)) < 0) log("ERROR - PIPE1 missing");
	if ((atoi(argv[1])) == 2)
		if ( (pipe = open( PIPE_FIFO2 , O_RDONLY)) < 0) log("ERROR - PIPE2 missing");

	if ( (buf_size = read(pipe, buffer, MAX_BUF)) < 0) log("ERROR - Buffer misread");
	buffer[buf_size] = 0; //null terminate string

	printf("\E[31;40mIncoming:  %s\E[0m  ", buffer);

#ifdef DEBUG
	printf("[ ");
	for (int i=0;i<MAX_BUF;i++)
	  printf("\E[31;40m%02x ", buffer[i]);
	printf("]\E[0m");
#endif
	printf("\n");

	// Pull out address to strncat into response later
	memcpy(address_buf, buffer, 3);
	address_buf[3] = 0;
	inj_address = atoi(address_buf);

	strcpy(response, address_buf);

	// Parse buffer, supply response buffer

	//TITAN INJECTOR
	if( strncmp ("BI 001", buffer+3, 6) == 0 )
	{
	    NRT +=0.02; // increment the NRT for additive totals
	    strcat(response, "OK");
	}
	else if ( strncmp ("AS 001 0010.0",buffer+3, 13) == 0 )
	   strcat(response, "OK");
	else if ( strncmp ("AL 001", buffer+3, 6) == 0)
	   strcat(response, "OK");
	else if ( strncmp ("UL 001", buffer+3, 6) == 0)
	   strcat(response, "OK");
	else if ( strncmp ("PS 001 0010", buffer+3, 11) == 0)
	  strcat(response,"OK");
	else if ( strncmp ("ac 001", buffer+3, 6) == 0)
	  strcat(response,"ac 001 0000");
	else if ( strncmp ("at 001", buffer+3, 6) == 0)
	{
	  sprintf(tempBuf, "at 001 %10.1f", NRT);
	  strcat(response, tempBuf);
	}
	else if ( strncmp ("ls 001", buffer+3, 6) == 0)
	{
	  sprintf(tempBuf, "ls 001 %09.4f 0000", NRT);
	  strcat(response, tempBuf);
	}	

	//BLENDPAK/MINIPAK
	else if ( strncmp ("EX 050", buffer+3, 6) == 0)
	{
	  if(inj_address == 72) NRT2 += 0.01; // increment the NRT for additive totals
	  if(inj_address == 83) NRT3 += 0.01; // increment the NRT for additive totals
	  strcat(response,"OK");
	}
	else if (strncmp("EX", buffer+3, 2) == 0)
	  strcat(response,"OK");
	else if ( strncmp("RV 802", buffer+3, 6) == 0 )
	  strcat(response, "RV 802 0000");
	else if (strncmp("RV 860", buffer+3, 6) == 0)
	{
	  sprintf(tempBuf, "RV 860 %09.3f", NRT2);
	  strcat(response, tempBuf);
	}
	else if (strncmp("RV 850", buffer+3, 6) == 0) // MINI-PAK POLL_TOTALS
	{
	  sprintf(tempBuf, "RV 850 %09.3f", NRT3);
	  strcat(response, tempBuf);
	}
	else if (strncmp("WV", buffer+3, 2) == 0)
	  strcat(response,"OK");

	// GENERAL ERROR RESPONSES
	else if ( strncmp ("", buffer, MAX_BUF_LEN) == 0)
	{
	  memset(response,0,MAX_BUF);
	  printf("\E[33;40m[ERROR -- NO INC BUFFER]\E[0m\n");
	  log("Error:  No incoming buffer detected");
	  continue;
	}
	else
	{
	  strcat(response,"NO00");
	  printf("\E[33;40m[ERROR -- NO CMD MATCH]\E[0m\n");
	  log("Error:  No command match");
	}

	printf("\E[32;40mOutgoing: ");
	printf("\E[32;40m %s \E[0m ",response);

#ifdef DEBUG
	printf("[ ");
	for (int i=0;i<MAX_BUF;i++)
	  printf("\E[32;40m%02x ", response[i]);
	printf("]\E[0m\n");
#endif
	printf("\n");

	// Open up inj_rx and write response
	if ((atoi(argv[1])) == 1)
		pFile = fopen( RXFILE1, "w+");
	if ((atoi(argv[1])) == 2)
		pFile = fopen( RXFILE2, "w+");
  	fwrite(response, sizeof(char), strlen(response), pFile);
  	fclose(pFile);

	//Zero out tx/rx buffer strings
	memset(response,0,MAX_BUF);
	memset(buffer,0,MAX_BUF);
	close(pipe);
  }

  return 0;
}
