#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h>	
#include <string.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>

#define PORT 	 8050 
#define MAXLINE 1024 

struct pseudo_hdr {
    u_int32_t source;
    u_int32_t dest;
    u_int8_t zero; 
    u_int8_t protocol;
    u_int16_t udp_length;
};

static unsigned short csum(unsigned short* addr, int len) 
{
	int nleft = len; 
	int sum = 0; 
	unsigned short* w = addr; 
	unsigned short answer = 0; 

	while(nleft > 1) { 
		sum += *w ++; 
		nleft -= 2; 
	} 

	if (nleft == 1) { 
		*(unsigned char*) (&answer) = *(unsigned char*) w;
		sum += answer;
	}
	
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;

	return answer;	
}
	

int main() {
 
	char *datagram = calloc(1, sizeof(char));	
	struct ether_header *mac_header = (struct ether_header*) datagram;
	struct iphdr *ip_header = (struct iphdr*) (datagram + sizeof(struct ether_header));
	struct udphdr *udp_header = (struct udphdr*) (datagram + sizeof(struct iphdr) + sizeof(struct ether_header));
	
	unsigned char mac_s[6]={0x08,0x00,0x27,0x9b,0x6d,0x6c};
	unsigned char mac_d[6]={0x08,0x00,0x27,0x59,0x76,0x08};
		
	memset(datagram, 1, 1);
	
	char message[10];
	strncpy(message,"Hello!",sizeof(message)+1);
	printf("%s\n",message);
		
  	udp_header->source = htons(55959);
  	udp_header->dest = htons(5000);
  	udp_header->len = htons(sizeof(struct udphdr) + strlen(message));
  	udp_header->check = 0;

  	
  	ip_header->version = 4;
  	ip_header->ihl = 5;
  	ip_header->tos = 0;
  	ip_header->id = htons(11111);
  	ip_header->frag_off = htons(0);
  	ip_header->ttl = 64;
  	ip_header->protocol = 17;
  	ip_header->saddr = inet_addr("192.168.56.5");
  	ip_header->daddr = inet_addr("192.168.56.3");
  	ip_header->tot_len = htons(28 + strlen(message));
  	ip_header->check = csum((unsigned short*)ip_header, ip_header->ihl);;
  	
  	mac_header->ether_type = htons(ETHERTYPE_IP);
  	memcpy(mac_header->ether_shost,mac_s,6);
  	memcpy(mac_header->ether_dhost,mac_d,6);
  	
	int skt = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (skt < 0) {
		perror("socket() error");
		exit(2);
	}
  	
  	struct sockaddr_ll sock_str;
  	memset(&sock_str, 0, sizeof(sock_str));
  	sock_str.sll_family = AF_PACKET;
  	sock_str.sll_ifindex = if_nametoindex("enp0s8");
  	sock_str.sll_halen = ETH_ALEN;
  	//sock_str.sll_protocol=htons(ETH_P_ARP);
  	
  	struct sockaddr_in     servaddr; 
  	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_addr.s_addr = inet_addr("192.168.56.3"); 
    
  	memmove((void *) (sock_str.sll_addr), (void *) mac_d, ETH_ALEN);
  	memmove(datagram + sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(struct ether_header),message,strlen(message));
  	
  	
  	if 
        (sendto(skt, datagram, 60, MSG_CONFIRM, (struct sockaddr *)&sock_str, sizeof(struct sockaddr_ll)) < 0)
  	//(sendto(skt, datagram, 60, 0, (struct sockaddr *) &servaddr, sizeof(struct sockaddr_in)) < 0)
  	 
	{
		perror("sendto()");
		exit(3);
	}
	
  	
  	//sleep(5);
  	
  	close(skt);
}





