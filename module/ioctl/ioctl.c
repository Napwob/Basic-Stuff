#include "chardev.h"
#include <fcntl.h>		
#include <unistd.h>		
#include <sys/ioctl.h>	
#include <stdio.h>
#include <stdlib.h>



int ioctl_set_msg(int file_desc, char *message)
{
	int ret_val;

	ret_val = ioctl(file_desc, IOCTL_SET_MSG, message);

	if (ret_val < 0) {
		printf("Error at calling ioctl_set_msg: %d\n", ret_val);
		exit(-1);
	}
	return 0;
}

int ioctl_get_msg(int file_desc)
{
	int ret_val;
	char message[100];

	ret_val = ioctl(file_desc, IOCTL_GET_MSG, message);

	if (ret_val < 0) {
		printf("Error at calling ioctl_get_msg: %d\n", ret_val);
		exit(-1);
	}

	printf("Message recieve (get_msg): %s\n", message);
	return 0;
}

int ioctl_get_nth_byte(int file_desc)
{
	int i;
	char c;

	printf("N bite in message (get_nth_byte): ");

	i = 0;
	while (c != 0) {
		c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);

		if (c < 0) {
			printf("Error ioctl_get_nth_byte на %d b.\n", i);
			exit(-1);
		}

		putchar(c);
	}
	putchar('\n');
	return 0;
}
	

int main()
{
	int file_desc;
	char *msg = "This is message\n";

	file_desc = open("/dev/chardev", 0);
	if (file_desc < 0) {
		printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(-1);
	}

	ioctl_set_msg(file_desc, msg);
	ioctl_get_msg(file_desc);
	ioctl_get_nth_byte(file_desc);

	close(file_desc);
}
	
