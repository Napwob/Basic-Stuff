#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h>	
#include <string.h>

#define PORT 	 8080 
#define MAXLINE 1024 
unsigned short csum(unsigned short *buf, int nwords)
{
	  unsigned long sum;
	  for(sum=0; nwords>0; nwords--)
		sum += *buf++;
	  sum = (sum >> 16) + (sum &0xffff);
	  sum += (sum >> 16);
	  return (unsigned short)(~sum);
}	

int main() {
 
	char *datagram  = calloc(1, sizeof(char));	
	struct iphdr *ip_header = (struct iphdr*) (datagram);
	struct udphdr *udp_header = (struct udphdr*) (datagram + sizeof(struct iphdr));
	//struct ether_header *mac_header = (struct ether_header*) datagram;;
	struct sockaddr_in sock_str;
	
	memset(datagram, 0, 1);
	
	char message[10];
	strncpy(message,"Hello!",sizeof(message)-1);
	printf("%s\n",message);
		
  	udp_header->source = htons(8050);
  	udp_header->dest = htons(8080);
  	udp_header->len = htons(sizeof(struct udphdr) + strlen(message)+1);
  	udp_header->check = 0;
  	
  	ip_header->version = 4;
  	ip_header->ihl = 5;
  	ip_header->tos = 0;
  	ip_header->id = htons(11111);
  	ip_header->frag_off = 0;
  	ip_header->ttl = 64;
  	ip_header->protocol = 17;
  	ip_header->saddr = inet_addr("127.0.0.1");
  	ip_header->daddr = inet_addr("127.0.0.1");
  	ip_header->tot_len = htons(5+strlen(message));
  	ip_header->check = csum((unsigned short*)ip_header, ip_header->ihl);;
  	
	int skt = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
	if (skt < 0) {
		perror("socket() error");
		exit(2);
	}
  	int one = 1;
  	const int *val = &one;
  	setsockopt(skt, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));
  	
  	sock_str.sin_family = AF_INET;
  	sock_str.sin_port = htons(8050);
  	sock_str.sin_addr.s_addr = inet_addr("127.0.0.1");
  	
  	memmove(datagram + sizeof(struct iphdr)+sizeof(struct udphdr),message,strlen(message)+1);
  	
  	if (sendto(skt, datagram, ip_header->tot_len, 0, (struct sockaddr *)&sock_str, sizeof(sock_str)) < 0)
	{
		perror("sendto()");
		exit(3);
	}	
  	
  	
  	
  	close(skt);
}





