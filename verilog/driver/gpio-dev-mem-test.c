/*
 * This test application is to read/write data directly from/to the device 
 * from userspace. 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define IN 0
#define OUT 1

void usage(void)
{
	printf("*argv[0] -g <GPIO_ADDRESS> -i|-o <VALUE>\n");
	printf("    -g <GPIO_ADDR>   GPIO physical address\n");
	printf("    -i               Input from GPIO\n");
	printf("    -o <VALUE>       Output to GPIO\n");
	return;
}

void open_mem();
void map_mem(unsigned gpio_addr,unsigned page_size);
void rd_mem(int gpio_addr,int bits);
void wr_mem(int gpio_addr,char *optarg);
void wr(bool flag, int addr_offset);

int c;
int fd;
unsigned long long int value;
unsigned page_addr, page_offset;
void *ptr;
unsigned int buff[80];
int zero;
int bits;
int buff_bytes;
int print_bytes;
int write_length;
int write_times;
char mid_value[8];
int k;
bool flag=false;
unsigned page_size;

int main()
{
  page_size=sysconf(_SC_PAGESIZE);

  rd_mem(0xa0000000,32);
  wr_mem(0xa0000000,"00000103");
  rd_mem(0xa0000000,32);

  rd_mem(0xa0000004,256);
  wr_mem(0xa0000004,"00000000000000000000000000000000"
		    "000102030405060708090a0b0c0d0e0f");
  rd_mem(0xa0000004,256);

  rd_mem(0xa0000024,128);
  wr_mem(0xa0000024,"00000000000000000000000000000000");
  rd_mem(0xa0000024,128);

  rd_mem(0xa0000034,256);
  wr_mem(0xa0000034,"00112233445566778899aabbccddeeff"
		    "00112233445566778899aabbccddeeff");
  rd_mem(0xa0000034,256);

  rd_mem(0xa0000058,256);
  wr_mem(0xa0000078,"00000000");
  rd_mem(0xa0000000,32);

  munmap(ptr, page_size);

  return 0;
}

	/* mmap the device into memory */
	void map_mem(unsigned gpio_addr,unsigned page_size){
	page_addr = (gpio_addr & (~(page_size-1)));
	page_offset = gpio_addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);
	}

void rd_mem(int gpio_addr, int bits){
    int mod_addr = gpio_addr%8;
    if(mod_addr != 0){
       gpio_addr = gpio_addr - 4;
       bits = bits + 64;
	if(bits%8!=0) buff_bytes = bits/8+4;
   	 else buff_bytes = bits/8;
   	 print_bytes = buff_bytes/4;

    	fd = open ("/dev/mem", O_RDWR);

    	map_mem(gpio_addr,page_size);
		/* Read value from the device register */
		//value = *((unsigned *)(ptr + page_offset));
		//printf("gpio dev-mem test: input: %llX\n",value);
		memcpy(buff, ptr + page_offset, buff_bytes);
		int i;
		for(i = 1; i < print_bytes-1; i++){
			printf("%08X ", buff[i]);
		}
		printf("\n");

    } else{
    	if(bits%8!=0) buff_bytes = bits/8+4;
   	 else buff_bytes = bits/8;
   	 print_bytes = buff_bytes/4;

    	fd = open ("/dev/mem", O_RDWR);

    	map_mem(gpio_addr,page_size);
		/* Read value from the device register */
		//value = *((unsigned *)(ptr + page_offset));
		//printf("gpio dev-mem test: input: %llX\n",value);
		memcpy(buff, ptr + page_offset, buff_bytes);
		int i;
		for(i = 0; i < print_bytes; i++){
			printf("%08X ", buff[i]);
		}
		printf("\n");
    }	
}

void wr_mem(int gpio_addr, char *optarg){
    //printf("optarg = %s\n", optarg);
    fd = open ("/dev/mem", O_RDWR);
    map_mem(gpio_addr,page_size);

    write_length = strlen(optarg);
    //printf("write_length = %d\n",write_length);
    write_times = write_length/8;
    if(write_length%8!=0){
      zero = 8-(write_length%8);
      write_times++;

      for(k=0;k<write_times;k++){
        memcpy(mid_value,optarg,8);
        optarg=optarg+8;
        //printf("mid_value = %s\n",mid_value);
        value=strtoull(mid_value, NULL, 16);
        //printf("value = %llX\n", value);

        if(k==write_times-1) flag = true;

        wr(flag, k);

        flag = false;
      }
    }
    else{
      zero = 0;

      for(k=0;k<write_times;k++){
				memcpy(mid_value,optarg,8);
				optarg=optarg+8;
				//printf("mid_value = %s\n",mid_value);
				value=strtoull(mid_value, NULL, 16);
        //printf("value = %llX\n", value);
        wr(flag,k);
      }
    }
}
void wr(bool flag, int addr_offset){       
	/* Write value to the device register */
		if(flag == false) *((unsigned *)(ptr + page_offset+4*k)) = value;
		else{
		        value = value<<(zero*4);
			*((unsigned *)(ptr + page_offset+4*k)) = value;

		}
		//memset(ptr + page_offset+1, value, 1);
		//value=value>>8;
		//memset(ptr + page_offset, value, 1);
	}

