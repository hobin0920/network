/*
   C ECHO client example using sockets
 */
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <inttypes.h>

#include <errno.h>
#define LENGTH 512

void Receive_File(int sock)
{
    /*Receive File from Client */
    char fr_name[100];
    char tmp[100]="/receive.txt";
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
	while((fr_block_sz = recv(sock, revbuf, LENGTH, 0)) > 0)
	{

	    if (fr_block_sz < LENGTH) {
	        printf ("wait for block transfer done !!! size=%d",fr_block_sz);
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
		int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
	        continue;
	    }else {
		printf("size=%d", fr_block_sz);
	    }

	    int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
	    if(write_sz < fr_block_sz)
	    {
		printf("File write failed on server.\n");
	    }
	    bzero(revbuf, LENGTH);
	    if (fr_block_sz == 0 || fr_block_sz != 512)
	    {
		printf("break size =%d",fr_block_sz);
		break;
	    }
	    puts(" ");
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
		return;
	    }
	}
	printf("Ok received from client!\n");
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
	return;
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

void send_receive_message(int sock){

    int read_size;
    char message[1000], server_reply[2000];

    while(1)
    {
	printf("Enter message : ");
	scanf("%s" , message);

	////Send some data
	if( send(sock , message , strlen(message) , 0) < 0)
	{
	    puts("Send failed");
	    return;
	}

	//Receive a reply from the server
	if( recv(sock , server_reply , 2000 , 0) < 0)
	{
	    puts("recv failed");
	    break;
	}

	puts("Server reply :");
	puts(server_reply);
	memset(server_reply,0,sizeof(server_reply));
    }
}



#if 1
int main(int argc , char *argv[])
{
    int sock, c;
    struct sockaddr_in server;
    int slen=sizeof(server);
    char message[1000] , server_reply[2000], IP[20];
    uint16_t port;
    int count =0;
    int protocol =-1, job=-1;

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    //Create socket

    printf("Choose 1.TCP 2.UDP : ");
    scanf("%d" , &protocol);
    if (protocol ==2) {
	sock = socket(AF_INET , SOCK_DGRAM, 0);
    } else {
	sock = socket(AF_INET , SOCK_STREAM , 0);
    }

    if (sock == -1)
    {
	printf("Could not create socket");
    }
    puts("client Socket created");

    //if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	//printf("setsockopt failed\n");

    //if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	//printf("setsockopt failed\n");

    printf("Enter Server IP : ");
    scanf("%s" , IP);
    server.sin_addr.s_addr = inet_addr(IP);
    server.sin_family = AF_INET;
    printf("Enter Server Port : ");
    scanf("%hd", &port);
    server.sin_port = htons(port);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
	perror("connect failed. Error");
	return 1;
    }

    puts("Connected\n");

    if (protocol ==2) {
	while(1)
	{
	    printf("Enter message : ");
	    gets(message);

	    //send the message
	    if (sendto(sock, message, strlen(message) , 0 , (struct sockaddr *) &server, slen)==-1)
	    {
		puts("sendto()");
	    }

	    //receive a reply and print it
	    //clear the buffer by filling null, it might have previously received data
	    memset(message,'\0', 1000);
	    //try to receive some data, this is a blocking call
	    if (recvfrom(sock, message, 1000, 0, (struct sockaddr *) &server, &slen) == -1)
	    {
		puts("recvfrom()");
	    }
	    puts(message);
	}
    }else {


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


	//SendFile(sock);
	//keep communicating with server
	//while(1)
	//{
	//    //printf("Enter message : ");
	//    //scanf("%s" , message);

	//    ////Send some data
	//    //if( send(sock , message , strlen(message) , 0) < 0)
	//    //{
	//    //    puts("Send failed");
	//    //    return 1;
	//    //}
	//    //if (count==0){
	//    //    count++;
	//    //    SendFile(sock);
	//    //}
	//    //Receive_File(sock);

	//    //Receive a reply from the server
	//    if( recv(sock , server_reply , 2000 , 0) < 0)
	//    {
	//	puts("recv failed");
	//	break;
	//    }

	//    puts("Server reply :");
	//    puts(server_reply);
	//    memset(server_reply,0,sizeof(server_reply));
	//}
    }
    close(sock);
    return 0;
}
#endif
