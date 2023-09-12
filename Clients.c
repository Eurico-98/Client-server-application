#include "header.h"

//variaveis globais para o UDP
struct sockaddr_in address_servidor;
int Socket, recv_len, sendv_len;
socklen_t socket_len = sizeof(address_servidor);

//variaveis para a listagem igual para TCP e UDP
char *mensagem;
char buffer[size];
	
//variaveis para o download, igual para TCP e UDP
FILE *fp;
//char ficheiro[10];
char buffer[size], nome_fich[20], file_input[20], file[100], encriptacao[3];
int bytes_recebidos = 0, aux, pos = 0;
unsigned long bytes_em_falta;
clock_t start_t, end_t;

//variaveis para parsing
char *separator, dados[size];
int modo_encriptacao = 0, valido = 0;

//*********************************************************DOWNLOAD TCP***************************************************************//
void GereDownloadTCP(int fd){
	
	bzero(buffer, size);//limpa o buffer todo
	read(fd, buffer, size);//espera pela confirmação do ficheiro pedido

	if(strcmp(buffer,"valido") == 0){
			
		if(strcmp(encriptacao,"ENC") == 0) printf("\ndesencripta\n");//desencripta();
		strcpy(file,"/home/user/Desktop/IRC/downloads/");
		strcat(file,file_input);//acrescenta ao path o ficheiro pedido
		
		if((fp = fopen(file,"w")) == NULL){
			perror("Erro a abrir o ficheiro!");
			exit(-1);
		}
		
		bzero(buffer, size);
		read(fd, buffer, size);
		bytes_em_falta = atoi(buffer);
		bzero(buffer,size);

		start_t = clock();//marca o tempo de inicio do download
		while( bytes_em_falta > 0){
		
			fseek(fp, bytes_recebidos, SEEK_SET);//percorre o ficheiro desde o inicio ate ao ultimo byte enviado
			aux = read(fd,buffer, size);//aux: divide a mensagem enviada para que nao ultrapasse o limite do pacote
		
			fwrite(buffer,1,aux,fp);//escreve para o ficheiro
			
			bytes_em_falta -= aux;//decrementa bytes
			bytes_recebidos += aux;//incrementa 
		}
		end_t = clock();//marca o tempo de terminação do download
	
		bzero(buffer,size);
		fclose(fp);
		
		//informação do download
		for(int i=33; i <= strlen(file)-1; i++)
			nome_fich[i-33] = file[i];
		printf("\nDownload:\n");
		printf("Nome do ficheiro: %s\n",nome_fich);
		printf("Num de bytes recebidos: %d\n",bytes_recebidos);
		printf("Protocolo: TCP\n");
	  	printf("Tempo total de download do ficheiro: %f\n", (double)(end_t - start_t) / CLOCKS_PER_SEC );
	}
	else  printf("\nFicheiro não existente!\n");
}

//*************************************************************LIST TCP***************************************************************//
int listarTCP(int fd){	
	
	while(1){
	
		bzero(buffer,sizeof(buffer));	
		read(fd, buffer, sizeof(buffer));
		mensagem = strtok(buffer,"\n");
		
		if(strcmp(mensagem,"Fim") == 0){
			return 0;
		}
      		else 
      			printf("FICHEIRO: %s\n", buffer);	
    	}	
}

//*********************************************************DOWNLOAD UDP***************************************************************//
void GereDownloadUDP(int Socket){
	
	bzero(buffer, size);//limpa o buffer todo
	recvfrom(Socket, buffer, size, 0, (struct sockaddr *) &address_servidor, (socklen_t *) &socket_len);
	
	if(strcmp(buffer,"valido") == 0){
	
		if(strcmp(encriptacao,"ENC") == 0) printf("\ndesencripta\n");//desencripta();
		strcpy(file,"/home/user/Desktop/IRC/downloads/");
		strcat(file,file_input);//acrescenta ao path o ficheiro pedido
	
		if((fp = fopen(file,"w")) == NULL){
			perror("Erro a abrir o ficheiro!");
			exit(-1);
		}

		bzero(buffer, size);//limpa o buffer todo
		recvfrom(Socket, buffer, size, 0, (struct sockaddr *) &address_servidor, (socklen_t *) &socket_len);
		bytes_em_falta = atoi(buffer);
		bzero(buffer,size);

		start_t = clock();//marca o tempo de inicio do download
		while( bytes_em_falta > 0){	
			fseek(fp, bytes_recebidos, SEEK_SET);//percorre o ficheiro desde o inicio ate ao ultimo byte enviado	
			aux = recvfrom(Socket, buffer, size, 0, (struct sockaddr *) &address_servidor, (socklen_t *) &socket_len);
			fwrite(buffer,1,aux,fp);//escreve para o ficheiro		
			bytes_em_falta -= aux;//decrementa bytes	
			bytes_recebidos += aux;//incrementa 
		}
		end_t = clock();//marca o tempo de terminação do download
	
		bzero(buffer,size);
		fclose(fp);

		//informação do download
		for(int i=33; i <= strlen(file)-1; i++)
			nome_fich[i-33] = file[i];
		printf("\nDownload:\n");	
		printf("Nome do ficheiro: %s\n",nome_fich);
		printf("Num de bytes recebidos: %d\n",bytes_recebidos);
		printf("Protocolo: UDP\n");
  		printf("Tempo total de download do ficheiro: %f\n", (double)(end_t - start_t) / CLOCKS_PER_SEC );
  	}
	else  printf("\nFicheiro não existente!\n");
}

