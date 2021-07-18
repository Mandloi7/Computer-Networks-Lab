
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


struct cachenode {                          //Cache is implemented as a linked list with the following data in each node
    char ip_address[1024];
    char domain_name[1024];
    struct cachenode *next;
};

void resetbuffer(char *x){
    for (int i = 0; i < 1024; i++)
        x[i] = '\0';
}


int cachesize(struct cachenode *head){
    int size=0;
    while(head!=NULL){
        head=head->next;
        size++;
    }
    return size;
}




struct cachenode *cache_head;



/*Checks for data in cache*/
int cache_search(char *x, char *y)  /*Checks for data in cache*/
{
    int decider = x[0] - '0';
    char msg[1023];
    strcpy(msg, x + 1);

    struct cachenode *temp_head = cache_head;
    while (temp_head != NULL)
    {
        if (decider == 1 && strcmp(msg, temp_head->ip_address) == 0)
        {
            strcpy(y, "3");
            strcat(y + 1, temp_head->ip_address);
            return 1;
        }
        else if (decider == 2 && strcmp(msg, temp_head->domain_name) == 0)
        {
            
            strcpy(y, "3");
            strcat(y + 1, temp_head->domain_name);
            return 1;
        }
        temp_head = temp_head->next;
    }
    return 0;
}



