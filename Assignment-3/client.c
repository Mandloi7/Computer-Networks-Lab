
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    if (argc != 3){
        fprintf(stderr, "%s\n", "Usage : ./client <server_ipaddress> <port_number>");
        exit(1);
        return 0;
    }
    char *server_ipaddr = argv[1];
    char *port = argv[2];

    struct sockaddr_in address;
    struct sockaddr_in serv_addr;
    int sock = 0, response;
    char *message;
    
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, server_ipaddr, &serv_addr.sin_addr) <= 0)        // Convert IPv4 and IPv6 addresses from text to binary form
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    printf("Please Give Input format as <TYPE_NUM> <IP/DOMAIN_NAME> \n");
    
    while (1){

        memset(buffer, '\0', 1024);
        scanf("%[^\n]%*c", message);                                       //scanning user input, Eg: 1 www.google.com
        int spacecount = 0, firstspace = 0;
        int ci = 0;
        while (message[ci] != '\0' && message[ci] != '\n')                 //verifying correctness of user input 
        {
            ci++;
        }
        for (int i = 0; i < ci; i++)
        {
            if (message[i] == ' ')
            {
                spacecount++;
                firstspace = i;
            }
        }
        if (spacecount != 1 || !(message[0] == '1' || message[0] == '2'))  //summarizing incorrect cases
        {
            printf("\nINVALID SYNTAX: Eg 1 google.com \n");
            continue;
        }
        for (int i = firstspace; i < ci - 1; i++)                          
        {
            message[i] = message[i + 1];
        }
        message[ci - 1] = '\0';
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)                  //Creating socket
        {
            printf("\n Socket creation error \n");
            return -1;
        }
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        {
            printf("\nConnection Failed \n");
            return -1;
        }

        send(sock, message, strlen(message), 0);      //sending the request message
        response = read(sock, buffer, 1024);           //reading the response message

        if (buffer[0] == '3')                         //entry exists in database
        {
            printf("RESPONSE TYPE : 3\t\t\n");        
            printf("RESPONSE MESSAGE : ENTRY EXIST IN DATABASE \n");
            if (message[0] == '1')
                printf("THE CORRESPONDING IP IS :   ");
            else
            {
                printf("THE CORRESPONDING DOMAIN IS : ");
            }
            ci = 1;
            while (buffer[ci] != '\0' && buffer[ci] != '\n'){
                printf("%c", buffer[ci]);
                ci++;
            }
            printf("\n");
        }
        else
        {
            printf("RESPONSE TYPE : 4\n");
            ci = 1;
            while (buffer[ci] != '\0' && buffer[ci] != '\n')
            {
                printf("%c", buffer[ci]);
                ci++;
            }
            printf("\n");
        }
        close(sock);
    }
    return 0;
}
