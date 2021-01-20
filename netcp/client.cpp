//@author: Eduardo José Barrios García
//@Brief: This program allows us to send files between client and sever using multithreading functions and IP ports to support each sent file between the mentioned parties.

/*                                  @How_To_Compile:

                                    1. Open terminal 
                            2. Type "cd located_folder/netcp" 
                             3. Press the following command:

                         g++ client.cpp -lpthread -o sample_name

                            4. Press the following command:

                                     ./sample_name

                5. Open a new window inside your terminal, repeat step 2
                            6. Press the following command:

                       g++ server.cpp -lpthread -o sample_name

                            7. Press the following command:

                                     ./sample_name
*/

//Optimized for Mac OS X thread, file, mutex, and map issues

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <pthread.h> //necessary to work with threads
#include <fstream>
#include <semaphore.h> //this library will allow us to work with multiple process and thrreads
#define MAX 1024
#define PORT 5001
#define SA struct sockaddr 
using namespace std;

//initialization of int's we will be using through the code
int file_send_id=0;
int pause_id=0;
int resume_id=0;
int abort_id=0;
int quit=0;
int thread_count=0;
sem_t mutex;

string getMd5sum(string path)
{
	string execute = "md5sum ";
	execute.append(path);
	execute.append(" > md.txt");
	system(execute.c_str());
	ifstream FD;
	FD.open("md.txt");
	string readstr;
	getline(FD,readstr,' ');
	//cout<<readstr<<endl;
	return readstr;
}
void * send_file(void * param)
{

	//structures declaration
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// Here we assign IP and PORT to start processing the file holding
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
			//Initialize 
			char buff[MAX]; 
			int n; 
			fstream FD;
			string fromFile, Name, toFile, choice;
			
			strcpy(buff,"send\n");
			write(sockfd, buff, sizeof(buff)); 
			bzero(buff, sizeof(buff)); 
			printf("Enter File name : "); 
			n = 0; 
			/*while ((buff[n++] = getchar()) != '\n'); 
			write(sockfd, buff, sizeof(buff)); //send filename
			Name = buff;*/
			cin>>Name;
			cout << "File name is : " << Name;
			strcpy(buff,Name.c_str());
			write(sockfd, buff, sizeof(buff));
			//line of code that will allow us to open our files  on mac devices, using the undeclared mutex
			FD.open(Name, ios::in);
			//we put waiting the possible process
			sem_wait(&mutex);
			file_send_id++;
			int tid=file_send_id;
			sem_post(&mutex);
			
			cout<<"sending file. Transfer id = "<<tid<<endl;
			if(FD){
				if(!FD.is_open())
				{
					strcpy(buff, "0");
					write(sockfd, buff, sizeof(buff));
					cout<<"\nCould not open file"<<endl;
				}
				else
				{
					strcpy(buff, "1");
					write(sockfd, buff, sizeof(buff));
					
					string checksum = getMd5sum(Name);
					// send checksum of file to reciever
					strcpy(buff,checksum.c_str());
					write(sockfd, buff, sizeof(buff)); 
				
					int flag = 0;
					FD.seekg(0, ios::end);
	   				int file_size = FD.tellg();
	   				//cout<<"FILE SIZE "<<file_size<<endl;
	   				strcpy(buff,to_string(file_size).c_str());
	   				write(sockfd, buff, sizeof(buff)); 
	   				char file_contents[file_size]="";
	   				FD.close();
	   				FD.open(Name, ios::in);
					while(!FD.eof()) {
						
						sem_wait(&mutex);
						if(quit==1 || abort_id==tid)
						{
							sem_post(&mutex);
							close(sockfd);
							pthread_exit(NULL);
						}
						if(pause_id==tid && resume_id!=tid)
						{
							flag=0;
						}
						else
							flag=1;
						sem_post(&mutex);
						
						if(flag==1)
						{
							getline(FD, fromFile);
							//cout<<"File contents "<<fromFile<<endl;
							bzero(buff, sizeof(buff));
							n = 0; 
							// copy server message in the buffer 
							while ((fromFile[n]) != '\0') {
								buff[n] = fromFile[n]; 
								n++;
							}
							
							//strcat(file_contents,"\n");
							write(sockfd, buff, sizeof(buff)); 
							
							//read(sockfd, buff, sizeof(buff)); 
							strcat(buff,"\n");
							strcat(file_contents,buff);
							fromFile.clear();
						}
					}
					cout<<"File sent successfully. Transfer id: "<<tid<<endl;
					cout<<"File contents were : "<<endl;
					cout<<file_contents<<endl;
					
				}
			}
			else{
				strcpy(buff, "1");
				write(sockfd, buff, sizeof(buff));
				cout << "Please enter valid file name" << endl;
			}
	close(sockfd);
	sem_wait(&mutex);
	thread_count--;
	sem_post(&mutex);
	pthread_exit(NULL);

}

