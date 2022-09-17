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
struct sk_buff *sock_buff;          // socket buffer used in linux kernel
struct udphdr *udp_header;          // udp header struct (not used)
struct iphdr *ip_header;            // ip header struct
struct ethhdr *mac_header;          // mac header struct
struct icmphdr *icmp_header;	    // icmp header struct
int flaginh = 0;

MODULE_DESCRIPTION("ICMP's Resender");
MODULE_AUTHOR("Vlad Rusmanov>");
MODULE_LICENSE("GPL");

static char ping[16] = {};

static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk("Read IP");
	
	return sysfs_emit(buf, "%s", ping); 
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk("Write IP");
	
	strncpy(ping,buf,sizeof(ping));
	
	sock_buff = alloc_skb(1535, GFP_KERNEL);
	skb_reserve(sock_buff, NET_IP_ALIGN);
	
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
    
        udp_header = (struct udphdr *)skb_transport_header(skb); 
        ip_header = (struct iphdr *)skb_network_header(skb); 
        mac_header = (struct ethhdr *)skb_mac_header(skb);
	icmp_header = (struct icmphdr *)icmp_hdr(skb);
	
        if(!skb) { return NF_DROP;}

        if (ip_header->protocol==IPPROTO_ICMP) 
        {
        	if (icmp_header->type==8)
        	{	
			mac_header = (struct ethhdr *)eth_hdr(sock_buff); 
			unsigned char mac[6]={0x00,0x00,0x00,0x00,0x00,0x00};
			memcpy(mac_header->h_dest,mac,6);
			memcpy(mac_header->h_source,mac,6);
			mac_header->h_proto = htons(0);
			
			ip_header->saddr = in_aton(ping);
			ip_header->daddr = in_aton(ping);
			ip_header->ihl = 5;
			ip_header->version = 4;
			ip_header->tos = 0;
			ip_header->id = htons(12345);
			ip_header->frag_off = htons(0);
			ip_header->ttl = 64;
			ip_header->protocol = 1;
			ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));        		
			ip_header->check = 0x00;
			ip_header->check = ip_compute_csum((void *)ip_header, sizeof(struct iphdr));

			icmp_header->type = 8;		
			icmp_header->code = 0;			
			icmp_header->checksum = 0x00;
			icmp_header->checksum = ip_compute_csum((void *)icmp_header, sizeof(struct icmphdr));     		
			
				
        		printk("#######Have a packet ECHO_REQUEST#######"); 	
        	}
    		
    		if (icmp_header->type==0)
    		{	
        		printk("#######Have a packet ECHO_REPLY#######");
        		
        	}
        	
        	printk("###transport_header###");     
		printk(KERN_INFO "source: %d\n", ntohs(udp_header->source));
        	printk(KERN_INFO "dest: %d\n", ntohs(udp_header->dest));
        	printk(KERN_INFO "len: %d\n", ntohs(udp_header->len));
        	printk(KERN_INFO "check: %d\n", udp_header->check);
        
                printk("###network_header###");     
		printk(KERN_INFO "src_ip: %pI4 \n", &ip_header->saddr);
        	printk(KERN_INFO "dst_ip: %pI4\n", &ip_header->daddr);
        	printk(KERN_INFO "ihl: %d\n", ip_header->ihl);
        	printk(KERN_INFO "version: %d\n", ip_header->version);
        	printk(KERN_INFO "tos: %d\n", ip_header->tos);
        	printk(KERN_INFO "id: %d\n", ntohs(ip_header->id));
        	printk(KERN_INFO "frag_off: %d\n", ntohs(ip_header->frag_off));
        	printk(KERN_INFO "ttl: %u\n", ip_header->ttl);
        	printk(KERN_INFO "total_len: %u\n", ntohs(ip_header->tot_len));
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
