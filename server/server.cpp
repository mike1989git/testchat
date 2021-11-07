#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <map>
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
};
struct stmessage
{
     int64_t id;
     string body;
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


int
main(int argc, char **argv)
{
    std::vector <stclient*> list_clients;
    std::vector <stsession*> list_sessions;
    std::vector <stmessage*> list_messages;
  if (getlist(list_clients)!=0)
  {
      printf("error geting list clients\n");
      exit(-1);
  }




    struct sockaddr_in server_addr,client_addr;
    socklen_t clientlen = sizeof(client_addr);
    int option, port, reuse;
    int server, client;
    char *buf;
    int buflen;
    int nread;

    // setup default arguments
    port = 3000;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"p:")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                cout << "server [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }

      // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

      // create socket
    server = socket(PF_INET,SOCK_STREAM,0);
    if (!server) {
        perror("socket");
        exit(-1);
    }

      // set socket to immediately reuse port when the application closes
    reuse = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

      // call bind to associate the socket with our local address and
      // port
    if (bind(server,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

      // convert the socket to listen for incoming connections
    if (listen(server,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }

    // allocate buffer
    buflen = 1024;
  //  buf = new char[buflen+1];
    buf =(char*)malloc(buflen+1);

      // accept clients
    while ((client = accept(server,(struct sockaddr *)&client_addr,&clientlen)) > 0) {
          // loop to handle all requests
        while (1) {
        
              // read a request
            memset(buf,0,buflen);
            nread = recv(client,buf,buflen,0);
            printf("buf = %s %d \n",buf,buflen);
///////////////////////////////////////////
            string responseString = buf;
            json object = json::parse(responseString);

            string command;

                      std::cout << object.size()/*<<" количество элементов в массиве" <<object[0].size()*/<< '\n';
                     int buflen = 1024;
                    //  buf = new char[buflen+1];
                      char *buf =(char*)malloc(buflen+1);
                      memset(buf,0,buflen);
                      int id =0;

                      if (object.contains("command"))
                          command=  object["command"];

                      if (object.contains("id"))
                          id=  object["id"];

                      if (!strcmp(command.c_str(),"HELLO"))
                      {
                          //TODO HELLO ANSWER
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
                          //TODO LOGIN ANSWER
                          string name,passwd;
                          if (object.contains("login"))
                              name=  object["login"];
                          if (object.contains("passwd"))
                              passwd=  object["passwd"];


            //todo проверка на аутентификацию из списка пользователей
                          bool auth_login=false;
                          for (auto cl: list_clients) {

                              if (!strcmp(name.c_str(),cl->name) && !strcmp(passwd.c_str(),cl->passwd))
                              {
                                  stsession sess;
                                  sess.id=list_sessions.size()+1;
                                  sess.id_client=cl->id_client;
                                  sess.isAuthorization=true;
                                  list_sessions.push_back(&sess);
                                  auth_login=true;
                              }
                          }


                          json j;
                        if (auth_login)
                          {
                              j["session"]=1;//todo следующий по порядку номер list_session.size()
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

                      if (!strcmp(command.c_str(),"message"))
                      {
                          //TODO message ANSWER

                          uint64_t session =0;

                          if (object.contains("session"))
                             session   =  object["session"];

                          json j;

                          if (object.contains("sender_login"))
                          {// Отсылка сообщения с сервера
                              string sender_login   =  object["sender_login"];


                            //todo непонятно как пересылать с сервера и как должен отвечать клиент


                          }
                          /////////////////////////////////////////////////////////
                          else// Отсылка сообщения на сервер

                          {

                              if (list_sessions.size()>session && list_sessions[session]->isAuthorization) //todo  если авторезирован ???
                              {

                                 stmessage mes;
                                  if (object.contains("body"))
                                  {

                                      mes.id=list_messages.size()+1;
                                      mes.body =  object["body"];
                                      list_messages.push_back(&mes);
                                      list_sessions[session]->id_message=mes.id;
                                  }

                                  j["client_id"]=mes.id;
                                  j["status"]="ok";
                                  j["command"]="message_reply";
                                  j["id"]=id;
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

                      if (!strcmp(command.c_str(),"ping"))
                      {
                          //TODO ping ANSWER
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
                      if (!strcmp(command.c_str(),"logout"))
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


                          //TODO logout ANSWER
                          json j;

                              j["status"]="ok";
                              j["command"]="logout_reply";
                              j["id"]=id;
                          std::cout <<"CMD logout ANSWER"<< j.dump()<< "\n";

                          memcpy(buf,j.dump().c_str(),j.dump().size());
                          nread=j.dump().size();
                      }

////////////////////////////////////////////////////////////////////

            if (nread == 0)
                break;

            // send a response
            send(client, buf, nread, 0);
        }
        close(client);
    }

    free(buf);

    for (auto cl: list_clients) {
        delete cl;
    }
    list_clients.clear();
    close(server);
}
