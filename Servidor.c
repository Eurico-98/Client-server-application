#include "header.h"

int client_counter;
//variaveis globais para aceitar um cliente TCP
int fd,  client,  client_addr_size;
struct sockaddr_in addr, client_addr;//estrutura para os dados: ip, porto e familia do servidor

//variaveis globais para aceitar um cliente UDP
int Socket, recv_len;
struct sockaddr_in address_cliente, address_servidor;
socklen_t socket_len = sizeof(address_cliente);

//variaveis globais para o LIST igual para o TCP e UDP
char comand[size], buffer[size];
DIR *dirp;
struct dirent *dp;

//variaveis globais para download igual prar TCP e UDP
FILE *fp;
char buffer[size], *separator, file[100];
int bytes_enviados = 0, aux, valido = -1;
unsigned long bytes_em_falta;

void erro(char *msg){
	printf("Error: %s\n", msg);
	exit(-1);
}

//*********************************************************DOWNLOAD TCP***************************************************************//
void downloadsTCP(int client){

	bzero(comand, size);
	read(client, comand, size);//espera próximos dados do comando download
	
	separator = strtok(comand," ");//separa os dados recebidos para saber a encriptaçao e o ficheiro pedido
	separator = strtok(NULL,"\n ");//encriptação

	if( strcmp(separator,"ENC") == 0) printf("\nencripta\n");//encripta_dados();

	//verifica se o ficheiro pedido exite
	separator = strtok(NULL,"\n ");//ficheiro esta nesta posiçao da string recebida

	dirp = opendir("/home/user/Desktop/IRC/server_files");
	while((dp = readdir(dirp)) != NULL){
		bzero(buffer, size);
		sprintf(buffer,"%s",dp->d_name);
		if( strcmp(separator,buffer) == 0){
			valido = 1;
			break;
		}
		else valido = -1;
	}
	closedir(dirp);	
	
	if(valido == 1){
		write(client,"valido",sizeof("valido"));
		
		strcpy(file,"/home/user/Desktop/IRC/server_files/");
		strcat(file,separator);//adiciona o ficheiro ao path e continua
		if((fp = fopen(file,"r")) == NULL){
			perror("Erro a abrir o ficheiro");
			exit(-1);
		}
	
		bzero(buffer, size);
		fseek(fp,0,SEEK_END);
		bytes_em_falta = ftell(fp);
		sprintf(buffer,"%lu",bytes_em_falta);
		sleep(1);//para que o sevidor consiga enviar tudo
		write(client, buffer, size);//numero total de bytes a enviar
		bzero(buffer,size);
		
		bytes_enviados = 0;
		while( bytes_em_falta > 0){
		
			fseek(fp, bytes_enviados, SEEK_SET);//percorre o ficheiro desde o inicio ate ao ultimo byte enviado
			aux = fread(buffer,1,size,fp);//aux: divide a mensagem a enviar para que nao ultrapasse o limite do pacote
			write(client,buffer,aux);//envia para o cliente
			bytes_em_falta -= aux;//decrementa bytes
			bytes_enviados += aux;//incrementa
		}
	}
	else write(client,"invalido",sizeof("invalido"));
	
	bzero(comand, size);
	bzero(buffer,size);
	fclose(fp);
}

//**************************************************************LIST TCP**************************************************************//
void process_clientTCP() {	
	
	while(1){
		bzero(comand,size);//limpa o buffer
		printf("\nProcessando cliente %d, á espera comando...\n",client_counter);
		read(client, comand, size);//le o comando enviado pelo cliente
		printf("Comando recebido: (%s)\n",comand);
		
		if(strcmp(comand,"DOWNLOAD") == 0)  downloadsTCP(client);
		else{
			write(client,comand, size);
		
			if(strcmp(comand,"LIST") == 0){
			
				if((dirp = opendir("/home/user/Desktop/IRC/server_files")) == NULL){
					perror("Erro a abir diretoria!");
					exit(-1);
				}
				
				while((dp = readdir(dirp)) != NULL){
					bzero(buffer, size);
					sprintf(buffer,"%s ",dp->d_name);
					printf("%s\n",buffer);
					write(client,buffer, size);
				}
				closedir(dirp);
				bzero(buffer,strlen(buffer));
				
				//para que o cliente termine a ligação
				write(client,"Fim",sizeof("Fim"));
			}
			
			else if(strcmp(comand,"QUIT") == 0){
				printf("\nCLIENTE SAIU\n");
				break;//termina a ligação
			}
			
			else{
				bzero(comand, size);
				strcpy(comand,"comand not found");
				write(client, comand, strlen(comand));
			}
		}
	}
	close(client); //termina ligação com o cliente
}

