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

unsigned short csum(unsigned short *buf, int nwords)
{
	  unsigned long sum;
	  for(sum=0; nwords>0; nwords--)
		sum += *buf++;
	  sum = (sum >> 16) + (sum &0xffff);
	  sum += (sum >> 16);
	  return (unsigned short)(~sum);
}

uint16_t udp_checksum(const struct iphdr  *ip,
        const struct udphdr *udp,
        const uint16_t *buf)
{
 
    int calculated_length = ntohs(udp->len)%2 == 0 ? ntohs(udp->len) : ntohs(udp->len) + 1;
 
    struct pseudo_hdr ps_hdr = {0};
    bzero (&ps_hdr, sizeof(struct pseudo_hdr));
    uint8_t data[sizeof(struct pseudo_hdr) + calculated_length];
    bzero (data, sizeof(struct pseudo_hdr) + calculated_length );
 
    ps_hdr.source = ip->saddr;
    ps_hdr.dest = ip->daddr;
    ps_hdr.protocol = IPPROTO_UDP; //17
    ps_hdr.udp_length = udp->len;
 
    memcpy(data, &ps_hdr, sizeof(struct pseudo_hdr));
    memcpy(data + sizeof(struct pseudo_hdr), buf, ntohs(udp->len) ); //the remaining bytes are set to 0
 
    return csum((uint16_t *)data, sizeof(data)/2);
}
	

int main() {
 
	char *datagram = calloc(1000, sizeof(char));	
	struct ether_header *mac_header = (struct ether_header*) datagram;
	struct iphdr *ip_header = (struct iphdr*) (datagram + sizeof(struct ether_header));
	struct udphdr *udp_header = (struct udphdr*) (datagram + sizeof(struct iphdr) + sizeof(struct ether_header));
	
	unsigned char mac_s[6]={0x08,0x00,0x27,0x9b,0x6d,0x6c};
	unsigned char mac_d[6]={0x08,0x00,0x27,0x59,0x76,0x08};
		
	memset(datagram, 1, 1000);
	
	char message[10];
	strncpy(message,"Hello!",sizeof(message)-1);
	printf("%s\n",message);
		
  	udp_header->source = htons(5000);
  	udp_header->dest = htons(5000);
  	udp_header->len = htons(1001 + strlen(message)+1);
  	udp_header->check = udp_checksum(ip_header,udp_header, (const uint16_t *)udp_header);

  	
  	ip_header->version = 4;
  	ip_header->ihl = 5;
  	ip_header->tos = 0;
  	ip_header->id = htons(11111);
  	ip_header->frag_off = htons(0x4000);
  	ip_header->ttl = 64;
  	ip_header->protocol = 17;
  	ip_header->saddr = inet_addr("192.168.56.5");
  	ip_header->daddr = inet_addr("192.168.56.3");
  	ip_header->tot_len = htons(1028);
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
  	
  	memmove((void *) (sock_str.sll_addr), (void *) mac_d, ETH_ALEN);
  	memmove(datagram + sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(struct ether_header),message,1000);
  	
  	
  	if (sendto(skt, datagram, 1042, 0, (struct sockaddr *)&sock_str, sizeof(struct sockaddr_ll)) < 0)
	{
		perror("sendto()");
		exit(3);
	}	
  	
  	
  	
  	close(skt);
}





