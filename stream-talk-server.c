
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/uio.h>
#include <string.h>
#include <netinet/in.h>

#define BACKLOG 10

int main(int argc, char *argv[])
{
    struct addrinfo hints, *rp, *result;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    
    
    
    int success, new_success;
    
    if ((success = getaddrinfo(NULL, argv[1], &hints, &result)) != 0 ){
        fprintf(stderr, "%s: getaddrinfo: %s\n", argv[0],
                gai_strerror(success));
        exit(1);
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        if ((success = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol))
            == -1 ){
            perror("server: socket");
            continue;
        }
        
        int optval = 1;
        
        if(setsockopt(success, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))
           == -1){
            printf("setsocketopt \n");
            exit(1);
        }
        
        if (!bind(success, rp->ai_addr, rp->ai_addrlen))
            break;
        
        
        perror("server: bind");
        close(success);
        
    }
    
    freeaddrinfo(result);
    
    if (rp == NULL){
        perror("server: bind");
        exit(1);
    }
    
    if (listen(success, BACKLOG) == -1){
        perror("server: listen");
        close(success);
        exit(1);
    }
    
    while(1){
        new_success = accept(success, rp->ai_addr, &(rp->ai_addrlen));
        
        if(new_success == -1){
            perror("server: accept");
            close(success);
            exit(1);
        }
        
        char buffer[256] = {'\0'};
        
        recv(new_success, buffer, sizeof(buffer), 0);
        char *client_req_file = buffer;
        int file_name = open(client_req_file, O_RDONLY);
        
        if(file_name == -1){
            char err[2] = "-1";
            
            send(new_success, err, sizeof(err), 0);
            
            close(new_success);
            close(success);
            exit(1);
        }
        
        int got;
        
        while((got = read(file_name, &buffer, 255))){
            send(new_success, &buffer, got, 0);
            memset(buffer, 0, sizeof(buffer));
        }
        
        close(new_success);
        close(file_name);
        
        break;
        
    }
    
    close(success);
    
    return 0;
}