//************************************************************DOWNLOAD UDP************************************************************//
void downloadsUDP(){
	
	bzero(comand, size);
	recvfrom(Socket, comand, size, 0, (struct sockaddr *) &address_cliente, (socklen_t *)&socket_len);
	
	separator = strtok(comand," ");//separa os dados recebidos para saber a encriptaçao e o ficheiro pedido
	separator = strtok(NULL,"\n ");//encriptação

	if( strcmp(separator,"ENC") == 0) printf("\nencripta\n");//encripta_dados();

	//verifica se o ficheiro pedido exite
	separator = strtok(NULL,"\n ");//ficheiro esta nesta posiçao da string recebida

	dirp = opendir("/home/user/Desktop/IRC/server_files");
	while((dp = readdir(dirp)) != NULL){
		bzero(buffer, size);
		sprintf(buffer,"%s",dp->d_name);
		if( strcmp(separator,buffer) == 0){
			valido = 1;
			break;
		}
		else valido = -1;
	}
	closedir(dirp);
	
	if(valido == 1){
	
		sendto(Socket, "valido", sizeof("valido"), 0,(struct sockaddr *) &address_cliente, socket_len);
		
		strcpy(file,"/home/user/Desktop/IRC/server_files/");
		strcat(file,separator);//adiciona o ficheiro ao path e continua
		if((fp = fopen(file,"r")) == NULL){
			perror("Erro a abrir o ficheiro");
			exit(-1);
		}
	
		bzero(buffer,size);
		fseek(fp,0,SEEK_END);
		bytes_em_falta = ftell(fp);
		sprintf(buffer,"%lu",bytes_em_falta);
		sleep(1);
		sendto(Socket, buffer, strlen(buffer), 0,(struct sockaddr *) &address_cliente, socket_len);
		bzero(buffer,size);
	
		bytes_enviados = 0;
		while( bytes_em_falta > 0){
		
			fseek(fp, bytes_enviados, SEEK_SET);//percorre o ficheiro desde o inicio ate ao ultimo byte enviado
			aux = fread(buffer, 1,size,fp);//aux: divide a mensagem a enviar para que nao ultrapasse o limite do 	pacote
			sendto(Socket, buffer, aux, 0,(struct sockaddr *) &address_cliente, socket_len);
			bytes_em_falta -= aux;//decrementa bytes
			bytes_enviados += aux;//incrementa 
		}
	}
	else  sendto(Socket, "invalido", sizeof("invalido"), 0,(struct sockaddr *) &address_cliente, socket_len);
	
	bzero(buffer,size);
	fclose(fp);
}

