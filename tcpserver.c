#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <mhash.h>

#define MAX_PENDING 5
#define MAX_LINE 1024

int main(int argc, char * argv[])
{
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	char buf_to_client[MAX_LINE];
	char filename[100];
	short int file_size;
	int len;
	int bytes, bytesSent;
	int s, new_s;
	int opt = 1;

	/* Check number of arguments and retrieve port number */
	if (argc!=2) {
	 	fprintf(stderr, "usage: port is not specified\n");
	  	exit(1);
	}

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(atoi(argv[1]));

	/* setup passive open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}

	// set socket option
	if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(int)))<0){
		perror ("simplex-talk:setscokt");
		exit(1);
	}
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind"); exit(1);
	}
	if ((listen(s, MAX_PENDING))<0){
		perror("simplex-talk: listen"); exit(1);
	}

	bzero((char*)&buf, sizeof(buf));
	bzero((char*)&buf_to_client, sizeof(buf_to_client));

	/* wait for connection, then receive and print text */
	while(1) {
		len = sizeof(sin);
		if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
			perror("simplex-talk: accept");
			exit(1);
		}

		while (1){

			/* Receive file name */
			if((len=recv(new_s, buf, sizeof(buf), 0))==-1){
				perror("Server Received Error!");
				exit(1);
			}
			if (len==0) break;

			strcpy(buf_to_client, buf);
			int len = strlen(buf_to_client);
			strcpy(filename, buf_to_client);
			printf("%s\n", filename);

			FILE* file = fopen(filename,"rb");

			/* If the file does not exist, send -1 */
			if(file == NULL)
			{
				printf("File does not exist\n");
				char errorNumber[3];
				strcpy(errorNumber, "-1");
				if(send(new_s, errorNumber, strlen(errorNumber) + 1, 0)==-1){
					perror("server send error!"); exit(1);
				}
			}
			else{ /* If it exists, send its size */

				fseek(file, 0, SEEK_END);
			 	long int size = ftell(file);
			 	printf("file size: %ld\n", size);
			 	char fileSizeStr[30];
			 	// printf("file size: %ld\n", size);
			 	sprintf(fileSizeStr, "%ld", size);
				rewind(file);

				fread(buf_to_client, 1, MAX_LINE, file);
			printf("1\n");
				if(send(new_s, fileSizeStr, strlen(fileSizeStr) + 1, 0)==-1){
					perror("server send error!"); exit(1);
				}
			printf("2\n");

				usleep(10000);

				/* Send File to client after 0.1s */
				bytesSent = 0;
				int nit = 0;
				while( bytesSent < size){
					fread(buf_to_client, 1, MAX_LINE, file);
					printf("%d\n", nit);

					bytes = send(new_s, buf_to_client, size, 0);
					if(bytes == -1){
						perror("server send error!"); exit(1);
					} else {
						bytesSent += bytes;
						printf("%d\n", bytesSent);
					}
					nit++;
				}

				/* Compute hash value */
				FILE* file2 = fopen(filename, "r");
				int i;
				MHASH td;
				unsigned char *hashbuf;
				unsigned char *hash;
				fseek(file2, 0, SEEK_END);
				size = ftell(file2);
				rewind(file2);

				hashbuf=calloc(size+1,1);
				td=mhash_init(MHASH_MD5);
				if(td==MHASH_FAILED){
					printf("Error: Hash fail\n");
					exit(1);
				}
				fread(hashbuf,1,size,file2);
				mhash(td,hashbuf,size);
				hash=mhash_end(td);
				char hashstr[32];

				/* Send hash value */
				usleep(100000);

				if(send(new_s,hash,16,0) == -1){
					perror("server send error!\n");
				}

				bzero((char*)hashbuf,sizeof(hashbuf));
				bzero((char*)hash,sizeof(hash));
			}

			bzero((char*)&buf, sizeof(buf));
			bzero((char*)&buf_to_client, sizeof(buf_to_client));
		}

		// printf("Client finishes, close the connection!\n"); //DEBUG
		close(new_s);
	}
}
