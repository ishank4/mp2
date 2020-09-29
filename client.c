//ABOUT:  Code detailing the client functionality in Simple Broadcast Chat Server (SBCS)
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>


#include "client_server.h"
time_t current_time;

//connection is made, time to join the chat room

void start_chatting (int soc_fd, char * argv[]){

	struct sb_chat_server_hdr hdr;
	struct sb_chat_server_att attr;
	struct sb_chat_server_MSG msg;
	
	int status = 0;

	hdr.version = '3'; // as per the manual
	hdr.type    = '2'; //join request hdr

	attr.type = 2;//sending the username info
	attr.length = strlen(argv[3]) + 1; //length () username + null char
	strcpy(attr.PayloadData, argv[3]); // temp copy of username
	
	msg.hdr = hdr; 
	msg.attr[0] = attr;// just one attr for joining.

	write(soc_fd,(void *)&msg, sizeof(msg));

	status = read_msg(soc_fd);

	if (status==1)
		{ERROR_MSG("username already present.");
		 close(soc_fd);}

}	

//read server msg

int read_msg(int soc_fd){

	struct sb_chat_server_MSG server_msg;
	int i;
	int status_Value = 0;
	int numb_bytes = 0;

// bytes from the server is stored 
	numb_bytes = read(soc_fd,(struct sb_chat_server_MSG*)& server_msg, sizeof(server_msg));
// total count for payload check
	int total_count = 0;//
	
	// forward_msg

	//Testing conditions for check the username, compare the actual length with the length specified in the hdr, check the hdr type with the attr index. If all are correct print the msg. 

	if (server_msg.hdr.type==3){// forward msg
	
		if((server_msg.attr[0].PayloadData!=NULL||server_msg.attr[0].PayloadData!='\0') &&(server_msg.attr[1].PayloadData!=NULL||server_msg.attr[1].PayloadData!='\0') &&(server_msg.attr[0].type==4)
		 
                &&(server_msg.attr[1].type==2)){
		//to checkthe size of the payload matches the payload length specified. 
			for(i=0; i<sizeof(server_msg.attr[0].PayloadData);i++){
				if (server_msg.attr[0].PayloadData[i]=='\0'){// string is present
					total_count = i-1;
					break;
				}
			}//end for 
// to check the total count of msg att 
			if(total_count==server_msg.attr[0].length){
				
				printf("USER by the username  %s has sent %s",server_msg.attr[1].PayloadData,
					server_msg.attr[0].PayloadData);
				current_time = time (NULL);
		// time_ctrl function call	
				printf("CLIENT:: current time is :%s \n",asctime(localtime(&current_time)));

			}
// else confiton for the client 
			else ERROR_MSG("Incorrect length of message at client \n");
		}//mismatch of the any data such as payload data type or username data type 
		else ERROR_MSG("CLIENT: hdr type mismatch or null value has been recevied\n");

		status_Value = 0; //sucessfull/
	}//


	//NACK as sent by the server
	if(server_msg.hdr.type ==5){    // for header type 5 condition to write and update
		if((server_msg.attr[0].PayloadData!=NULL||server_msg.attr[0].PayloadData!='\0')
		 &&(server_msg.attr[0].type==1)){// if it is unsuccessful
			printf("CLIENT:: Failed  to join as of now, reason %s",server_msg.attr[0].PayloadData);
		}
		status_Value = 1;		
	}// if (server_msg.hdr.type)

//msg received from the server which is offline
	if (server_msg.hdr.type==6){
	//payload data message check	
		if ((server_msg.attr[0].PayloadData!=NULL||server_msg.attr[0].PayloadData!='\0')
		   &&server_msg.attr[0].type ==2){ //client info that has left the chatroom
		
			printf(" USERNAME:: %s has left the chat room now\n",server_msg.attr[0].PayloadData);
		}
	status_Value =0;//value is read 
	}

//acknowdledgment meesage from the client 
	if (server_msg.hdr.type==7){
		if ((server_msg.attr[0].PayloadData!=NULL||server_msg.attr[0].PayloadData!='\0')&&server_msg.attr[0].type ==4){
		   printf("TOTAL  number of clients and the ACK msg is %s\n",server_msg.attr[0].PayloadData);
		}
	status_Value =0;
	}

// NEW particpant has arrived the chatroom that needs to be handled 
	if (server_msg.hdr.type ==8){  //message header condiiton
		if ((server_msg.attr[0].PayloadData!=NULL||server_msg.attr[0].PayloadData!='\0')
		   &&server_msg.attr[0].type ==2){ //new has one joined
			printf("New user Name::  %s has joined the chatroom \n",server_msg.attr[0].PayloadData);
		}

	status_Value = 0; 
	}

// reurn status values 
	return status_Value;
}