//*******************************************************LIST UDP*********************************************************************//
int listarUDP(int Socket){

printf("\nchegou ao listar UDP\n");

	while(1){
	
		bzero(buffer,sizeof(buffer));
		recvfrom(Socket, buffer, size, 0, (struct sockaddr *) &address_servidor, (socklen_t *)&socket_len);
		mensagem = strtok(buffer,"\n");
		
		if(strcmp(mensagem,"Fim") == 0){
			return 0;
		}
      		else 
      			printf("FICHEIRO: %s\n", buffer);	
	}
}

void erro(char *msg){
	printf("Error: %s\n", msg);
	exit(-1);
}

//***************************************************************MAIN*****************************************************************//
int main(int argc, char *argv[]) {

	char *mensagem, comand[size], IPservidor[100];
	
	if (argc != 5) {//argumentos de entrada tem de ser 2: ip e porto
		printf("Input: cliente <proxy ip> <host ip> <port> <protocol>\n");
		exit(-1);
	}
	
	if ( strcmp(argv[1],"127.0.0.1") != 0 || strcmp(argv[2],"127.0.0.1") != 0){
		printf("\nEndereço do proxy e do servidor têm de ser: 127.0.0.1 !\n");
		exit(-1);
	}
	
	if ( atoi(argv[3]) != PROXY) {
		printf("\nPorto tem de ser: 9002 !\n");
		exit(-1);
	}

	if ( strcmp(argv[4],"TCP") != 0 && strcmp(argv[4],"UDP") != 0) {
		printf("\nProtocolo inválido!\n");
		exit(-1);
	}
	
	struct hostent *hostPtr;//para associar o IP do servidor à sua address igual para TCP e UDP
	strcpy(IPservidor, argv[2]);//obtem o IP do servidor
	if( (hostPtr = gethostbyname(IPservidor)) == 0)
		erro(" Failed obtaning server IP");
	
//*************************************************************CLIENTE TCP************************************************************//
	if (strcmp(argv[4],"TCP") == 0){
	
		int fd;//para o socket
		
		struct sockaddr_in addr;//address do cliente TCP
		
		bzero((void *) &addr, sizeof(addr));//limpa a address para certtificar que nao se liga a uma que ja la esta previamente	
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
		addr.sin_port = htons((short) atoi(argv[3]));//porto do proxy
	
		if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
			erro(" failed to craete the socket");
	
		if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
			erro(" failed to connect to proxy");
			
		//pede o nome do comando e faz a respectiva ação
		printf("\nSelect comand: LIST/DOWNLOAD/QUIT\n");

		while(1){
			bzero(comand,size);//limpa o buffer
			fgets(comand,size,stdin);
			mensagem = strtok(comand,"\n");
			fflush(stdin);
			strcpy(comand, mensagem);
		
			if(strcmp(comand,"DOWNLOAD") == 0){
			
				write(fd, comand, strlen(mensagem));//envia o comando para o servidor
				
				//pede os resto dos dados
				printf("\nSelecionar:\nProtocolo: <TCP/UDP>\nModo de encriptaçao: <ENC/NOR>\nFicheiro de download\n");
				while( valido != 1){
					
					valido = 0;
					bzero(comand, size);
					fgets(dados,size,stdin);
					strcpy(comand, dados);
					fflush(stdin);
					
					//valida protocolo
					separator = strtok(dados,"\n ");
					if(strcmp(separator,"TCP") != 0)   valido =-1;
					
					//valida encriptaçao
					separator = strtok(NULL,"\n ");
					if(strcmp(separator,"ENC")!=0 && strcmp(separator,"NOR")!=0)   valido = -1;
					strcpy(encriptacao,separator);//guarda o modo de encriptação

					separator = strtok(NULL,"\n ");
					strcpy(file_input,separator);//guarda o ficheiro pedido
				
					if(valido == -1) printf("\nComando inválido, tentar novamente!\n");
					else valido = 1; //caso os dados sejam corretos
					
					bzero(dados, size);					
				}
				valido = 0;
				write(fd, comand, strlen(comand));//depois de validados envia os restantes dados
				GereDownloadTCP(fd);
				bzero(comand, size);//limpa o buffer para nao ficar com lixo
			}
			
			else{
				write(fd, comand, strlen(mensagem));//envia o comando para o servidor
		
				if(strcmp(comand,"QUIT")==0){//termina o programa caso QUIT
					printf("\nA sair......\n");
					break;
				}
				
				bzero(comand, size);//limpa buffer
				read(fd, comand, size);//espera pela validação do comando
				
				if(strcmp(comand,"comand not found") == 0)  erro(" comand not found");
				if(strcmp(comand,"LIST") == 0)	listarTCP(fd);
			}
			printf("\nSelect new comand or QUIT\n");
		}
		close(fd);//termina ligação		
	}
	
//*************************************************************CLIENTE UDP************************************************************//
	else {
		if((Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)//cria um socket para recepção de pacotes UDP
			erro("Erro na criação do socket");

		bzero( (void *) &address_servidor, sizeof(address_servidor));//limpa a address para nao apanhar uma existente
		address_servidor.sin_family = AF_INET;//127.0.0.1
		address_servidor.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
		address_servidor.sin_port = htons((uint16_t) atoi(argv[3]));

		printf("\nSelect comand: LIST/DOWNLOAD/QUIT\n");
		
		while(1){
			bzero(comand,size);//limpa o buffer
			fgets(comand,size,stdin);
			mensagem = strtok(comand,"\n");			
			fflush(stdin);
			
			if(strcmp(comand,"DOWNLOAD") == 0){
			
				//envia a mensagem para o Scoket == servidor
				sendto(Socket, comand, strlen(comand), 0, (struct sockaddr *) &address_servidor, socket_len);
				
				//pede os resto dos dados
				printf("\nSelecionar:\nProtocolo: <TCP/UDP>\nModo de encriptaçao: <ENC/NOR>\nFicheiro de download\n");
				while( valido != 1){
					
					bzero(comand, size);
					fgets(dados,size,stdin);
					strcpy(comand, dados);
					fflush(stdin);
					
					//valida protocolo
					separator = strtok(dados,"\n ");
					if(strcmp(separator,"UDP") != 0)   valido =-1;
					
					//valida encriptaçao
					separator = strtok(NULL,"\n ");
					if(strcmp(separator,"ENC")!=0 && strcmp(separator,"NOR")!=0)   valido = -1;
					strcpy(encriptacao,separator);//guarda o modo de encriptação

					separator = strtok(NULL,"\n ");
					strcpy(file_input,separator);//guarda o ficheiro pedido
				
					if(valido == -1) printf("\nComando inválido, tentar novamente!\n");
					else valido = 1; //caso os dados sejam corretos
					
					bzero(dados, size);					
				}
				valido = 0;
				sendto(Socket, comand, strlen(comand), 0, (struct sockaddr *) &address_servidor, socket_len);
				GereDownloadUDP(Socket);
				bzero(comand, size);//limpa o buffer para nao ficar com lixo
			}
			else {

				sendto(Socket, comand, strlen(comand), 0, (struct sockaddr *) &address_servidor, socket_len);
	
				if( strcmp(comand,"QUIT") == 0){//termina o programa caso QUIT
					printf("\nA sair......\n");
					close(Socket);
					break;
				}
				bzero(comand, size);//limpa buffer
		
				recv_len = recvfrom(Socket,comand,size,0,(struct sockaddr*)&address_servidor,(socklen_t *)&socket_len);
	
				comand[recv_len] = '\0';// Para ignorar o restante conteúdo (anterior ao buffer)

				if( strcmp(comand,"comand not found") == 0) 	erro(" comand not found");
				if( strcmp(comand,"LIST") == 0) listarUDP(Socket);
			}				
			printf("\nSelect new comand or QUIT\n");
		}
		close(Socket);//termina ligação
	}
	return 0;	
}
