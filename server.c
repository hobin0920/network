/*
   C socket server example, handles multiple clients using threads
 */

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>

#define LENGTH 1000

//the thread function
void *connection_handler(void *);

void Receive_File(int sock)
{
    /*Receive File from Client */
    char fr_name[100];
    char tmp[100]="/abc.h264";
    getcwd(fr_name,100);
    strcat(fr_name,tmp);
    printf("\n %s \n",fr_name);


    //char* fr_name = "/home/hobinlin/AAA/network/linux/receive.txt";
    FILE *fr = fopen(fr_name, "a");
    char revbuf[LENGTH]; // Receiver buffer
    if(fr == NULL)
	printf("File %s Cannot be opened file on server.\n", fr_name);
    else
    {
	bzero(revbuf, LENGTH);
	int fr_block_sz = 0;
	int total_size=0;
	//while((fr_block_sz = recv(sock, revbuf, LENGTH, 0)) > 0)
	while(1)
	{
	    fr_block_sz = recv(sock, revbuf, LENGTH, 0);
	    total_size=total_size+fr_block_sz;
	    printf("total_size=%d fr_block_sz=%d \n",total_size,fr_block_sz);

	    if (fr_block_sz < LENGTH && fr_block_sz!=0) {
		printf ("wait for block transfer done !!! size=%d",fr_block_sz);
		int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
		continue;
	    }


	    int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
	    if(write_sz < fr_block_sz)
	    {
		error("File write failed on server.\n");
	    }
	    bzero(revbuf, LENGTH);
	    if (fr_block_sz == 0 || fr_block_sz != LENGTH)
	    {
		break;
	    }
	}
	if(fr_block_sz < 0)
	{
	    if (errno == EAGAIN)
	    {
		printf("recv() timed out.\n");
	    }
	    else
	    {
		fprintf(stderr, "recv() failed due to errno = %d\n", errno);
		exit(1);
	    }
	}
	printf("Ok received from client  %d!\n",total_size);
	fclose(fr);
    }
}

void SendFile(int sock)
{
    char fs_name[100];
    char tmp[100]="/test.txt";
    getcwd(fs_name,100);
    strcat(fs_name,tmp);
    printf("\n %s \n",fs_name);

    //char* fs_name = "/home/hobinlin/AAA/network/linux/test.txt";
    char sdbuf[LENGTH];
    printf("[Client] Sending %s to the Server... ", fs_name);
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    {
	printf("ERROR: File %s not found.\n", fs_name);
	exit(1);
    }

    bzero(sdbuf, LENGTH);
    int fs_block_sz;
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0)
    {
	if(send(sock, sdbuf, fs_block_sz, 0) < 0)
	{
	    fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
	    break;
	}
	bzero(sdbuf, LENGTH);
    }
    printf("Ok File %s from Client was Sent!\n", fs_name);
}

#if 1
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    struct ifreq ifr;
    char ip[30];
    int protocol=-1;
    char buf[2000];
    int slen = sizeof(client);
    int recv_len;
    //Create socke

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    printf("Choose 1.TCP 2.UDP : ");
    scanf("%d" , &protocol);
    if (protocol==2) {
	//UDP
	socket_desc = socket(AF_INET , SOCK_DGRAM, 0);
    } else {
	//TCP
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    }
    if (socket_desc == -1)
    {
	printf("Could not create socket");
    }
    puts("Server Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    //Get IP address of an interface on linux
    //strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(socket_desc, SIOCGIFADDR, &ifr);
    printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    sprintf(ip,"%s",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons( 8888 );

    int on=1;
    if((setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
    {
	perror("setsockopt failed");
	exit(EXIT_FAILURE);
    }


    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
	//print the error message
	perror("bind failed. Error");
	return 1;
    }
    puts("bind done");

    //keep listening for data
    if (protocol ==2){
	while(1)
	{
	    printf("Waiting for data...");
	    fflush(stdout);

	    //try to receive some data, this is a blocking call
	    if ((recv_len = recvfrom(socket_desc, buf, 1000, 0, (struct sockaddr *) &client, &slen) == -1))
	    {
		puts("recvfrom() error");
	    }

	    //print details of the client/peer and the data received
	    printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
	    printf("Data: %s\n" , buf);

	    //now reply the client with the same data
	    if (sendto(socket_desc, buf, strlen(buf), 0, (struct sockaddr*) &client, slen) == -1)
	    {
		puts("sendto() error");
	    }
	}
    }else {
	//Listen
	listen(socket_desc , 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);


	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
	    puts("Connection accepted");

	    pthread_t sniffer_thread;
	    new_sock = malloc(1);
	    *new_sock = client_sock;

	    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
	    {
		perror("could not create thread");
		return 1;
	    }

	    //Now join the thread , so that we dont terminate before the thread
	    //pthread_join( sniffer_thread , NULL);
	    puts("Handler assigned");
	}

	if (client_sock < 0)
	{
	    perror("accept failed");
	    return 1;
	}
    }
    return 0;
}
#endif

void send_receive_message(int sock){

    int read_size;
    char client_message[2000];
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //printf("%s",client_message);
        printf("%d",read_size);

        puts("");
        //Send the message back to client
        //write(sock , client_message , strlen(client_message));
        memset(client_message,0,sizeof(client_message));
    }

    if(read_size == 0)
    {
	puts("Client disconnected");
	fflush(stdout);
    }
    else if(read_size == -1)
    {
	perror("recv failed");
    }

}


/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int count=0,job=0;


    puts("1. send/receice massage");
    puts("2. send file");
    puts("3 receive file");

    scanf("%d" , &job);
    switch (job)
    {
	case 1:
	    puts("send/receice massage");
	    send_receive_message(sock);
	break;

	case 2:
	    puts("send file");
	    SendFile(sock);
	break;

	case 3:
	    puts("receive file");
	    Receive_File(sock);
	break;

	default:
	    puts("nothing to do GG");
	break;
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}
