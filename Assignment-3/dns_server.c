
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 8080


const char* getfield(char* line, int num){   //function that returns the particular field from a given line of a csv file.
    const char* tok;
    tok=strtok(line,","); //split by ','
    if(num==2){   //ip_address requested
	int i=0;
	while(line[i]!='\n'){i++;}
	line[i]='\0';
	i=0;
	while(1){
		if(line[i]=='\0'){  //strtok modifies ',' to '\0', so return the next pointer for ip address.
			return line+i+1; 
		}
		i++;
	}
    }else{       //if the domain is requested
		return tok;
    }
    return NULL;
}



int main(int argc, char const *argv[]){

	struct sockaddr_in dns_address;
	struct sockaddr_in client_address;
	int opt = 1, dns_server_fd, cur_socket, message;

	//char buffer_in[1024] = {0}, buffer_out[1024] = {0};
	char buffer_in[1024];
	char buffer_out[1024];
	char ip_address[1024];
	char domain_name[1024];
	memset(&buffer_in, '\0', 1024);
	memset(&buffer_out, '\0', 1024);
	int found = 0;
	

	// Creating a socket file descriptor
	if ((dns_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("Socket Failure : ");
		exit(EXIT_FAILURE);
	}

	// Setting options for allowing the reuse of DNS Address, port
	if (setsockopt(dns_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
		perror("Socket Options Failure : ");
		exit(EXIT_FAILURE);
	}

	// Setting the DNS server IP Address and Port
	dns_address.sin_family = AF_INET;
	dns_address.sin_addr.s_addr = INADDR_ANY;
	dns_address.sin_port = htons(PORT);

	// Attaching socket to the address & port
	if (bind(dns_server_fd, (struct sockaddr *)&dns_address, sizeof(dns_address)) < 0){
		perror("Bind Failure : ");
		exit(EXIT_FAILURE);
	}

	// Listening on the created socket
	if (listen(dns_server_fd, 3) < 0){
		perror("Socket Listen Failure : ");
		exit(EXIT_FAILURE);
	}

	while (1){      // infinite loop for accepting requests always

		// Accepting a connection
		if ((cur_socket = accept(dns_server_fd, (struct sockaddr *)&client_address, (socklen_t *)&client_address)) < 0){
			perror("Socket Accept Failure : ");
			exit(EXIT_FAILURE);
		}

		// emptying the input and output buffers 
		memset(&buffer_in, '\0', 1024);
		memset(&buffer_out, '\0', 1024);

		// Reading the received message
		message = read(cur_socket, buffer_in, 1024);

		char decider=buffer_in[0]; //variable to decode the type of request

		printf("************************************************************************\n");
		if(decider!='\0')printf("REQUEST INFO :::: \tREQUEST_TYPE : %c,\tMESSAGE : %s\n", buffer_in[0], buffer_in + 1);
		
		

		//variable declarations for storing fields of csv file.
		char line[1024];
		char* domain;
		char* ipaddr;
		found = 0;
		FILE *database_fp = fopen("database.csv", "r");   //the database file pointer

		// Process each request depending on the type of request
		switch (decider){ 						

			//initial empty message for establishing connection.
			case '\0':
				printf("CONNECTION ESTABLISHED!\n");
				break;


			// Recieved message contains Domain name i.e. IP Address is requested.
			case '1':
				
				while (fgets(line, 1024, database_fp)){  // Search the database & send the response if found in the database
					char* tmp = strdup(line);
					domain=getfield(tmp, 1);
					ipaddr=getfield(tmp, 2);
					if (strcmp(domain, buffer_in + 1) == 0){
						buffer_out[0] = '3';
						strcat(buffer_out, ipaddr);
						send(cur_socket, buffer_out, strlen(buffer_out), 0);
						found = 1;
						break;
					}
					free(tmp);
				}
				fclose(database_fp);

				// If the domain doesn't exist in the database.
				if (found == 0){
					strcpy(buffer_out, "4 The domain doesn't exist in the database.");
					send(cur_socket, buffer_out, strlen(buffer_out), 0);
				}
				break;



			// Recieved message contains IP Address i.e. Domain name  is requested.
			case '2':

				while (fgets(line, 1024, database_fp)){ // Search the database & send the response if found in the database
					char* tmp = strdup(line);
					domain=getfield(tmp, 1);
					ipaddr=getfield(tmp, 2);
					if (strcmp(ipaddr, buffer_in + 1) == 0){
						buffer_out[0] = '3';
						strcat(buffer_out, domain);
						send(cur_socket, buffer_out, strlen(buffer_out), 0);
						found = 1;
						break;
					}
					free(tmp);
				}
				fclose(database_fp);

				// If the IP doesn't exist in the database.
				if (found == 0){
					strcpy(buffer_out, "4 The IP doesn't exist in the database.");
					send(cur_socket, buffer_out, strlen(buffer_out), 0);
				}
				break;

			default:
				// Incorrect format of the message.
				strcpy(buffer_out, "4 Incorrect Packet Format");
				send(cur_socket, buffer_out, strlen(buffer_out), 0);
				fprintf(stderr, "%s\n", "Incorrect Packet Format Recieved");
				break;
		}

		//printing the response 
		if(buffer_in[0]!='\0')printf("RESPONSE_INFO :::: \tRESPONSE_TYPE : %c,\tMESSAGE : %s\n", buffer_out[0], buffer_out + 1);
		printf("%s\n", "************************************************************************");
		close(cur_socket);
	}
	return 0;
}
