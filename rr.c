#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

#define DEFAULTBLOCKSIZE  32768
#define DEFAULTBLOCKS      1024
#define DEFAULTNUM 1000

unsigned long get_us(struct timeval start, struct timeval end)
{
	unsigned long ns = 0;
	if (end.tv_usec < start.tv_usec) {
		end.tv_usec += 1000000;
		end.tv_sec--;
	}
    ns = (end.tv_sec - start.tv_sec) * 1000000 +
                 (end.tv_usec - start.tv_usec);
    return ns;
}
char *usagestr = 
 "Usage: rr [OPTION]...\n"
 " -i             inputdevice that will be tested\n"
 " -b BLOCKSIZE   read BLOCKSIZE bytes at a time\n"
 " -c BLOCKS      read only BLOCKS blocks\n"
 " -n NUM         iterate NUM times before next read\n";

void usage()
{
	fprintf(stderr, "%s", usagestr);
	exit(1);
}

int main(int argc, char **argv){
	struct timeval sstamp, estamp;
	unsigned long readtime = 0;
	char *buf = NULL, *input = NULL;
	unsigned long blocksize = DEFAULTBLOCKSIZE, blocks = DEFAULTBLOCKS, i = 0;
	unsigned long num = DEFAULTNUM;
	int c = -1, j = -1, ret = -1, fd = -1;
	
	while ((c = getopt(argc, argv, "i:b:c:n:")) != -1){
		switch (c){
			case 'i':
				input = optarg;
				break;
			case 'b':
				blocksize = strtoul(optarg, NULL, 10);
				break;
			case 'c':
				blocks = strtoul(optarg, NULL, 10);
				break;
			case 'n':
				num = strtoul(optarg, NULL, 10);
				break;
			default:
				usage();
		}
	}
	
	if(input == NULL){
		usage();
	}
	
	assert(blocksize > 0 && blocks > 0 && num >= 0);
	if( (buf = (char*)malloc(blocksize)) == NULL){
		fprintf(stderr, "failed to malloc the buffer\n");
		return -1;
	}
	
	printf("inputdevice: %s, blocksize: %lu, blocks: %lu, iteratenum: %lu\n\n",
				input, blocksize, blocks, num);
				
	if( (fd = open(input, O_RDONLY)) == -1){
		fprintf(stderr, "failed to open %s\n", input);
		return -1;
	}
	
	while(i < blocks){
		gettimeofday(&sstamp, 0);
		if( (ret = read(fd, buf, blocksize)) < 0){
			fprintf(stderr, "read fails: %d\n", ret);
			return ret;
		}
		gettimeofday(&estamp, 0);
		readtime += get_us(sstamp, estamp);
		for(j = 0; j < num; j++);
		i++;
		
	}
	
	printf("\tread: %0.2f MBs, time: %0.2f s, throughput: %0.2f MB/s\n",
				(blocksize*blocks/(1024.0*1024.0)),
				(readtime/1000000.0), 
				(blocksize*blocks/(1024.0*1024.0))/(readtime/1000000.0));
	free(buf);
	close(fd);
}
