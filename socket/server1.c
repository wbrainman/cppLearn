#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>


#define BUFFER_SIZE 2000
#define MAX_INTERFACE 16

static struct sockaddr_in*  get_if_info(int fd);
static void  show_if_info(int fd, int if_num, struct ifreq* pbuf);
static void  get_if_status(unsigned int flag);


struct ifreq buf[MAX_INTERFACE];


static struct sockaddr_in*  get_if_info(int fd)
{
    int if_num = 0;
    struct ifconf ifc;
    struct ifreq ifr;
    
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;

    if(ioctl(fd, SIOCGIFCONF, (char*)&ifc)) {
        perror("get if config"); 
    }

    if_num = ifc.ifc_len/sizeof(struct ifreq);

    printf("interface num = %d\n\n",if_num);

    show_if_info(fd, if_num, buf);

    while(if_num --) {

        if(ioctl(fd, SIOCGIFADDR, &buf[if_num]) < 0) { 
            perror("ioctl");
        }

        if(htonl(INADDR_LOOPBACK) == ((struct sockaddr_in*)&(buf[if_num].ifr_addr))->sin_addr.s_addr) {
            continue; 
        }

        /*get network interface address*/
        printf("ip type: %d\n\n", ((struct sockaddr_in*)&buf[if_num].ifr_addr)->sin_family);
        printf("ip addr: %s\n\n", inet_ntoa(((struct sockaddr_in*)&(buf[if_num].ifr_addr))->sin_addr));

        return (struct sockaddr_in*)&(buf[if_num].ifr_addr);
    }

}

static void  get_if_status(unsigned int flag)
{
    if(flag & IFF_UP) {
        printf("is up\n");
    }

    if(flag & IFF_BROADCAST) {
        printf("is broadcast\n");
    }

    if(flag & IFF_LOOPBACK) {
        printf("is loop back\n");
    }

    if(flag & IFF_POINTOPOINT) {
        printf("is point to point\n");
    }

    if(flag & IFF_RUNNING) {
        printf("is running\n");
    }

    if(flag & IFF_PROMISC) {
        printf("is promisc\n");
    }
}

static void  show_if_info(int fd, int if_num, struct ifreq* pbuf)
{
    while(if_num --) {

        printf("net dev %d name = %s\n",if_num, pbuf[if_num].ifr_name);

        if(ioctl(fd, SIOCGIFADDR, &pbuf[if_num]) < 0) { 
            perror("ioctl");
        }
        /*get network interface status*/
        get_if_status(pbuf[if_num].ifr_flags);

        /*get network interface address*/
        printf("ip type: %d\n", ((struct sockaddr_in*)&pbuf[if_num].ifr_addr)->sin_family);
        printf("ip addr: %s\n", inet_ntoa(((struct sockaddr_in*)&(pbuf[if_num].ifr_addr))->sin_addr));
        printf("\n\n");
    }

}

int main()
{

    int server_sockfd, client_sockfd;
    int server_len, client_len;
	struct sockaddr_in *pServer_addr;
	struct sockaddr_in client_address;

	char data[BUFFER_SIZE] = {0};
	unsigned int n;

    //2. Remove any old sockets and create an unnamed socket for the server:

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //printf("if num %d\n",get_if_info(server_sockfd);
    pServer_addr = get_if_info(server_sockfd);

    

    //3. Name the socket:
	//pServer_addr->sin_family = AF_INET;
	//pServer_addr->sin_addr.s_addr = inet_addr("192.168.0.106");
	pServer_addr->sin_port = htons(9734);

    server_len = sizeof(struct sockaddr_in);

    bind(server_sockfd, (struct sockaddr *)pServer_addr, server_len);

    //4. Create a connection queue and wait for clients:
    listen(server_sockfd, 5);

    while(1) {

        printf("server waiting\n");

        //5. Accept a connection:
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
		printf("client info:\n");
		printf("	type: %d\n",client_address.sin_family);
		printf("	port: %d\n",ntohs(client_address.sin_port));
		printf("	addr: %s\n",inet_ntoa(client_address.sin_addr));

		if(fork() == 0) {
            printf("in child process\n");
			if(recv(client_sockfd, data, BUFFER_SIZE, 0) > 0) {
                printf("recv ID : %s\n",data);

            }
#if 0
			while(n = read(STDIN_FILENO, data, BUFFER_SIZE) > 0) {
					
				send(client_sockfd, data, strlen(data), 0);
				memset(data, 0,BUFFER_SIZE);
			}
			close(client_sockfd);
#endif
		}
		else {
            printf("in main process\n");
		}

    }

}

















