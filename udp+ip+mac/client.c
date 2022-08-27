#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
    
#define PORT     5000 
#define MAXLINE 1024 
    
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello!"; 
    struct sockaddr_in     servaddr, cliaddr; 
    
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
            

    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(5000); 
    servaddr.sin_addr.s_addr = inet_addr("192.168.56.3"); 
        
    int n, len = sizeof(cliaddr); 
    
    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
            
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                0, (struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Server : %s\n", buffer); 
    
    close(sockfd); 
    return 0; 
}