void * recv_file(void * param)
{

	
	
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
		
			char buff[MAX]; 
			int n; 
			fstream FD;
			string fromFile, Name, toFile, choice;
			strcpy(buff,"recv\n");
			write(sockfd, buff, sizeof(buff)); 
			
			bzero(buff, sizeof(buff)); 
			printf("Enter File name : "); 
			n = 0; 
			cin>>Name;
			strcpy(buff,Name.c_str());
			write(sockfd, buff, sizeof(buff)); 
			Name = buff;
			
			//check the file name what we send
			cout << "File name is : " << Name;
			
			
			
			//Receive acknowledge from server that it receive or not
			read(sockfd, buff, sizeof(buff)); 
			string check = buff;
			int flag=0;
			if(check == "1"){
				cout << "File found" << endl;
				
				read(sockfd, buff, sizeof(buff)); //read checksum
				string checksum = buff;
				
				string path="./client_receive/";
		
				FD.open(path+Name, ios::out );
				
				read(sockfd, buff, sizeof(buff)); //read the size
				int file_size = atoi(buff);
				int size=0;
				//cout<<"file sizr "<<file_size<<endl;
				char file_contents[file_size]="";
				bzero(buff, sizeof(buff));
				while(size < file_size){
				
					
					
					sem_wait(&mutex);
					if(quit == 1)
					{
						thread_count--;
						sem_post(&mutex);
						pthread_exit(NULL);
					}
					sem_post(&mutex);
					
					//Receive content from server
					read(sockfd, buff, sizeof(buff)); 
					size = size + strlen(buff)+1;
					strcat(buff, "\n");
					toFile = buff;
					FD << toFile;
					
					strcat(file_contents,buff);	
					
					if(strlen(buff)==0){
						
						break;
					}
					bzero(buff, sizeof(buff));
					toFile.clear();
								
				}
				if(size != file_size)
				{
					FD.close();
					remove((path+Name).c_str());
				}
				cout<<"File successfully recieved"<<endl;
				cout<<"File contents are :"<<endl;
				cout<<file_contents<<endl;
				FD.close();
				string new_checksum=getMd5sum(path+Name);
				
				if(checksum == new_checksum)
				{
					cout<<"File received correctly"<<endl;
				}
				else
					cout<<"File received with errors"<<endl;
				
				
			}
			else if(check == "0"){
				cout << "\nFile NOT found\n";
			}
			
	close(sockfd);
	sem_wait(&mutex);
	thread_count--;
	sem_post(&mutex);
	pthread_exit(NULL);
		
}
void * func(void * param) 
{ 
	char buff[MAX]; 
	int n; 
	fstream FD;
	string fromFile, Name, toFile, choice;
	
	int i=0;
	while(true)
	{
	
		if(i > 0)
		{
			
			cout<<"Do you want to continue sending and recieving? yes/no"<<endl;
			cin>>choice;
			
			if(choice == "no")
			{
				while(thread_count > 1)
					{
						sleep(1);
					}
				thread_count--;
				//break;
				pthread_exit(NULL);
			}
			else
				cin.ignore();
		}

		bzero(buff, sizeof(buff)); 
		
		cout << "Please enter one the following command\n1 : send\n2 : recv\n3 : quit\n";
		if(file_send_id > 0)
		{
			cout<<"4 : pause\n5 : resume\n";
		}
		printf("Enter your command : "); 
		n = 0; 
		while ((buff[n++] = getchar()) != '\n'); 
		choice = buff;
		cout << "your choice is :" << choice/* << "HAHAHAHAHA"*/;
		
		if(choice == "quit\n")
		{
				quit=1;
				while(thread_count > 1)
				{
					sleep(1);
				}
				thread_count--;
				pthread_exit(NULL);
				
		}
		else if(choice == "send\n")
		{
				//write(sockfd, buff, sizeof(buff)); 
				pthread_t thread1;
				
				sem_wait(&mutex);
				thread_count++;
				sem_post(&mutex);
				int ret1 = pthread_create(&thread1, NULL, send_file, NULL);
				if (ret1!=0)
				{
					printf("Error In Creating Thread with ID:%d\n",1);
				}
				//pthread_join(thread1,NULL);
				
				sleep(10);
		}
		else if(choice == "recv\n"){
			
				pthread_t thread1;
				
				sem_wait(&mutex);
				thread_count++;
				sem_post(&mutex);
				int ret1 = pthread_create(&thread1, NULL, recv_file, NULL);
				if (ret1!=0)
				{
					printf("Error In Creating Thread with ID:%d\n",1);
				}
				//pthread_join(thread1,NULL);
				
				sleep(10);
		}
		else if(file_send_id > 0 && choice == "pause\n")
		{
				cout<<"Enter transfer id to pause shipment"<<endl;
				cin>>n;
				cin.ignore();
				if(n<0 || n>file_send_id)
				{
					cout<<"Invalid transfer id"<<endl;
				}
				else
					pause_id = n;
					
					sleep(10);
		}
		else if(file_send_id > 0 && choice == "resume\n")
		{
				cout<<"Enter transfer id to resume shipment"<<endl;
				cin>>n;
				cin.ignore();
				if(n<0 || n>file_send_id)
				{
					cout<<"Invalid transfer id"<<endl;
				}
				else
					resume_id = n;
					
					sleep(10);
		}
		else if(file_send_id > 0 && choice == "abort\n")
		{
				cout<<"Enter transfer id to abort shipment"<<endl;
				cin>>n;
				cin.ignore();
				if(n<0 || n>file_send_id)
				{
					cout<<"Invalid transfer id"<<endl;
				}
				else
					abort_id = n;
					
					sleep(10);
		}
		else{
			cout << "Please enter valid command" << endl;
		}
		
		i++;
		
	}
	pthread_exit(NULL);
} 

int main() 
{ 
	int sockfd, connfd; 
	sem_init(&mutex,0,1);
	struct sockaddr_in servaddr, cli; 
	pthread_t thread;
	
	sem_wait(&mutex);
	thread_count++;
	sem_post(&mutex);
	
	// function for chat 
	int ret1 = pthread_create(&thread, NULL, func, NULL);
	if (ret1!=0)
	{
		printf("Error In Creating Thread with ID:%d\n",1);
	}
	
	pthread_join(thread,NULL);
	return 0;
} 

