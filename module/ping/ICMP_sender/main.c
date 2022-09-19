#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/icmp.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/delay.h>

static struct nf_hook_ops nfho;     // net filter hook option struct    
struct udphdr *udp_header;          // udp header struct (not used)
struct iphdr *ip_header;            // ip header struct
struct ethhdr *mac_header;          // mac header struct
struct icmphdr *icmp_header;	    // icmp header struct
int missed_packets = 0;

MODULE_DESCRIPTION("ICMP's Sender");
MODULE_AUTHOR("Vlad Rusmanov>");
MODULE_LICENSE("GPL");

static char ping[16] = {};

static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char message_to_show[100];
	
	//printk("Read IP");
	
	sprintf(message_to_show, "Ping result: %d missed packet from %s",missed_packets,ping);
	
	return sysfs_emit(buf, "%s", message_to_show); 
}


void build_and_send(void)
{
	static int counter = 1234;
	struct sk_buff *sock_buff = NULL;
	struct iphdr *ip_header_m = NULL;            
	//struct ethhdr *mac_header_m = NULL;          
	struct icmphdr *icmp_header_m = NULL;
	struct net_device *net_dev = NULL;
	struct flowi4 fl4;
	struct rtable *route_table = NULL;
	//unsigned char mac_d[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	//unsigned char mac_s[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	
	int len_skb = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct icmphdr);
	
	sock_buff = alloc_skb(len_skb, GFP_ATOMIC);
	
	skb_reserve(sock_buff, len_skb); 
	skb_push(sock_buff, sizeof(struct icmphdr));
	skb_push(sock_buff, sizeof(struct iphdr));
	skb_push(sock_buff, sizeof(struct ethhdr));
                	
	skb_set_mac_header(sock_buff, 0);       
        skb_set_network_header(sock_buff, 0);//sizeof(struct ethhdr));	
        skb_set_transport_header(sock_buff, sizeof(struct iphdr));// + sizeof(struct ethhdr));	
        
        /*mac_header_m = eth_hdr(sock_buff); 
        memcpy(mac_header_m->h_dest,mac_d,6);
        memcpy(mac_header_m->h_source,mac_s,6);
        mac_header_m->h_proto = htons(0);*/
        
	ip_header_m = ip_hdr(sock_buff);
	ip_header_m->saddr = in_aton(ping);
	ip_header_m->daddr = in_aton(ping);
	ip_header_m->ihl = sizeof(struct iphdr)/4;
	ip_header_m->version = 4;
	ip_header_m->tos = 0;
	ip_header_m->id = htons(counter);
	counter++;
	ip_header_m->frag_off = 0;
	ip_header_m->ttl = 255;
	ip_header_m->protocol = IPPROTO_ICMP;
	ip_header_m->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);// + sizeof(struct ethhdr);    
	    		
	ip_header_m->check = 0x00;
	ip_header_m->check = ip_compute_csum((void *)ip_header_m, sizeof(struct iphdr));
			
	icmp_header_m = icmp_hdr(sock_buff);
	icmp_header_m->type = ICMP_ECHO;		
	icmp_header_m->code = 0;
	icmp_header_m->un.echo.sequence = 0;
	icmp_header_m->un.echo.id = 0;	
		
	icmp_header_m->checksum = 0x00;
	icmp_header_m->checksum = ip_compute_csum(icmp_header_m, sizeof(struct icmphdr));
	
	net_dev = dev_get_by_name(&init_net, "usb0");
	sock_buff->dev = net_dev;
			
	fl4.flowi4_oif = net_dev->ifindex;
	fl4.daddr = in_aton(ping);
	fl4.saddr = in_aton(ping);
	
	route_table = ip_route_output_key(&init_net, &fl4);

	skb_dst_set(sock_buff, &route_table->dst);

	ip_local_out(&init_net, NULL, sock_buff);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int iterator;
	//printk("Write IP");
	
	strncpy(ping,buf,sizeof(ping));
	if(buf != NULL)
	{
		for(iterator=0;iterator<25;iterator++)
		{
			build_and_send();
		}
	}
	
	return count;
}

static struct kobj_attribute ping_attribute =
	__ATTR(ping, 0664, b_show, b_store);


static struct attribute *attrs[] = {
	&ping_attribute.attr,
	NULL,	
};


static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj;

unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{  	
        if(!skb) { return NF_DROP;}
	
	udp_header = (struct udphdr *)skb_transport_header(skb); 
        ip_header = (struct iphdr *)skb_network_header(skb); 
        mac_header = (struct ethhdr *)skb_mac_header(skb);
	icmp_header = (struct icmphdr *)icmp_hdr(skb);
	
        if (ip_header->protocol==IPPROTO_ICMP) 
        {
        	if (icmp_header->type==8)
        	{	
        		missed_packets++;		
               		//printk("#######Have a packet ECHO_REQUEST#######");	
        	}
    		
    		if (icmp_header->type==0)
    		{	
    			missed_packets--;
        		//printk("#######Have a packet ECHO_REPLY#######");
        		
        	}     
        }
       
        return NF_ACCEPT;
}



int init_module()
{	
	int retval;
	//printk("LOAD KERNEL MODULE");
	example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(example_kobj, &attr_group);
	if (retval)
		kobject_put(example_kobj);
	//printk("LOADING COMPLETE\n");

        nfho.hook = hook_func;
        nfho.hooknum = 4; 
        nfho.pf = PF_INET;
        nfho.priority = NF_IP_PRI_FIRST;
        nf_register_net_hook(&init_net, &nfho); 
        //printk(KERN_INFO "---------------------------------------\n\n");
		
        return 0;
}
 
void cleanup_module()
{
	//printk(KERN_INFO "---------------------------------------\n");
        nf_unregister_net_hook(&init_net, &nfho); 
        kobject_put(example_kobj);
	//printk("KERNEL MODULE UNLOADED");    
}

