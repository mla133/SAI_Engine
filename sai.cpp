/* SAI Engine - rev 0.91 by Matt Allen */
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
#define RXFILE 		"/var/tmp/a4m/socat_smart_injector_input_data_file"
#define PIPE_FIFO   	"/var/tmp/a4m/socat_output_smart_injector_fifo"
#define logger { printf("%s:%d\n",__FILE__,__LINE__);}
#define MAX_BUF 1024
#define MAX_BUF_LEN 17

#define DEBUG 0
int main()
{
  char buffer[MAX_BUF];
  char response[80];
  char tempBuf[40];
  char address_buf[3];

  FILE * pFile;
  long lSize;
  size_t result;
  double NRT = 0.0;
  double NRT2 = 0.0;
  double NRT3 = 0.0;
  int pipe;
  int buf_size;
  int inj_address;

  printf("STARTING UP SAI BLACK BOX\n");
  while(1)
  {
	//Read in inj_tx to buffer
	if ( (pipe = open( PIPE_FIFO , O_RDONLY)) < 0) logger;
	if ( (buf_size = read(pipe, buffer, MAX_BUF)) < 0) logger;
/*	if ( (buf_size = read(pipe, buffer, MAX_BUF)) < 3) 
	{
	  logger; // We're missing something...like the injector address...
	  memset(buffer, 0, MAX_BUF);  //clear out buffer
	  close(pipe);
	  continue;
	}
*/
	buffer[buf_size] = 0; //null terminate string

	// Pull out address to strncat into response later
	memcpy(address_buf, buffer, 3);
	address_buf[3] = 0;
	inj_address = atoi(address_buf);


#ifdef DEBUG
	printf("\E[31;40m***********************\E[0m\n");
	printf("\E[31;40mIncoming: \E[0m");
	printf(" %s -->  ",buffer);
#endif

	// Parse buffer, supply response buffer

	//TITAN INJECTOR
	if( strncmp ("BI 001", buffer+3, 6) == 0 )
	{
	    NRT +=0.02; // increment the NRT for additive totals
	    strcpy(response, address_buf);
	    strcat(response, "OK");
	    printf("\E[33;40mTITAN(%d): II\E[0m\n", inj_address);
	}
	else if ( strncmp ("AS 001 0010.0",buffer+3, 13) == 0 ) 
	{
	   strcpy(response, address_buf);
	   strcat(response, "OK");
	   printf("\E[33;40mTITAN(%d): SET_VOL_PER_CYCLE\E[0m\n", inj_address);
	}
	else if ( strncmp ("at 001", buffer+3, 6) == 0)
	{
	  strcpy(response, address_buf);
	  sprintf(tempBuf, "at 001 %10.1f", NRT);
	  strcat(response, tempBuf);
	  printf("\E[33;40mTITAN(%d): POLL_TOTALS\E[0m\n", inj_address);
	}
	else if ( strncmp ("ls 001", buffer+3, 6) == 0)
	{
	  strcpy(response, address_buf);
	  sprintf(tempBuf, "ls 001 %09.4f 0000", NRT);
	  strcat(response, tempBuf);
	  printf("\E[33;40mTITAN(%d): POLL_TOTALS_AND_ALARMS\E[0m\n", inj_address);
	}	
	else if ( strncmp ("ac 001", buffer+3, 6) == 0)
	{
	  strcpy(response, address_buf);
	  strcat(response,"ac 001 0000");
	  printf("\E[33;40mTITAN(%d): POLL_ALARMS\E[0m\n", inj_address);
	}
	else if ( strncmp ("PS 001 0010.0", buffer+3, 13) == 0)
	{
	  strcpy(response, address_buf);
	  strcat(response,"OK");
	  printf("\E[33;40mTITAN(%d): SET_VOL_PER_INJ\E[0m\n", inj_address);
	}
	//BLENDPAK/MINIPAK
	else if ( strncmp ("EX 050", buffer+3, 6) == 0)
	{
	  if(inj_address == 72) NRT2 +=10.0; // increment the NRT for additive totals
	  if(inj_address == 83) NRT3 +=20.0; // increment the NRT for additive totals
	  strcpy(response, address_buf);
	  strcat(response,"OK");
	  if(atoi(address_buf) == 72)
	    printf("\E[35;40mBLENDPAK(%d): II\E[0m\n", inj_address);
	  else
	    printf("\E[36;40mMINIPAK(%d): II\E[0m\n", inj_address);
	}
	else if ( (strncmp("EX", buffer+3, 2) == 0) || (strncmp("WV", buffer+3, 2) == 0) )
	{
	  strcpy(response, address_buf);
	  strcat(response,"OK");
	  if(inj_address  == 72)
	    printf("\E[35;40mBLENDPAK(%d): POLLING/SETTING VOLUME/RATES\E[0m\n", inj_address);
	  else
	    printf("\E[36;40mMINIPAK(%d): POLLING/SETTING VOLUME/RATES\E[0m\n", inj_address);
	}
	else if ( strncmp("RV 802", buffer+3, 6) == 0 )
	{
	  strcpy(response, address_buf);
	  strcat(response, "RV 802 0000");
	  if(atoi(address_buf) == 72)
	    printf("\E[35;40mBLENDPAK(%d): POLL ALARMS\E[0m\n", inj_address);
	  else
	    printf("\E[36;40mMINIPAK(%d): POLL ALARMS\E[0m\n", inj_address);
	}
	else if ( (strncmp("RV 860", buffer+3, 6) == 0) || ( (strncmp("RV 850", buffer+3, 6) == 0) ) )
	{
	  strcpy(response, address_buf);
	  sprintf(tempBuf, "RV 870 %09.3f", NRT);
	  strcat(response, tempBuf);
	  printf("\E[35;40mBLENDPAK(%d): POLL TOTALS\E[0m\n", inj_address);
	}
	else if ( (strncmp("RV 850", buffer+3, 6) == 0) || ( (strncmp("RV 850", buffer+3, 6) == 0) ) )
	{
	  strcpy(response, address_buf);
	  sprintf(tempBuf, "RV 870 %09.3f", NRT);
	  strcat(response, tempBuf);
	  printf("\E[36;40mMINIPAK(%d): POLL TOTALS\E[0m\n", inj_address);
	}
	else if ( strncmp ("", buffer, MAX_BUF_LEN) == 0)
	{
	  memset(response,0,80);
	  logger;
	  continue;
	}
	else
	{
	  strcpy(response, address_buf);
	  strcat(response,"NO00");
	  printf("\E[33;40m[ERROR]\E[0m\n");
	}

#ifdef DEBUG
	printf("\E[32;40mOutgoing: \E[0m");
	printf(" %s\n",response);
	printf("\E[32;40m***********************\E[0m\n\n");
#endif

	// Open up inj_rx and write response
	pFile = fopen( RXFILE, "w+");
  	fwrite(response, sizeof(char), strlen(response), pFile);
  	fclose(pFile);

	//Zero out tx/rx buffer strings
	memset(response,0,MAX_BUF);
	memset(buffer,0,MAX_BUF);
	close(pipe);
  }

  return 0;
}
