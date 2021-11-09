#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <unistd.h>

#define TRUE 1
#define FALSE 0
//#define PORT 3000
#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;
#include "json.hpp"
#include <vector>
using json = nlohmann::json;
struct stclient
{
     int64_t id_client;
     char name[255];
     char passwd[8];
};
struct stsession
{
     int64_t id_client;
     int64_t id;
     int64_t id_message;
     bool isAuthorization;
     int fd;
};
struct stmessage
{
     int64_t id;
     char body[1000];
     char sender_login[255];
};




int getlist(std::vector <stclient*> &list_clients)
{

    stclient *st1=new stclient;
    memset(st1,0,sizeof(stclient));
    st1->id_client=list_clients.size();
    string str1="mike";
    memcpy(st1->name,str1.c_str(),str1.size());
    string pas1="12345678";
    memcpy(st1->passwd,pas1.c_str(),pas1.size());
    list_clients.push_back(st1);

    stclient *st2=new stclient;
    memset(st2,0,sizeof(stclient));
    st2->id_client=list_clients.size();
    string str2="ivan";
    memcpy(st2->name,str2.c_str(),str2.size());
    string pas2="12345678";
    memcpy(st2->passwd,pas2.c_str(),pas2.size());
    list_clients.push_back(st2);

    stclient *st3=new stclient;
    memset(st3,0,sizeof(stclient));
    st3->id_client=list_clients.size();
    string str3="alex";
    memcpy(st3->name,str3.c_str(),str3.size());
    string pas3="12345678";
    memcpy(st3->passwd,pas3.c_str(),pas3.size());
    list_clients.push_back(st3);

    return 0;
}

