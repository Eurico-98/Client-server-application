#include "header.h"

char IPservidor[100]; 
int i;
struct hostent *hostPtr;//para associar o IP do servidor à sua address igual para TCP e UDP

//variaveis globais para aceitar um cliente TCP
int proxySocket, fd,  client, client_addr_size;
struct sockaddr_in   addr,   client_addr, proxy_addr;


//variaveis globais para aceitar um cliente UDP
int Socket, proxy , recv_len;
struct sockaddr_in address_cliente, address_servidor;
socklen_t socket_len = sizeof(address_cliente);
socklen_t socket_len2 = sizeof(address_servidor);

//ficheiro para guardar infos
char file[] = {"/home/user/Desktop/IRC/proxy_files/lixo.txt"};
FILE *fp;

char IP_V4[10],IP_V5[10],IP_V4_TCP[10],IP_V5_TCP[10];

SHM *ptr_shm;

void erro(char *msg){
	printf("Error: %s\n", msg);
	exit(-1);
}

//****************************************************************UDP*****************************************************************//
void *UDPClient(){
	int aux;
	char comand[size];
	while(1){
		if (ptr_shm->show==1){
			printf("\nSHOW ATIVADO\n");
			printf("\nENDEREÇO ORIGEM : %s\tPORTO  : %d\n",IP_V5,PORT );
			printf("\nENDEREÇO DESTINO: %s\tPORTO  : %d\n",IP_V4,PROXY );
			printf("\nPROTOCOLO : TCP\n");
		}
		printf("\nWaiting UDP Client ----> ");
		aux = recvfrom(proxy, comand, size, 0, (struct sockaddr *) &address_cliente, (socklen_t *)&socket_len2);
		printf(" recebeu: %s\n",comand);
		sendto(Socket, comand , aux, 0, (struct sockaddr *) &address_servidor, socket_len);
		bzero(comand,size);
		aux = 0;
	}
}

void *UDPServer(){
	int aux;
	char comand[size];
	while(1){
		printf("Waiting UDP Server ----> ");
		aux = recvfrom(Socket, comand, size, 0, (struct sockaddr *) &address_servidor, (socklen_t *)&socket_len);
		if(ptr_shm->save == 1){
			fp = fopen(file,"a");
			fwrite(comand,1,aux,fp);//escreve para o ficheiro
			fclose(fp);
		}
		else if(ptr_shm->save == -1){//limpa o ficheiro
			fp = fopen(file,"w");
			fclose(fp);
		}
		if(ptr_shm->loss == 1){
			aux -= ptr_shm->packet_loss;
		}
		printf(" recebeu: %s\n",comand);
		sendto(proxy, comand , aux, 0, (struct sockaddr *) &address_cliente, socket_len2);
		bzero(comand,size);
		aux = 0;
	}
}

//****************************************************************TCP*****************************************************************//
void *TCPClient(){
	int aux;
	char comand[size];
	while(1){
		if (ptr_shm->show==1){
			printf("\nSHOW ATIVADO\n");
			printf("\nENDEREÇO ORIGEM : %s\tPORTO  : %d\n",IP_V5_TCP,PORT );
			printf("\nENDEREÇO DESTINO: %s\tPORTO  : %d\n",IP_V4_TCP,PROXY );
			printf("\nPROTOCOLO : TCP\n");
		}
		printf("\nWaiting TCPClient ----> ");
		aux = read(client,comand,size);
		printf(" recebeu: %s\n",comand);
		write(fd,comand,aux);
		bzero(comand,size);
		aux = 0;
	}
}

void *TCPServer(){
	int aux;
	char comand[size];
	
	while(1){
		printf("Waiting TCP Server ----> ");
		aux = read(fd,comand,size);
		if(ptr_shm->save == 1){
printf("\nENTROU NO save = 1\n");
			fp = fopen(file,"w");
			fwrite(comand,1,aux,fp);//escreve para o ficheiro
			fclose(fp);
		}
		else if(ptr_shm->save == -1){//limpa o ficheiro
printf("\nENTROU NO save = -1\n");
			fp = fopen(file,"w");

			fclose(fp);
		}
		printf(" recebeu: %s\n",comand);
		write(client,comand,aux);
		bzero(comand,size);
		aux = 0;
	}
}

