#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <pthread.h>


using namespace std;
//подключаем json
#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <cstring>
#include <fstream>
//функции записи/чтения через сокет для вывода в поток
#define BUF_SIZE 1024
void *write(void *);
void *read(void *);
 int sock=0;
void *write (void *dummyPt)
{
    for(;;)
       {
           char s[BUF_SIZE];

           cout << "<-";
           bzero(s, BUF_SIZE + 1);
           cin.getline(s, BUF_SIZE);
           ///////////////////////////
           string line=s;
           char* buf_cmd = (char*) malloc(BUF_SIZE+1);

            memset(buf_cmd,0,BUF_SIZE+1);
           uint64_t id=0;
           id++;
            int nwrite=0;
           //-----------------------------------------------
           if (!strcmp((const char*)line.c_str(),"HELLO"))
           {
               json object;
               object["command"]="HELLO";
               object["id"]=id;
               memcpy(buf_cmd,object.dump().c_str(),object.dump().size());
               nwrite=object.dump().size();
           }
           else if (!strcmp((const char*)line.c_str(),"login"))
           {
                json object;
                object["command"]="login";
                string login;
                cout << "Enter your login:" << endl;
                getline(cin,login);
                object["login"]=login.c_str();

                string passwd;
                cout << "Enter passwd:" << endl;
                getline(cin,passwd);
                object["passwd"]=passwd;


                object["id"]=id;
                memcpy(buf_cmd,object.dump().c_str(),object.dump().size());
                nwrite=object.dump().size();
           }
           else if (!strcmp((const char*)line.c_str(),"login"))
           {
                json object;
                object["command"]="login";
                string login;
                cout << "Enter your login:" << endl;
                getline(cin,login);
                object["login"]=login.c_str();

                string passwd;
                cout << "Enter passwd:" << endl;
                getline(cin,passwd);
                object["passwd"]=passwd;


                object["id"]=id;
                memcpy(buf_cmd,object.dump().c_str(),object.dump().size());
                nwrite=object.dump().size();
           }
           else if (!strcmp((const char*)line.c_str(),"message"))
           {
               json object;
               string session;

               cout << "Enter id session for send message:" << endl;
               getline(cin,session);
               if (isdigit(session.c_str()[0])) //если первый сисмвол число преобразуем
               {
                   object["session"]=atoi(session.c_str());
               } else {
                   cout << "Error" << endl;
               }

               string body;
               cout << "Enter text message:" << endl;
               getline(cin,body);

                object["body"]=body;

               string send_login;
               cout << "Enter sending login:" << endl;
               getline(cin,send_login);

               if (send_login.size()>0)
                        object["send_login"]=send_login.c_str();

               object["command"]="message";
               object["id"]=id;
               memcpy(buf_cmd,object.dump().c_str(),object.dump().size());
               nwrite=object.dump().size();
           }

           else if (!strcmp((const char*)line.c_str(),"ping"))
           {
               json object;
               string session;
               cout << "Enter id session for ping:" << endl;
               getline(cin,session);
               if (isdigit(session.c_str()[0])) //если первый сисмвол число преобразуем
               {
                   object["session"]=atoi(session.c_str());
               } else {
                   cout << "Error" << endl;
               }
               object["command"]="ping";
               object["id"]=id;
               memcpy(buf_cmd,object.dump().c_str(),object.dump().size());
               nwrite=object.dump().size();
           }
           else if (!strcmp((const char*)line.c_str(),"logout"))
           {
               json object;
               string session;
               cout << "Enter id session for logout:" << endl;
               getline(cin,session);
               if (isdigit(session.c_str()[0])) //если первый сисмвол число преобразуем
               {
                   object["session"]=atoi(session.c_str());
               } else {
                   cout << "Error" << endl;
               }
               object["command"]="logout";
               object["id"]=id;
               memcpy(buf_cmd,object.dump().c_str(),object.dump().size());
               nwrite=object.dump().size();
           }

            {
               FILE * ptrFile = fopen ( "/tmp/file.txt" , "wb" );

               fwrite(buf_cmd , 1 , nwrite+1 , ptrFile ); // записать в файл содержимое буфера
               fclose (ptrFile);
           }

            ////////отправка//////////
           send(sock, buf_cmd, nwrite+1, 0);
           if (buf_cmd)
               free(buf_cmd);

       }
    close(sock);

}

void *read (void *dummyPt)
{
    char test[BUF_SIZE];
    bzero(test, BUF_SIZE + 1);
    bool loop = false;

    while(!loop)
    {
        bzero(test, BUF_SIZE + 1);
        int rc = read(sock, test, BUF_SIZE);
        if ( rc > 0)
        {
        string tester (test);
        cout << "\r\nmessage-> "<< tester << endl;

        if(tester == "exit_server")
            break;
        }

    }
    cout << "\nClosing thread and conn" << endl;
    close(sock);
    return 0;
}




//Client side
int main(int argc, char *argv[])
{
    //we need 2 things: ip address and port number, in that order
    
    if(argc != 3)
    {
        cerr << "Usage: ip_address port" << endl; exit(0); 
    } //grab the IP address and port number 

    char *serverIp = argv[1]; int port = atoi(argv[2]);

//     char *serverIp ="localhost"; int port = 3000;
    //create a message buffer
    char msg[1025];
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(sock,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;
    }
    cout << "Connected to the server!" << endl;


    int bytesRead, bytesWritten = 0;
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);

    //разводим чтение и запись по потокам

    pthread_t tid1,tid2; /* идентификатор потока */
    pthread_attr_t attr1,attr2; /* отрибуты потока */
      pthread_attr_init(&attr1);
    pthread_create(&tid1, &attr1, write, NULL);
    pthread_attr_init(&attr2);
   pthread_create(&tid2, &attr2, read, NULL);
   pthread_join(tid1, NULL);
   pthread_join(tid2, NULL);

    gettimeofday(&end1, NULL);
    close(sock);
    return 0;    
}
