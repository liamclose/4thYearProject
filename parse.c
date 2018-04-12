#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  char *stringLocation;
  char *identifier1 = "10.";
  char *identifier2 = "/wasu_";
  char *identifier3 = "video/";
  char *identifier4 = "kTime=";
  char *identifier5 = " X)\" ";
  char *identifier6 = "/GX_";
  char *identifier7 = "/bjgh_";
  char *identifier8 = "/lutong_";
  char *identifier9 = "/WASU_";
  char *identifier10 = "/yifang";
  char *spacing = ", ";
  char ip[12],oldIp[12],filepath[7],file[23],epoch[14],holder[62];
  char filepath2[23], holder2[78];
  char filepath3[12], holder3[67];
  char filepath4[24], holder4[79];

  char output[1250];
  FILE *from, *to;

  // Read data Location
  from = fopen("new.log", "r");
  if (from == NULL){
    perror("new.log doesn't exist.");
    exit(1);
  }
  
  // Write Location
  to = fopen("cleanData.txt", "wb");
  if (to == NULL){
      perror("cleanData.txt doesn't exist.");
      exit(1);
  }
	holder[0] = 0; // Empty arrays like junk characters, this fixes
	int counter = 0;
// Parsing Section
while (fgets(output, sizeof(output), from)!=NULL){
	  if(counter==48000){
		   fclose(to);
           fclose(from);
           exit(0); 
	  }
	  // Parse the IP
	  stringLocation = strstr(output,identifier1);
	  strncpy(ip,stringLocation,12);
	  
	  printf("%i\n",counter);
	  // Parse the Filepath
	  if(strstr(output,identifier2)==NULL){
		  if(strstr(output,identifier6)!=NULL){
			  
		  	stringLocation = strstr(output,identifier6);
		  	strncpy(filepath2,stringLocation+4,23);
			
			// Parse the File
	        stringLocation = strstr(output,identifier3);
	        strncpy(file,stringLocation+6,23);

	        // Parse the Epoch
	        stringLocation = strstr(output,identifier4);
	        strncpy(epoch,stringLocation+6,14);
 
       	    // Formatting for Row
	        strcpy(holder2,ip);
	        strcat(holder2,spacing);
	        strcat(holder2,filepath2);
	        strcat(holder2,spacing);
	        strcat(holder2,file);
	        strcat(holder2,spacing);
	        strcat(holder2,epoch);
	        // Formatting for Row
	
	        // Write the array to output file
            fwrite(holder2, sizeof(char), sizeof(holder2), to);
	        fwrite("\n", sizeof(char), 1, to);
			printf("%s\n",holder2);
	        strncpy(oldIp,ip,12);
	        output[0] = 0;
		    counter++;
		  }
		  else if(strstr(output,identifier7)!=NULL||strstr(output,identifier8)!=NULL){
		   	if(strstr(output,identifier8)!=NULL){
				stringLocation = strstr(output,identifier8);
				strncpy(filepath3,stringLocation+8,12);
			}else{
		  	    stringLocation = strstr(output,identifier7);
		  	    strncpy(filepath3,stringLocation+6,12);
			}
			
			// Parse the File
	        stringLocation = strstr(output,identifier3);
	        strncpy(file,stringLocation+6,23);

	        // Parse the Epoch
	        stringLocation = strstr(output,identifier4);
	        strncpy(epoch,stringLocation+6,14);
 
       	  // Formatting for Row
	       strcpy(holder3,ip);
	       strcat(holder3,spacing);
	       strcat(holder3,filepath3);
	       strcat(holder3,spacing);
	       strcat(holder3,file);
	       strcat(holder3,spacing);
	       strcat(holder3,epoch);
	       // Formatting for Row
	
	       // Write the array to output file
           fwrite(holder3, sizeof(char), sizeof(holder3), to);
	       fwrite("\n", sizeof(char), 1, to);
		   printf("%s\n",holder3);
	       output[0] = 0;
		  }else if(strstr(output,identifier10)!=NULL){	
			stringLocation = strstr(output,identifier10);
		  	strncpy(filepath,stringLocation+7,7);
			
			// Parse the File
	        stringLocation = strstr(output,identifier3);
	        strncpy(file,stringLocation+6,23);

	        // Parse the Epoch
	        stringLocation = strstr(output,identifier4);
	        strncpy(epoch,stringLocation+6,14);
 
       	  // Formatting for Row
	       strcpy(holder,ip);
	       strcat(holder,spacing);
	       strcat(holder,filepath4);
	       strcat(holder,spacing);
	       strcat(holder,file);
	       strcat(holder,spacing);
	       strcat(holder,epoch);
	       // Formatting for Row
	
	       // Write the array to output file
           fwrite(holder, sizeof(char), sizeof(holder), to);
	       fwrite("\n", sizeof(char), 1, to);
		   printf("%s\n",holder);
	       output[0] = 0;
		   counter++;
		  }else{
		  	stringLocation = strstr(output,identifier9);
		  	strncpy(filepath4,stringLocation+6,24);
			printf("%s\n",filepath4);
			
			// Parse the File
	        stringLocation = strstr(output,identifier3);
	        strncpy(file,stringLocation+6,23);

	        // Parse the Epoch
	        stringLocation = strstr(output,identifier4);
	        strncpy(epoch,stringLocation+6,14);
 
       	  // Formatting for Row
	       strcpy(holder4,ip);
	       strcat(holder4,spacing);
	       strcat(holder4,filepath4);
	       strcat(holder4,spacing);
	       strcat(holder4,file);
	       strcat(holder4,spacing);
	       strcat(holder4,epoch);
	       // Formatting for Row
	
	       // Write the array to output file
           fwrite(holder4, sizeof(char), sizeof(holder4), to);
	       fwrite("\n", sizeof(char), 1, to);
		   printf("%s\n",holder4);
	       output[0] = 0;
		   counter++;
		  }
			  
	  }else{
	  stringLocation = strstr(output,identifier2);
	  strncpy(filepath,stringLocation+6,7);

	  // Parse the File
	  stringLocation = strstr(output,identifier3);
	  strncpy(file,stringLocation+6,23);

	  // Parse the Epoch
	  stringLocation = strstr(output,identifier4);
	  strncpy(epoch,stringLocation+6,14);

	  // Formatting for Row
	  strcpy(holder,ip);
	  strcat(holder,spacing);
	  strcat(holder,filepath);
	  strcat(holder,spacing);
	  strcat(holder,file);
	  strcat(holder,spacing);
	  strcat(holder,epoch);
	  // Formatting for Row
	
	  // Write the array to output file
      fwrite(holder, sizeof(char), sizeof(holder), to);
	  fwrite("\n", sizeof(char), 1, to);
	  printf("%s\n",holder);
	  output[0] = 0;
	  counter++;
	  }
  }
 fclose(to);
  fclose(from);
  exit(0); 
}
