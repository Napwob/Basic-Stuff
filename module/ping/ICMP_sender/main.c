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


static struct nf_hook_ops nfho;     // net filter hook option struct    
struct udphdr *udp_header;          // udp header struct (not used)
struct iphdr *ip_header;            // ip header struct
struct ethhdr *mac_header;          // mac header struct
struct icmphdr *icmp_header;	    // icmp header struct
int flaginh = 0;

MODULE_DESCRIPTION("ICMP's Sender");
MODULE_AUTHOR("Vlad Rusmanov>");
MODULE_LICENSE("GPL");

static char ping[16] = {};

static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk("Read IP");
	
	return sysfs_emit(buf, "%s", ping); 
}

void build_and_send(void)
{
	struct sk_buff *sock_buff;
	struct iphdr *ip_header_m;            
	struct ethhdr *mac_header_m;          
	struct icmphdr *icmp_header_m;
	
	int len_skb = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct icmphdr);
	
	sock_buff = alloc_skb(len_skb, GFP_KERNEL);
	skb_reserve(sock_buff, len_skb); 
	skb_push(sock_buff, sizeof(struct icmphdr) + sizeof(struct iphdr));
                	
	skb_set_mac_header(sock_buff, 0);
	mac_header_m = (struct ethhdr *)eth_hdr(sock_buff); 
	unsigned char mac[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	memcpy(mac_header_m->h_dest,mac,6);
	memcpy(mac_header_m->h_source,mac,6);
	mac_header_m->h_proto = htons(0);
        
        skb_set_network_header(sock_buff, sizeof(struct ethhdr));		
	ip_header_m = (struct iphdr *)ip_hdr(sock_buff);
	ip_header_m->saddr = in_aton("127.0.0.1");
	ip_header_m->daddr = in_aton("127.0.0.1");
	ip_header_m->ihl = 5;
	ip_header_m->version = 4;
	ip_header_m->tos = 0;
	ip_header_m->id = htons(12345);
	ip_header_m->frag_off = htons(0);
	ip_header_m->ttl = 64;
	ip_header_m->protocol = 1;
	ip_header_m->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));        		
	ip_header_m->check = 0x00;
	ip_header_m->check = ip_compute_csum((void *)ip_header, sizeof(struct iphdr));
	
	skb_set_transport_header(sock_buff, sizeof(struct ethhdr) + sizeof(struct iphdr));		
	icmp_header_m = (struct icmphdr *)icmp_hdr(sock_buff);
	icmp_header_m->type = 8;		
	icmp_header_m->code = 0;			
	icmp_header_m->checksum = 0x00;
	icmp_header_m->checksum = ip_compute_csum((void *)icmp_header, sizeof(struct icmphdr));
	
	
	printk("#######DEBUG INFORMATION#######"); 
        if (ip_header->protocol==IPPROTO_ICMP) 
        {
        	if (icmp_header->type==8)
        	{			
               		printk("#######Have a packet ECHO_REQUEST#######"); 	
        	}
    		
    		if (icmp_header->type==0)
    		{	
        		printk("#######Have a packet ECHO_REPLY#######");
        		
        	}
        
                printk("###network_header###");     
		printk(KERN_INFO "src_ip: %pI4 \n", &ip_header_m->saddr);
        	printk(KERN_INFO "dst_ip: %pI4\n", &ip_header_m->daddr);
        	printk(KERN_INFO "ihl: %d\n", ip_header_m->ihl);
        	printk(KERN_INFO "version: %d\n", ip_header_m->version);
        	printk(KERN_INFO "tos: %d\n", ip_header_m->tos);
        	printk(KERN_INFO "id: %d\n", ntohs(ip_header_m->id));
        	printk(KERN_INFO "frag_off: %d\n", ntohs(ip_header_m->frag_off));
        	printk(KERN_INFO "ttl: %u\n", ip_header_m->ttl);
        	printk(KERN_INFO "protocol: %u\n", ip_header_m->protocol);
        	printk(KERN_INFO "check: %d\n", ip_header_m->check);        	
        	
        	printk("##mac_header##");
        	printk(KERN_INFO "h_dest: %pM \n", &mac_header_m->h_dest);
        	printk(KERN_INFO "h_source: %pM \n", &mac_header_m->h_source);
        	printk(KERN_INFO "h_proto: %d \n", ntohs(mac_header_m->h_proto));
        	
        	printk("##icmp_header##");
        	printk(KERN_INFO "type: %u \n", icmp_header_m->type);
        	printk(KERN_INFO "code: %u \n", icmp_header_m->code);
        	printk(KERN_INFO "checksum: %d \n", icmp_header_m->checksum);
               	printk("#######End of a packet#######\n\n");
        }
        
        //ip_local_out(&init_net, NULL, sock_buff);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk("Write IP");
	
	strncpy(ping,buf,sizeof(ping));
	
	
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
        		build_and_send();		
               		printk("#######Have a packet ECHO_REQUEST#######");	
        	}
    		
    		if (icmp_header->type==0)
    		{	
        		printk("#######Have a packet ECHO_REPLY#######");
        		
        	}
        
                printk("###network_header###");     
		printk(KERN_INFO "src_ip: %pI4 \n", &ip_header->saddr);
        	printk(KERN_INFO "dst_ip: %pI4\n", &ip_header->daddr);
        	printk(KERN_INFO "ihl: %d\n", ip_header->ihl);
        	printk(KERN_INFO "version: %d\n", ip_header->version);
        	printk(KERN_INFO "tos: %d\n", ip_header->tos);
        	printk(KERN_INFO "id: %d\n", ntohs(ip_header->id));
        	printk(KERN_INFO "frag_off: %d\n", ntohs(ip_header->frag_off));
        	printk(KERN_INFO "ttl: %u\n", ip_header->ttl);
        	printk(KERN_INFO "protocol: %u\n", ip_header->protocol);
        	printk(KERN_INFO "check: %d\n", ip_header->check);        	
        	
        	printk("##mac_header##");
        	printk(KERN_INFO "h_dest: %pM \n", &mac_header->h_dest);
        	printk(KERN_INFO "h_source: %pM \n", &mac_header->h_source);
        	printk(KERN_INFO "h_proto: %d \n", ntohs(mac_header->h_proto));

        	
        	printk("##icmp_header##");
        	printk(KERN_INFO "type: %u \n", icmp_header->type);
        	printk(KERN_INFO "code: %u \n", icmp_header->code);
        	printk(KERN_INFO "checksum: %d \n", icmp_header->checksum);
               	printk("#######End of a packet#######\n\n");
        }
       
        return NF_ACCEPT;
}



int init_module()
{	
	int retval;
	printk("LOAD KERNEL MODULE");
	example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(example_kobj, &attr_group);
	if (retval)
		kobject_put(example_kobj);
	printk("LOADING COMPLETE\n");

        nfho.hook = hook_func;
        nfho.hooknum = 4; 
        nfho.pf = PF_INET;
        nfho.priority = NF_IP_PRI_FIRST;
        nf_register_net_hook(&init_net, &nfho); 
        printk(KERN_INFO "---------------------------------------\n\n");
	
		
        return 0;
}
 
void cleanup_module()
{
	printk(KERN_INFO "---------------------------------------\n");
        nf_unregister_net_hook(&init_net, &nfho); 
        kobject_put(example_kobj);
	printk("KERNEL MODULE UNLOADED");    
}