//**************************************************************LIST UDP**************************************************************//
void process_clientUDP(int total_clientes, int porto){

	//socket para receber de pacotes UDP
	if((Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		erro("Erro na criação do socket");

	bzero( (void *) &address_servidor, sizeof(address_servidor));//limpa a address para nao apanhar uma existente
	address_servidor.sin_family = AF_INET;//127.0.0.1
	address_servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	address_servidor.sin_port = htons(porto);

	//ligar o socket com a address  do cliente
	if( bind(Socket,(struct sockaddr*) &address_servidor, sizeof(address_servidor)) == -1)
		erro("Erro no bind");

	while(1){ //cria novos clientes enquanto o limite nao for atingido
	
		bzero(comand,size);//limpa o comand para nao ficar com lixo
		printf("\nServidor UDP á espera de contacto...\n");
		recvfrom(Socket, comand, size, 0, (struct sockaddr *) &address_cliente, (socklen_t *)&socket_len);
		client_counter++;//incrementa contador de clientes
		
		printf("\nProcessando cliente %d, á espera comando...\n",client_counter);
		printf("Comando recebido: (%s)\n",comand);
		
		if(strcmp(comand,"DOWNLOAD") == 0)  downloadsUDP();
		else{
			sendto(Socket, comand, strlen(comand), 0,(struct sockaddr *) &address_cliente, socket_len);
		
			if(strcmp(comand,"LIST") == 0 ){
			
				if( (dirp = opendir("/home/user/Desktop/IRC/server_files")) == NULL){
					perror("Erro a abir diretoria!");
					exit(-1);
				}
				
				while( (dp = readdir(dirp)) != NULL){
			
					bzero(buffer, size);
					sprintf(buffer,"%s ",dp->d_name);
					printf("%s\n",buffer);
					sendto(Socket, buffer, strlen(comand), 0,(struct sockaddr *) &address_cliente, socket_len);
				}
				closedir(dirp);
				bzero(buffer,strlen(buffer));
			
				strcpy(buffer,"Fim");//para que o cliente termine a ligação
				sendto(Socket, buffer, strlen(comand), 0,(struct sockaddr *) &address_cliente, socket_len);
			}
		
			else if(strcmp(comand,"QUIT") == 0){
				printf("\nCLIENTE SAIU\n");
				break;//termina a ligação
			}
		
			else{
				bzero(comand, size);
				strcpy(comand,"comand not found");
				sendto(Socket, comand, strlen(comand), 0,(struct sockaddr *) &address_cliente, socket_len);
			}
		}
	}
	close(Socket);//termina ligação com o cliente caso QUIT
}

//********************************************************MAIN SERVIVOR***************************************************************//
int main(int n_arg,char *argv[]) {
	
	//validaçao dos argumentos de entrada
	if (n_arg != 3) {
		printf("Input: server <port> <max_clients>\n");
		exit(-1);
	}
	if ( atoi(argv[1]) != PORT) {
		printf("\nPorto tem de ser: 9000 !\n");
		exit(-1);
	}
	if ( !( atoi(argv[2])) ){
		printf("\nValor máximo de clientes inválido!\n");
		exit(-1);
	}

//**************************************************************SERVER UDP************************************************************//
	if (fork() == 0) {
		process_clientUDP( atoi(argv[2]), atoi(argv[1]));
		exit(0);
	}
	
//***************************************************************SERVER TCP***********************************************************//
	bzero((void *) &addr, sizeof(addr));//limpa a estrutura sockaddr para ter a certeza que nao tem lixo
	
	addr.sin_family = AF_INET;//familia do servidor
	addr.sin_addr.s_addr = htonl(INADDR_ANY);//IP do servidor
	addr.sin_port = htons(atoi(argv[1]));//porto do servidor recebido como argumento de entrada

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)//abre um socket do servidor para poder aceitar clientes
		erro(" socket creation failed");
		
	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)//faz o bind do ip do servidor com o seu socket
		erro(" bind failed");
		
	if( listen(fd, atoi(argv[2])) < 0)//prepara o socket para receber n clientes onde n = argv[2]
		erro(" listen failed");

 	while(1){//cria novos clientes enquanto o limite nao for atingido
 			
		client_addr_size = sizeof(client_addr);//para guardar o tamanho da estrutura sockaddr de cada cliente que se ligue
		while(waitpid(-1,NULL,WNOHANG)>0);//para evitar que que bloqueie se o filho nao tiver terminado
		
		printf("\nServidor TCP esperando contacto...\n");

		client = accept(fd,(struct sockaddr *) &client_addr, (socklen_t *) &client_addr_size);//espera por um cliente

		if (client > 0) {
			if (fork() == 0) {
				close(fd);//apos uma ligaçao criada fecha o fd para que possa ser usado numa nova ligaçao
				process_clientTCP();//cria processo filho para processar cliente TCP
				exit(0);
			}
			close(client);//termina a ligação com o cliente
		}
		client_counter++;//incrementa contador de clientes
	}
	wait(NULL);//caso o servidor termina espera que o filho termine para nao gere processo orfao
	return 0;
}