//CRIA OS SOCKETS PARA LIGAÇÃO TCP
void process_proxyTCP(){

	//INFORMAÇÕES DO SERVER TCP 
	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons((short)PORT);
	

	//INFORMAÇÕES DO PROXY 
	bzero((void *) &proxy_addr, sizeof(proxy_addr));//limpa a estrutura sockaddr para ter a certeza que nao tem lixo
	proxy_addr.sin_family = AF_INET;//familia do servidor
	proxy_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	proxy_addr.sin_port = htons(PROXY);
	
	inet_ntop(AF_INET,&addr.sin_addr.s_addr,IP_V4_TCP,sizeof(IP_V4));
	inet_ntop(AF_INET,&proxy_addr.sin_addr.s_addr,IP_V5_TCP,sizeof(IP_V5));
	
	//SOCKET PARA O CLIENTE
	if ( (proxySocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)//abre um socket do proxy para poder aceitar clientes
		erro(" socket creation failed");
	
	if ( bind(proxySocket,(struct sockaddr*)&proxy_addr,sizeof(proxy_addr)) < 0)//faz o bind do ip do proxy com o seu socket
		erro(" bind failed");
	
	if( listen(proxySocket, 10) < 0)//prepara o socket para receber varios clientes
		erro(" listen failed");	
	
	while(1){
	
		client_addr_size = sizeof(client_addr);//para guardar o tamanho da estrutura sockaddr de cada cliente que se ligue
		
		while(waitpid(-1,NULL,WNOHANG)>0);//must use WNOHANG or would block whenever a child process was still working

		//wait for new connection
		client = accept(proxySocket,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);//aceitar o cliente

		if(client>0){

			//SOCKET PARA O SERVIDOR
			if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      				erro("ERRO A CRIAR SOCKET");
    				exit(-1);
    			}
    	
    			if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
      				erro("ERRO A CONECTAR PROXY AO SERVER");
      				exit(-1);
    			}
    		
			if(fork()==0){
				pthread_t threadsTCP[3];
				pthread_create(&threadsTCP[0], NULL, TCPClient, NULL);
    				pthread_create(&threadsTCP[1], NULL, TCPServer, NULL);

    				pthread_join(threadsTCP[0], NULL);
				pthread_join(threadsTCP[1], NULL);
			}
			close(client);	
		}

	}
}

void *COMANDOSauxiliar(){
	char comando_proxy[size];//apanha comandos
	int contadorS = 0, contadorL = 0, contador_save = 0;//conta para asber quando ativar/desativar
	ptr_shm->save = -1;
	ptr_shm->loss = -1;
	ptr_shm->show = -1;
	ptr_shm->packet_loss = 0;
	while(1){
		bzero(comando_proxy, size);
		fgets(comando_proxy, size, stdin);
		fflush(stdin);
		if(strcmp(comando_proxy,"SAVE\n") == 0){
			contador_save++;
			if(contador_save%2 == 0) ptr_shm->save =  -1;
			else ptr_shm->save = 1;
		}
		if(strcmp(comando_proxy,"SHOW\n") == 0){
			contadorS++;
			if(contadorS%2 == 0) ptr_shm->show = -1;
			else ptr_shm->show = 1;
		}
		if(strcmp(comando_proxy,"LOSS\n") == 0){
			printf("\nPercentagem de perdas:");
			scanf("%d",&(ptr_shm->packet_loss));
			fflush(stdin);
			contadorL++;
			if(contadorL%2 == 0) ptr_shm->loss = -1;
			else ptr_shm->loss = 1;
		}
	}
}

//******************************************************************MAIN**************************************************************//
int main(int n_arg,char *argv[]) {
	
	//validaçao dos argumentos de entrada
	if (n_arg != 2) {//argumntos de entrada tem de ser 2: porto e maximo de clientes
		printf("\nInput: proxy <port> \n");
		exit(-1);
	}

	if ( atoi(argv[1]) != PROXY) {
		printf("\nPorto tem de ser: 9002 !\n");
		exit(-1);
	}
	
	if((shmid = shmget(IPC_PRIVATE, sizeof(SHM), IPC_CREAT|0666)) == -1) exit(-1);
    	if((ptr_shm = (SHM *) shmat(shmid, NULL, 0)) == (SHM *)-1)  exit(-1);
	
	pthread_t threadsAUX;
	pthread_create(&threadsAUX, NULL, COMANDOSauxiliar, NULL);
	
//************************************************************PROXY TCP***************************************************************//
	
	if(fork()==0)	process_proxyTCP();

//************************************************************PROXY UDP***************************************************************//
   
  	bzero((void *)&address_cliente, sizeof(address_cliente));
	address_cliente.sin_family = AF_INET;
	address_cliente.sin_port = htons(PROXY);
	address_cliente.sin_addr.s_addr = htonl(INADDR_ANY);

	
	bzero((void *)&address_servidor, sizeof(address_servidor));
	address_servidor.sin_family = AF_INET;
	address_servidor.sin_port = htons(PORT);
	address_servidor.sin_addr.s_addr = htonl(INADDR_ANY);

  	inet_ntop(AF_INET,&address_cliente.sin_addr.s_addr,IP_V4,sizeof(IP_V4));
	inet_ntop(AF_INET,&address_servidor.sin_addr.s_addr,IP_V5,sizeof(IP_V5));
  
    	if((proxy = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		erro("\nCliente-proxy\n");

	
	if(bind(proxy,(struct sockaddr*)&address_cliente, sizeof(address_cliente)) == -1){
		erro("\nCLiente-Proxy BIND\n");
	}
	

	if((Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		erro("\nProxy-Server\n");
	}
    
	if(fork()==0){
			pthread_t threadsUDP[3];
			pthread_create(&threadsUDP[0], NULL, UDPClient, NULL);
    			pthread_create(&threadsUDP[1], NULL, UDPServer, NULL);

    			pthread_join(threadsUDP[0], NULL);
			pthread_join(threadsUDP[1], NULL);
	}	
	close(proxy);
	close(Socket);
	pthread_join(threadsAUX, NULL);
	wait(NULL);
	
	//termina e desmapeia a memoria partilhada
	shmdt(&ptr_shm);
	shmctl(shmid, IPC_RMID, NULL);
	
	return 0;
}
