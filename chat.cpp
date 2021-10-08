/*******************************************
* Group Name  : XXXXXX

* Member1 Name: XXXXXX
* Member1 SIS ID: XXXXXX
* Member1 Login ID: XXXXXX

* Member2 Name: XXXXXX
* Member2 SIS ID: XXXXXX
* Member2 Login ID: XXXXXX
********************************************/

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int sanityCheckPort(const std::string&);

int main(int argc, char* argv[]){
    
    int c;
    int count = 0;
    
    char* ip;
    int port;
    
    void helpMenu();
    struct sockaddr_in sa;
    
    while ((c = getopt(argc, argv, "Hhp:s:")) != -1)
    {
        if (c == 'h' || c == 'H') 
        {
            helpMenu();
            return 0;
        }
        if (c == 'p')
        {
            port = sanityCheckPort(optarg);
            count++;
        }
        if (c == 's')
        {
            ip = optarg;
            count++;
        }
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (count == 0) //no arguments, run Server setup
    { 
        //Wait for connection here
        struct addrinfo hints, *servinfo;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; //"By using the AI_PASSIVE flag, I’m telling the program to bind to the IP of the host it’s running on." -Beej
        int rv;
        
        //No address since AI_PASSIVE flag
        if((rv = getaddrinfo(NULL, "19993", &hints, &servinfo)) != 0)
        {
            std::cout << "Error getting addres info" << std::endl;
        }
        
        void *addr;
        char ipstr[32];
    
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
        addr = &(ipv4->sin_addr);
        
        //attempt to bind
        rv = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if (rv == -1) 
        { 
            close(sockfd);
            std::cout << "Binding error" << std::endl; 
            exit(1); 
        }
        
        inet_ntop(servinfo->ai_family, addr, ipstr, sizeof ipstr);
        freeaddrinfo(servinfo);
        
        //How do we know which IP/Port to listen on? Maybe hard code the port, what about IP?
        std::cout << "Welcome to Chat!\nWaiting for a connection on " << ipstr << " port " << "19993" << std::endl; 
    }
    else if (count == 2)//Arguments present, run Client setup
    { 
        // Establish Connection here
        std::cout << "Client" << std::endl;
        
        //add IP to sockaddr_in struct. Will return 0 or -1 if the address is invalid
        int addrStatus = inet_pton(AF_INET, ip, &(sa.sin_addr));
        if (addrStatus < 1)
        {
            std::cout << "Invalid IP address" << std::endl;
            exit(1);
        }
        sa.sin_port = port;
    }
    else //incorrect number of arguments
    {
        helpMenu();
    }
    
    //Client sends message first, then waits for message from other (140 char max)
//     while()
//     {
//         
//          IMPORTANT! convert the message to network byte order before sending, and then convert back to host byte order after receiving
//
//     }
    
    return 0;
}

int sanityCheckPort(const std::string& arg)
{
    int port;
    try 
    {
        port = std::stoi(arg);
    }     
    catch (const std::exception& e)
    {
        std::cout << "'" << arg << "' is not a valid port" << std::endl;
        exit(1);
    }

    if (port < 1 || port > 65535)
    {
        std::cout << "Port '"<< port << "' out of range" << std::endl;
        exit(1);
    }
    
    return port;
}

void helpMenu() {
    std::cout << "\nWelcome to Chat Help" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout <<  "To run SERVER use command: ./chat" << std::endl;
    std::cout << "To run CLIENT use command:" << std::endl;
    std::cout << "\t- ./chat -p [port #] -s [IP address]" << std::endl;
    std::cout << "\t  OR" << std::endl;
    std::cout << "\t- ./chat -s [IP address] -p [port #]\n" << std::endl;
            
    std::cout << "Sent MESSAGE LENGTH may be no longer than 140 characters.\n" << std::endl;
            
    std::cout << "To EXIT Chat - control+c \n" << std::endl;
}
