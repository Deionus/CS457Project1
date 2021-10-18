/*******************************************
* Group Name  : Web
* 
* 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 
* 
* Member1 Name: Deionus Bauer
* Member1 SIS ID: 831-959-164
* Member1 Login ID: deionus
* 
* Member2 Name: Meridith McCann
* Member2 SIS ID: 832-827-122
* Member2 Login ID: mjmccann
********************************************/

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <errno.h>
#include <limits>
#include <ifaddrs.h>

int sanityCheckPort(const std::string&);
void helpMenu();
bool sendMessage(int);

int main(int argc, char* argv[]){
    
    int c;
    int count = 0;
    
    char* ip;
    int port;
    
    char* portarg;
    
    int16_t version = 457;
    int16_t msg_len;
    
    while ((c = getopt(argc, argv, "Hhp:s:")) != -1)
    {
        if (c == 'h' || c == 'H') 
        {
            helpMenu();
            return 0;
        }
        if (c == 'p')
        {
            portarg = optarg;
            count++;
        }
        if (c == 's')
        {
            ip = optarg;
            count++;
        }
    }
    
    int sockfd;
    
    if (count == 0) //no arguments, run Server setup
    { 
        
        char interfaceName[] = "eno1";
        struct ifaddrs *addrs;
        getifaddrs(&addrs);
        
        struct sockaddr_in *interfaceInfo;
        interfaceInfo = (struct sockaddr_in *) addrs->ifa_addr;
        while((addrs = addrs->ifa_next) != NULL)
        {
            interfaceInfo = (struct sockaddr_in *) addrs->ifa_addr;
            if (interfaceInfo->sin_family == AF_INET && std::strcmp(interfaceName,addrs->ifa_name) == 0)
            {
                std::cout << addrs->ifa_name << ":" 
                << inet_ntoa(interfaceInfo->sin_addr) << ":"
                << std::endl;
                break;
            }
        }
        
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        //Wait for connection here
        struct sockaddr_storage connection_addr;
        socklen_t sin_size;
        char s[INET6_ADDRSTRLEN];
        int rv;
        
        void *addr;
        char ipstr[32];
    
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(19993);
        serverAddr.sin_addr = interfaceInfo->sin_addr;
        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        
        addr=&serverAddr.sin_addr;
        
        //attempt to bind
        rv = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof serverAddr);
        if (rv == -1) 
        { 
            close(sockfd);
            std::cout << "Binding error "<< errno << std::endl; 
            exit(1); 
        }
        
        inet_ntop(AF_INET, addr, ipstr, sizeof ipstr);
        
        //How do we know which IP/Port to listen on? Maybe hard code the port, what about IP?
        rv = listen(sockfd, 10); //listen on socket socketfd, backlog of 10
        if (rv == -1)
        {
            close(sockfd);
            std::cout << "Listening error" << std::endl;
            exit(1);
        }
        std::cout << "Welcome to Chat!\nWaiting for a connection on " << ipstr << " port " << "19993" << std::endl; 
        
        sin_size = sizeof connection_addr;
        int connection_fd = accept(sockfd, (struct sockaddr *)&connection_addr, &sin_size); //Accept the incoming connection
        if (connection_fd == -1)
        {
            close(sockfd);
            close(connection_fd);
            std::cout << "Error accepting connection" << std::endl;
            exit(1);
        }
        
        inet_ntop(connection_addr.ss_family, &(((struct sockaddr_in*)&connection_addr)->sin_addr), s, sizeof s);
        sockfd = connection_fd;
        std::cout << "Found a friend! You receive first." << std::endl;
        
        //Code now falls down to the while loop
    }
    else if (count == 2)//Arguments present, run Client setup
    { 
        port = sanityCheckPort(portarg);
        
        // Establish Connection here
        std::cout << "Connecting to server...";
        
        struct addrinfo hints, *servinfo;
        int rv;
        char s[INET6_ADDRSTRLEN];
        
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        std::string sport = std::to_string(port);
        char const *pchar = sport.c_str();
        //add IP to sockaddr_in struct. Will return 0 or -1 if the address is invalid
        rv = getaddrinfo(ip, pchar, &hints, &servinfo);
        if (rv != 0)
        {
            std::cout << "Invalid IP address" << std::endl;
            exit(1);
        }
        
        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if (sockfd == -1) 
        {
            std::cout << "Socket Error" << std::endl;
            close(sockfd);
            exit(1);
        }
        
        rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if (rv == -1)
        {
            close(sockfd);
            std::cout << "Connection Error" << std::endl;
            exit(1);
        }
        
        inet_ntop(servinfo->ai_family, &(((struct sockaddr_in*)&servinfo)->sin_addr), s, sizeof s);
        std::cout << " Connected!" << std::endl;
        std::cout << "Connected to a friend! You send first." << std::endl;

        freeaddrinfo(servinfo);
        
        //Send Message
        
        while (!sendMessage(sockfd));

    }
    else //incorrect number of arguments
    {
        helpMenu();
    }
    
//  Wait for a message, collect/display it, then send a message
    while(true)
    {
        int rv;
        char buf[141];
        while((rv = recv(sockfd, buf, 140, 0)) < 1)
        {
            //either -1 (error) or 0 length datagram
            if (rv == -1)
            {
                std::cout << "Error receiving message. Exiting" << std::endl;
                close(sockfd);
                std::cout << errno << std::endl;
                exit(1);
            }
        }
        
        buf[rv] = '\0';
        std::cout << "Friend: " << buf << std::endl;
        //IMPORTANT! convert the message to network byte order before sending, and then convert back to host byte order after receiving

        while (!sendMessage(sockfd));
    }
    
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
    
    exit(0);
}

bool sendMessage(int sockfd) {
    std::string msg;
    
    std::cout << "You: ";
    
//    std::cin.clear();
//    std::cin.ignore(10000, '\n');
//    std::cin.ignore(std::numeric_limits<std::streamsize>::max());

    std::getline(std::cin,msg,'\n');
    
    if (msg.length() > 140)
    {
        std::cout << "Message not sent - messages must be 140 characters or less" << std::endl;
        return false;
    }
    
    //stuff message in a buffer
    const char* buf = msg.c_str();
    
    send(sockfd, buf, msg.length(), 0);
    
    return true;
}