int main(int argc, char const *argv[])
{
    // The paramter passed should in specific order.
    if (argc != 2)
    {
        fprintf(stderr, "%s\n", "Usage : ./proxy <port>");
        exit(1);
        return 0;
    }


    char *PORT = argv[1];
    cache_head = NULL;
    int server_fd, cur_socket, valread;
    struct sockaddr_in address;
    struct sockaddr_in dns_serv_addr;

    int opt = 1;
    int addrlen = sizeof(address);
    int dns_sock = 0;
    char buffer_in[1024];
    char buffer_out[1024];

    for (int i = 0; i < 1024; i++)
    {
        buffer_in[i] = '\0';
        buffer_out[i] = '\0';
    }
    /*connecting with dns*/

    if ((dns_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&dns_serv_addr, '0', sizeof(dns_serv_addr));

    dns_serv_addr.sin_family = AF_INET;
    dns_serv_addr.sin_port = htons(8080);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, "127.0.0.1", &dns_serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(dns_sock, (struct sockaddr *)&dns_serv_addr, sizeof(dns_serv_addr)) < 0)
    {
        printf("\nConnection with DNS Failed \n");
        return -1;
    }

    close(dns_sock);

    /*-------------------------------------*/

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    memset(&address, '0', sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(PORT));

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (1){
        if ((cur_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t *)&addrlen)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        valread = read(cur_socket, buffer_in, 1024);
        char decider = buffer_in[0]; //variable to decode the type of request
        printf("************************************************************************\n");
        printf("REQUEST :\t TYPE-%c||\tMESSAGE-%s\n", buffer_in[0], buffer_in + 1);

        // switching on the basis of type 1/2 in input buffer.
        switch (decider){
            case '1':
                printf("REQUEST MESSAGE : %c %s\n", buffer_in[0],buffer_in+1);
                if (cache_search(buffer_in, buffer_out))
                {
                    /*If the required entry is found in the cache*/
                    resetbuffer(buffer_in);
                    send(cur_socket, buffer_out, strlen(buffer_out), 0);
                    resetbuffer(buffer_out);
                    close(cur_socket);
                    printf("ENTRY FOUND IN CACHE\n");
                }
                else
                {
                    /*If required entry not found in cache*/
                    struct cachenode *temp;
                    temp = (struct cachenode *)malloc(sizeof(struct cachenode));
                    /*resetting domain_name and ip_adress*/
                    for (int i = 0; i < 1024; i++)
                    {
                        temp->domain_name[i] = '\0';
                        temp->ip_address[i] = '\0';
                    }

                    /*as it is an type 1 request so domain is known(buffer_in)*/
                    for (int i = 1; i < strlen(buffer_in); i++)
                        temp->domain_name[i - 1] = buffer_in[i];
                    if ((dns_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        printf("\n SOCKET UNABLE TO CREATE \n");
                        return -1;
                    }
                    if (connect(dns_sock, (struct sockaddr *)&dns_serv_addr, sizeof(dns_serv_addr)) < 0)
                    {
                        printf("\nCONNECTION WITH DNS FAILED\n");
                        return -1;
                    }
                    /*Communicating with DNS*/
                    send(dns_sock, buffer_in, strlen(buffer_in), 0);
                    resetbuffer(buffer_in);
                    valread = read(dns_sock, buffer_in, 1024);
                    close(dns_sock);
                    
                    if (buffer_in[0] == '3')
                    {
                        /*If Entry is found in database*/
                        for (int i = 1; i < strlen(buffer_in); i++)
                            temp->ip_address[i - 1] = buffer_in[i];
                        int size = 0;
                        temp->next = cache_head;
                        cache_head = temp;

                        struct cachenode *current = cache_head;
                        size=cachesize(current);
                        if (size >= 2)
                        {
                            /*We use FIFO to clear cache*/
                            free(current);
                            current = cache_head;
                            current->next->next->next = NULL;
                        }
                        printf("ENTRY NOT FOUND IN CACHE\n");
                        printf("CACHE UPDATED\n");
                        printf("RESPONSE MESSAGE : 3 %s\n",buffer_in+1);
                    }
                    else{
                         /*If Entry is not found in database*/
                        printf("ENTRY NOT FOUND IN DATABASE OR SOME ERROR OCCURED\n");
                        free(temp);
                    }
                }
                break;
            case '2':
                printf("REQUEST MESSAGE : %c %s\n", buffer_in[0],buffer_in+1);
                if (cache_search(buffer_in, buffer_out))
                {
                    /*If the required entry is found in the cache*/
                    resetbuffer(buffer_in);
                    send(cur_socket, buffer_out, strlen(buffer_out), 0);
                    resetbuffer(buffer_out);
                    close(cur_socket);
                    printf("ENTRY FOUND IN CACHE\n");
                    printf("RESPONSE MESSAGE : %c %s", buffer_in[0],buffer_in+1);
                }
                else
                {   /*If required entry not found in cache*/
                    int i;
                    struct cachenode *temp;
                    temp = (struct cachenode *)malloc(sizeof(struct cachenode));

                    /*resetting domain_name and ip_adress*/
                    for (i = 0; i < 1024; i++)
                    {
                        temp->domain_name[i] = '\0';
                        temp->ip_address[i] = '\0';
                    }

                    /*as it is an type 2 request so IP is known(buffer_in)*/
                    for (i = 1; i < strlen(buffer_in); i++)
                        temp->ip_address[i - 1] = buffer_in[i];

                    if ((dns_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        printf("\n SOCKET UNABLE TO CREATE \n");
                        return -1;
                    }

                    if (connect(dns_sock, (struct sockaddr *)&dns_serv_addr, sizeof(dns_serv_addr)) < 0)
                    {
                        printf("\nCONNECTION WITH DNS FAILED\n");
                        return -1;
                    }

                    /*Communicating with DNS*/
                    send(dns_sock, buffer_in, strlen(buffer_in), 0);
                    resetbuffer(buffer_in);
                    valread = read(dns_sock, buffer_in, 1024);
                    close(dns_sock);
                    if (buffer_in[0] == '3')
                    {
                        /*If Entry is found in database*/
                        for (i = 1; i < strlen(buffer_in); i++)
                            temp->domain_name[i - 1] = buffer_in[i];
                        int size = 0;
                        temp->next = cache_head;
                        cache_head = temp;

                        struct cachenode *current = cache_head;
                        size=cachesize(current);
                        if (size >= 2)       
                        {
                            /*We use FIFO to clear cache*/
                            free(current);
                            current = cache_head;
                            current->next->next->next = NULL;
                        }
                        printf("ENTRY NOT FOUND IN CACHE\n");
                        printf("CACHE UPDATED\n");
                        printf("RESPONSE MESSAGE : 3 %s\n",buffer_in+1);
                        
                    }
                    else{
                        /*If Entry is not found in database*/
                        printf("ENTRY NOT FOUND IN DATABASE OR SOME ERROR OCCURED\n");
                        free(temp);
                    }
                        
                }
                break;
            default:
            /*The buffer_in has a type other then type 1/2*/
                printf("ERROR OCCURRED\n");
                resetbuffer(buffer_in);
                strcpy(buffer_in, "4 ErrorMessage");
                send(cur_socket, buffer_in, strlen(buffer_in), 0);
                close(cur_socket);
                resetbuffer(buffer_in);
                break;
        }
    }
    return 0;
}
