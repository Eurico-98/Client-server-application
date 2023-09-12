#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/ipc.h>		
#include <sys/shm.h>

#define PORT 9000
#define PROXY 9002
#define size 65536//tamanho total em bytes que os protocolos conseguem enviar, 65536 = 64kB * 1024B

int shmid;

typedef struct shm{
	int save;
	int loss;
	int show;
	int packet_loss;
}SHM;