// function to send the info to the server 

void sending(int soc_fd)
{
	struct sb_chat_server_hdr hdr;
	hdr.version = '3';
	hdr.type    = '4';//as defined in the manual.

	struct sb_chat_server_MSG msg;
	struct sb_chat_server_att attr;


	msg.hdr = hdr;// copying the hdr to the msg hdr.

	int num_read = 0;
	char temp_var[512];
	struct timeval waiting_time;
	fd_set read_f_d;

	waiting_time.tv_sec = 2;
	waiting_time.tv_usec = 0;

	FD_ZERO(&read_f_d); //clearing the read descriptor
	FD_SET(STDIN_FILENO, &read_f_d);// set to read from the input 

	select(STDIN_FILENO+1, &read_f_d, NULL, NULL, &waiting_time);

	if(FD_ISSET(STDIN_FILENO, &read_f_d)){
		num_read = read(STDIN_FILENO,temp_var, sizeof(temp_var));

		if (num_read >0)
			temp_var[num_read] = '\0';
	
	
	attr.type = 4;//msg as specified in the mannual 
	strcpy(attr.PayloadData, temp_var);// 
	msg.attr[0] = attr;
	msg.attr[0].length = num_read -1 ; //removing the extra read char
	write(soc_fd, (void *)&msg, sizeof(msg));

	}
	else {
//timeout condition from the client 
		printf("CLIENT:Timeout has happened from the client side \n");
	}
}



// main function for the client  

int main (int argc, char*argv[]){

	//int idle_time_count = 0;
	struct timeval idle_time_count;
	int selected_value_returned =0;
	if (argc!=4)
	{
		printf("CLIENT:USAGE:./client <IP_address> <port_num> <user_name> \n");
		ERROR_MSG("Please specify in the correct format as described");
	}
        char *uname, *IP_addr;
        IP_addr =argv[1];
        uname = argv[3];
	int soc_fd = socket(AF_INET, SOCK_STREAM, 0); // For both IPv4 AND IPv6
	char * p; // 
	//server add. 
	int port_num = strtol(argv[2],&p,10);
	struct hostent* IP =  gethostbyname(argv[1]);//IP address
	struct sockaddr_in server_address;
        struct addrinfo  check, *get_add_inf= NULL;  // this bifurcates IPVv4 vs IPv6 address
	bzero(&server_address, sizeof(server_address)); // 
	server_address.sin_family = AF_INET; //IPv4
	server_address.sin_port   = htons(port_num); // port number as MP1
	//memcpy(&server_address.sin_addr.s_addr, IP->h_addr, IP->h_length);
         int add_resolution = getaddrinfo( IP_addr, NULL, &check, &get_add_inf); // This helps in addres resolution between IPV4 and IPv6 

	//adding file descriptor to the select. 
	fd_set main_f_d;
	fd_set read_f_d;
	
	//clearing them and setting to zero. 
	FD_ZERO(&read_f_d);
	FD_ZERO(&main_f_d); 

	//connect to the server/or we can say the chatroom. 
	int connect_status = connect(soc_fd, (struct sockaddr *)&server_address, sizeof(server_address));
printf("hi exit if %d \n",connect_status );
	if (connect_status < 0)//error
		ERROR_MSG("Error in now connecting to the main server");
	
	printf("Connection Successfull ");

	start_chatting(soc_fd, argv);

	FD_SET(soc_fd, &main_f_d);// to check any input on socket connect
	FD_SET(STDIN_FILENO, &main_f_d);// to check about any input on the command line 


	while (1){

	read_f_d = main_f_d;

	idle_time_count.tv_sec =10;// waiting for 10 secs or the read file descriptor  to wake call 

	if ((selected_value_returned = select(soc_fd+1, &read_f_d,NULL,NULL,&idle_time_count))==-1)
		ERROR_MSG("CLIENT:SELECT issue happened");

	if (FD_ISSET(soc_fd,&read_f_d))//read from the socket
		read_msg(soc_fd);

	if (FD_ISSET(STDIN_FILENO, &read_f_d))//
		sending(soc_fd);
	}

//check the connection network 	

	printf("User has now left the chatroom and the chat has ended\n");
	printf("Closing the client now");
	return 0;

}
