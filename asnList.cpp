#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <strings.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define IP_LEN 16
#define IP_NO 30
#define CMD 50

using namespace std;

bool isValidIP(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}


int main() {

	int i ;
	int j ;
	char ascmd[CMD];
	char file_buf[(IP_LEN * IP_NO) + 16 ];
	int asn[10][30] = {{0}}; //can store 10 paths with 30 IPs(max hop count) each
	int routes = 0, current_route;
	int asnhops = 1000, asnhopsindex = 0;
	double percent = 0;
	FILE* fd2;

	char tr_ip[][30][16] = {  // 30 is max hop count ; 16 is max IPv4 string length
			{"174.22.45.32" ,"176.2.34.126", "55.244.108.20", "178.244.30.13"},
			{"178.22.45.32" ,"175.2.34.126", "86.244.108.20", "44.244.30.13"},
			{"130.22.45.32" ,"181.2.34.126", "11.214.104.20", "250.244.30.13"}
			};  //initialize for testing purpose

	int hops[10] = {4,4,4};

	routes = sizeof(tr_ip)/(30*16);	
	system("mv file_ip file_ip_old");
	system("mv file_as file_as_old");
	system("mv file_as_final file_as_final_old");
	fd2 = fopen("file_as_final",  "a");

	printf("Calculating ASNs\n");
	for(current_route = 0;current_route < routes;current_route++) {

	memset(file_buf, '0', sizeof(file_buf) ); 
	strcpy(file_buf, "begin\n");
	for(i=0;i< hops[current_route];i++) {
		if(isValidIP(tr_ip[current_route][i])) {
		strcat(file_buf, tr_ip[current_route][i]);
		strcat(file_buf, "\n");
		continue;
		}
		break;
	}
	strcat(file_buf,"end\n");
	int fd;
	fd = open("file_ip",O_CREAT | O_RDWR, S_IRWXU);
	write(fd, file_buf, strlen(file_buf));
	system("netcat whois.cymru.com 43 < file_ip > file_as");
	close(fd);	

	memset(file_buf, '0', sizeof(file_buf) ); // to be used as line buffer

	FILE* fd1;
	char temp[10];
	strcpy(temp, "ASN list for Route ");
	fputs(temp, fd2);
	fputc(current_route+1+48, fd2);
	strcpy(temp, "\n");
	fputs(temp, fd2);
	
	fd1 = fopen("file_as", 	"r+");
	j=0;
	while( fgets(file_buf, sizeof(file_buf), fd1) != NULL ) {
		if(j >= 2) {                      // start checking for immediate duplicate ASNs
			if((asn[current_route][j-1] == atoi(file_buf))|| (atoi(file_buf) == 0) ) {
				continue;
			}	
		}
		asn[current_route][j] = atoi(file_buf);
		fputs(file_buf, fd2);
		j++;
	}
	percent = ((double(current_route+1) )/ double(routes)) *100;
	printf("%lf complete\n",percent);
	
		
	if(j-1 < asnhops) {
		asnhops = j-1;
		asnhopsindex = current_route;
	}
	fputs(temp, fd2);
	fclose(fd1);

	}	
	printf("The optimum route is %d \n", current_route);
	printf("Please open file_as_final to find the ASNs for all routes\n");
	fclose(fd2);

	return asnhopsindex;
}