int main(int argc , char *argv[])
{
    if(argc != 2)
    {
        cerr << "Usage: port" << endl; exit(0); 
    } //grab the IP address and port number 

     int port = atoi(argv[1]);



    std::vector <stclient*> list_clients;
    std::vector <stsession*> list_sessions;
    std::vector <stmessage*> list_messages;
  if (getlist(list_clients)!=0)
  {
      printf("error geting list clients\n");
      exit(-1);
  }





    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
	max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
	
    char buffer[1025]; //data buffer of 1K
	
    //set of socket descriptors
    fd_set readfds;
    
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
	client_socket[i] = 0;
    }
	
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
	perror("socket failed");
	exit(EXIT_FAILURE);
    }
    
    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
	sizeof(opt)) < 0 )
    {
	perror("setsockopt");
	exit(EXIT_FAILURE);
    }
    
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );
	
    //bind
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
	perror("bind failed");
	exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", port);
	
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
	perror("listen");
	exit(EXIT_FAILURE);
    }
	
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
	
    while(TRUE)
    {
	//clear the socket set
	FD_ZERO(&readfds);
    
	//add master socket to set
	FD_SET(master_socket, &readfds);
	max_sd = master_socket;
	    
	//add child sockets to set
	for ( i = 0 ; i < max_clients ; i++)
	{
	    //socket descriptor
	    sd = client_socket[i];
		
	    //if valid socket descriptor then add to read list
	    if(sd > 0)
		FD_SET( sd , &readfds);
		
	    //highest file descriptor number, need it for the select function
	    if(sd > max_sd)
		max_sd = sd;
	}
    
	//wait for an activity on one of the sockets , timeout is NULL ,
	//so wait indefinitely
	activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
	if ((activity < 0) && (errno!=EINTR))
	{
	    printf("select error");
	}
	    
	//If something happened on the master socket ,
	//then its an incoming connection
	if (FD_ISSET(master_socket, &readfds))
	{
	    if ((new_socket = accept(master_socket,
		    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
	    {
		perror("accept");
		exit(EXIT_FAILURE);
	    }
	    
	    //inform user of socket number - used in send and receive commands
	    printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
		(address.sin_port));
			
	    //add new socket to array of sockets
	    for (i = 0; i < max_clients; i++)
	    {
		//if position is empty
		if( client_socket[i] == 0 )
		{
		    client_socket[i] = new_socket;
		    printf("Adding to list of sockets as %d\n" , i);
			
		    break;
		}
	    }
	}
	    
	//else its some IO operation on some other socket
	for (i = 0; i < max_clients; i++)
	{
	    sd = client_socket[i];
		
	    if (FD_ISSET( sd , &readfds))
	    {
		//Check if it was for closing , and also read the
		//incoming message
		if ((valread = read( sd , buffer, 1024)) == 0)
		{
		    //Somebody disconnected , get his details and print
		    getpeername(sd , (struct sockaddr*)&address , \
			(socklen_t*)&addrlen);
		    printf("Host disconnected , ip %s , port %d \n" ,
			inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
			
		    //Close the socket and mark as 0 in list for reuse
		    close( sd );
		    client_socket[i] = 0;
		}
		    
		//Echo back the message that came in
		else
		{
            //РАЗБИРАЕМ КОМАНДЫ
            ///////////////////////////////////////////
{
                        string responseString = buffer;

                        if (valread>10 )
                        {
                        json object = json::parse(responseString);
                        int nread=0;
                        string command;
                        int fdSender=0;
                        int idMessage=-1;
                        uint64_t idSession=0;

                                  std::cout << object.size()/*<<" количество элементов в массиве" <<object[0].size()*/<< '\n';
                                 int buflen = 1024;
                                //  buf = new char[buflen+1];
                                  char *buf =(char*)malloc(buflen+1);
                                  memset(buf,0,buflen+1);
                                  int id =0;

                                  if (object.contains("command"))
                                      command=  object["command"];

                                  if (object.contains("id"))
                                      id=  object["id"];

                                  if (!strcmp(command.c_str(),"HELLO"))
                                  {
                                      // HELLO ANSWER
                                      json j;
                                      j["auth_methon"]="plain-text";
                                      j["command"]="HELLO";
                                      j["id"]=id;
                                      std::cout <<"CMD HELLO ANSWER"<< j.dump()<< "\n";


                                      memcpy(buf,j.dump().c_str(),j.dump().size());
                                      nread=j.dump().size();
                                  }

                                  if (!strcmp(command.c_str(),"login"))
                                  {
                                      // LOGIN ANSWER
                                      string name,passwd;
                                      if (object.contains("login"))
                                          name=  object["login"];
                                      if (object.contains("passwd"))
                                          passwd=  object["passwd"];


                        // проверка на аутентификацию из списка пользователей
                                      int id_session=-1;
                                      for (auto cl: list_clients) {

                                          if (!strcmp(name.c_str(),cl->name) && !strcmp(passwd.c_str(),cl->passwd))
                                          {
                                              stsession *sess=new stsession;
                                              sess->id=list_sessions.size();
                                              sess->id_client=cl->id_client;
                                              sess->isAuthorization=true;
                                               sess->fd=client_socket[i];
                                              list_sessions.push_back(sess);
                                              id_session=sess->id;

                                          }
                                      }


                                      json j;
                                      //если пользователя нашли
                                    if (id_session>-1)
                                      {
                                          j["session"]=id_session;
                                          j["status"]="ok";
                                          j["command"]="login";
                                          j["id"]=id;
                                      }
                                      else { //err no auth
                                          j["message"]="Ошибка при авторизации!";
                                          j["status"]="failed";
                                          j["command"]="login";
                                          j["id"]=id;

                                      }

                                      std::cout <<"CMD login ANSWER"<< j.dump()<< "\n";

                                      memcpy(buf,j.dump().c_str(),j.dump().size());
                                      nread=j.dump().size();
                                  }

                                 else if (!strcmp(command.c_str(),"message"))
                                  {
                                      // message ANSWER

                                      uint64_t session =0;

                                      if (object.contains("session"))
                                         session   =  object["session"];

                                      json j;

                                      if (object.contains("sender_login"))
                                      {// Отсылка сообщения конкретному пользователю
                                          string sender_login   =  object["sender_login"];


                                          if (list_sessions.size()>session && list_sessions[session]->isAuthorization ) //todo  если авторезирован ???

                                          {
                                              int id_login=-1;
                                              for (auto cl: list_clients) {

                                                  if (!strcmp(sender_login.c_str(),cl->name))
                                                  {
                                                      id_login=cl->id_client;
                                                  }
                                              }
                                            if (id_login>-1)
                                            {
                                             stmessage *mes=new stmessage;
                                              memset(mes,0,sizeof(stmessage));
                                              if (object.contains("body"))
                                              {

                                                  mes->id=list_messages.size();
                                                  string str_body= object["body"];
                                                  memcpy(&mes->body,str_body.c_str(),str_body.size());
                                                  memcpy(&mes->sender_login,sender_login.c_str(),sender_login.size());
                                              //    mes->body =  object["body"];
                                                  list_messages.push_back(mes);
                                                  list_sessions[session]->id_message=mes->id;
                                              }

                                              j["client_id"]=mes->id;
                                              j["status"]="ok";
                                              j["command"]="message_reply";
                                              j["id"]=id;
                                              idMessage=mes->id;
                                              fdSender=list_sessions[session]->fd;
                                              idSession=session;

                                             }
                                            else
                                            {//err no auth
                                                j["message"]="Сообщение передать нельзя! Пользователь отсутсвует!";
                                                j["status"]="failed";
                                                j["command"]="message_reply";
                                                j["id"]=id;

                                            }
                                            }


                                          else { //err no auth
                                              j["message"]="Ошибка! Пользователь не авторизован!";
                                              j["status"]="failed";
                                              j["command"]="message_reply";
                                              j["id"]=id;
                                          }


                                      }
                                      /////////////////////////////////////////////////////////
                                      else// Отсылка сообщения на сервер для рассылки всем авторизованным пользователям

                                      {

                                          if (list_sessions.size()>session && list_sessions[session]->isAuthorization) //todo  если авторезирован ???
                                          {

                                             stmessage *mes=new stmessage;
                                              memset(mes,0,sizeof(stmessage));
                                              if (object.contains("body"))
                                              {

                                                  mes->id=list_messages.size();
                                                  string str_body= object["body"];
                                                  memcpy(&mes->body,str_body.c_str(),str_body.size());
                                                  list_messages.push_back(mes);
                                                  list_sessions[session]->id_message=mes->id;
                                              }

                                              j["client_id"]=mes->id;
                                              j["status"]="ok";
                                              j["command"]="message_reply";
                                              j["id"]=id;
                                              idMessage=mes->id;
                                              idSession=session;
                                          }

                                          else { //err no auth
                                              j["message"]="Ошибка! Пользователь не авторизован!";
                                              j["status"]="failed";
                                              j["command"]="message_reply";
                                              j["id"]=id;
                                          }

                                      }
                                      std::cout <<"CMD message ANSWER"<< j.dump()<< "\n";
                                      memcpy(buf,j.dump().c_str(),j.dump().size());
                                      nread=j.dump().size();
                                  }

                                  else if (!strcmp(command.c_str(),"ping"))
                                  {
                                      // ping ANSWER
                                      uint64_t session =0;
                                      if (object.contains("session"))
                                         session   =  object["session"];
                                      json j;
                                      if (list_sessions.size()>session && list_sessions[session]->isAuthorization) //todo  если авторезирован ???
                                      {
                                          j["status"]="ok";
                                          j["command"]="ping_reply";
                                          j["id"]=id;
                                      }
                                      else { //err no auth
                                          j["message"]="Пользователь не авторизован!";
                                          j["status"]="failed";
                                          j["command"]="ping_reply";
                                          j["id"]=id;

                                      }

                                      std::cout <<"CMD ping ANSWER"<< j.dump()<< "\n";
                                      memcpy(buf,j.dump().c_str(),j.dump().size());
                                      nread=j.dump().size();
                                  }
                                  else if (!strcmp(command.c_str(),"logout"))
                                  {

                                      uint64_t session =0;
                                      if (object.contains("session"))
                                      {
                                         session   =  object["session"];
                                         if (list_sessions.size()>session && list_sessions[session]->isAuthorization)
                                         {
                                             list_sessions[session]->isAuthorization=false;
                                         }
                                      }

                                    // logout ANSWER

                                      json j;

                                          j["status"]="ok";
                                          j["command"]="logout_reply";
                                          j["id"]=id;
                                      std::cout <<"CMD logout ANSWER"<< j.dump()<< "\n";

                                      memcpy(buf,j.dump().c_str(),j.dump().size());
                                      nread=j.dump().size();
                                  }

            /////////////////////////////////ОТПРАВКА ОТВЕТА///////////////////////////////////
                                  string strmsg;
                                  char str[10];
                                  sprintf (str, "user%d:", i);
                                  strmsg.append(str);
                                  printf("ANSWER %s %s \n",buf,  strmsg.c_str());

                                  send(sd , buf ,  nread+1, 0 );


          //РАССЫЛКА СООБЩЕНИЙ ВСЕМ ПОЛЬЗОВАТЕЛЯМ
                                  if (idMessage>-1)
                                  {
                                      if(fdSender>0)
                                      {
                                          if (FD_ISSET( list_sessions[ idSession]->fd , &readfds)  )
                                          {

                                              string strmsg;
                                              char str[10];
                                              sprintf (str, "%s (личное):",list_clients[ list_sessions[ idSession]->id_client] ->name);
                                              strmsg.append(str);

                                              strmsg.append(list_messages[idMessage]->body, strlen(list_messages[idMessage]->body));
                                              strmsg.append("\r\n");

                                              printf("MESSAGE %s \n",  strmsg.c_str());

                                              send(list_sessions[ idSession]->fd , strmsg.c_str() ,  strmsg.size(), 0 );
                                          }
                                      }
                                        else
                                          {

                                              for (auto sl: list_sessions) {

                                                  if (sl->fd>0 && sl->isAuthorization)

                                                  {

                                                      if (FD_ISSET( sl->fd , &readfds)  )
                                                      {

                                                          string strmsg;
                                                          int f=list_sessions[ idSession]->id_client;
                                                          string fff=list_clients[ list_sessions[ idSession]->id_client] ->name;

                                                        strmsg.append(" :");
                                                          strmsg.append(list_messages[idMessage]->body, strlen(list_messages[idMessage]->body));
                                                          strmsg.append("\r\n");

                                                          printf("MESSAGE %s \n",  strmsg.c_str());

                                                          send(sl->fd , strmsg.c_str() ,  strmsg.size(), 0 );
                                                      }
                                                  }
                                              }

                                      }
                                  }
              }

        }
        }
        }
	}
    }
	
    return 0;
}
