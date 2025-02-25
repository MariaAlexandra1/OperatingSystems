#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define FIFO_NAME_1 "RESP_PIPE_14043"
#define FIFO_NAME_2 "REQ_PIPE_14043"

char len1 = 7;

char len_req;
int mapFD;
int shmFD;
char* sharedMem;
char* sharedFile;
unsigned int dimensiune;
unsigned int offset, value;
unsigned int offset2, nrOfBytes;
unsigned int offset3, nrOfBytes2, nrSec;
ssize_t dim;

void send_res(int fd_resp, const char* mess) {
    char len = strlen(mess);
    write(fd_resp, &len, sizeof(char));
    write(fd_resp, mess, len * sizeof(char));
}

void variant_resp(int fd_resp){
    unsigned int nr1 = 14043;
    send_res(fd_resp, "VARIANT");
    write(fd_resp, &nr1, sizeof(unsigned int));
    send_res(fd_resp, "VALUE");
}

void create_shm_resp(int fd_req, int fd_resp){
    read(fd_req, &dimensiune, sizeof(unsigned int));
    send_res(fd_resp, "CREATE_SHM");

    shmFD = shm_open("/4r2VLs", O_CREAT | O_RDWR, 0664);
    if(shmFD < 0){
        send_res(fd_resp, "ERROR");
        perror("ERROR create 1");
        return;
    }

    ftruncate(shmFD, dimensiune * sizeof(char));
    sharedMem = (char*)mmap(0, sizeof(char) * dimensiune, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(sharedMem == (void*)-1) {
        send_res(fd_resp, "ERROR");
        close(shmFD);
        perror("ERROR create 2");
        return;
    }

    send_res(fd_resp, "SUCCESS");
}

void write_shm_resp(int fd_req, int fd_resp){

    read(fd_req, &offset, sizeof(unsigned int));
    read(fd_req, &value, sizeof(unsigned int));

    send_res(fd_resp, "WRITE_TO_SHM");

    if(offset >= dimensiune || offset < 0){
        send_res(fd_resp, "ERROR");
        return;
    }
    if(offset + sizeof(value) > dimensiune){
        send_res(fd_resp, "ERROR");
        return;
    }
    *((unsigned int *)(sharedMem + offset)) = value;


    send_res(fd_resp, "SUCCESS");
}

void map_file_resp(int fd_req, int fd_resp){
    char len5;
    read(fd_req, &len5, sizeof(char));

    int len = len5;
    char* file_name = (char*)malloc((len + 1) * sizeof(char));


    read(fd_req, file_name, len5 * sizeof(char));

    file_name[len] = '\0';

    send_res(fd_resp, "MAP_FILE");

    mapFD = open(file_name, O_RDONLY);
    if(mapFD == -1){
        send_res(fd_resp, "ERROR");
        return;
    }

    dim = lseek(mapFD, 0, SEEK_END);
    lseek(mapFD, 0, SEEK_SET);

    sharedFile = ( char*)mmap(0, sizeof(char) * dim, PROT_READ, MAP_SHARED, mapFD, 0);
    if(sharedFile == (void*)-1){
        send_res(fd_resp, "ERROR");
        free(file_name);
        close(mapFD);
        perror("ERROR mapfile");
        return;
    }

    send_res(fd_resp, "SUCCESS");
    free(file_name);
}

void read_offset(int fd_req, int fd_resp){

    read(fd_req, &offset2, sizeof(unsigned int));
    read(fd_req, &nrOfBytes, sizeof(unsigned int));

    send_res(fd_resp, "READ_FROM_FILE_OFFSET");

    if(sharedMem == (void*)-1 || sharedFile == (void*)-1){
        send_res(fd_resp, "ERROR");
        return;
    }
    if(offset2 + nrOfBytes > dim){
        send_res(fd_resp, "ERROR");
        return;
    }

    unsigned int final = offset2 + nrOfBytes;
    int memI = 0;
    for(unsigned int i = offset2; i <= final; i++)
    {
        sharedMem[memI] = sharedFile[i];
        memI++;
    }

    send_res(fd_resp, "SUCCESS");
}

void read_section(int fd_req, int fd_resp){

    read(fd_req, &nrSec, sizeof(unsigned int));
    read(fd_req, &offset3, sizeof(unsigned int));
    read(fd_req, &nrOfBytes2, sizeof(unsigned int));

    send_res(fd_resp, "READ_FROM_FILE_SECTION");
    if (sharedMem == (void*)-1 || sharedFile == (void*)-1) 				{
        send_res(fd_resp, "ERROR");
        return;
    }

    unsigned char no_of_sections = sharedFile[6];
    if (nrSec < 1 || nrSec > no_of_sections) {
        send_res(fd_resp, "ERROR");
        return;
    }


    unsigned int section_offset = (unsigned int)(sharedFile[11 + 24 * (nrSec - 1) + 20]);
    unsigned int section_size = (unsigned int)(sharedFile[11 + 24 * (nrSec - 1) + 24]);


    if(offset3 + nrOfBytes2 > section_size) {
        send_res(fd_resp, "ERROR");
        return;
    }


    for(unsigned int i = 0; i < nrOfBytes2; i++) {
        ((unsigned char*)sharedMem)[i] = ((unsigned char*)sharedFile)[section_offset + offset3 + i];
    }

    send_res(fd_resp, "SUCCESS");

}

int main(void){
    int fd_req = -1;
    int fd_resp = -1;

    if(mkfifo(FIFO_NAME_1, 0600) != 0){
        perror("mkfifo");
        return 1;
    }

    fd_req = open(FIFO_NAME_2, O_RDONLY);
    if(fd_req == -1) {
        perror("error open fdR");
        return 1;
    }

    fd_resp = open(FIFO_NAME_1, O_WRONLY);
    if(fd_resp == -1) {
        perror("error open fdW");
        return 1;
    }

    if(write(fd_resp, &len1, sizeof(char)) == -1){
        perror("write 1");
    }

    write(fd_resp, "CONNECT", 7 * sizeof(char));

    char* comanda = (char*)malloc(255 * sizeof(char));
    if (comanda == NULL) {
        perror("malloc");
        return 1;
    }

    for(;;) {
        if(read(fd_req, &len_req, sizeof(char)) == -1){
            perror("read 1");
            return 1;
        }

        if(read(fd_req, comanda, len_req * sizeof(char)) == -1){
            perror("read 2");
            return 1;
        }
      	comanda[(int)len_req] = '\0';

        if(strcmp(comanda, "VARIANT") == 0){
            variant_resp(fd_resp);
        }
        else if(strcmp(comanda, "CREATE_SHM") == 0){
            create_shm_resp(fd_req, fd_resp);
        }
        else if(strcmp(comanda, "WRITE_TO_SHM") == 0){
            write_shm_resp(fd_req, fd_resp);
        }
        else if(strcmp(comanda, "MAP_FILE") == 0){
            map_file_resp(fd_req, fd_resp);
        }
        else if(strcmp(comanda, "READ_FROM_FILE_OFFSET") == 0){
            read_offset(fd_req, fd_resp);
        }
        else if(strcmp(comanda, "READ_FROM_FILE_SECTION") == 0){
            read_section(fd_req, fd_resp);
        }
        else if(strcmp(comanda, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0){
            break;
        }
        else if(strcmp(comanda, "EXIT") == 0) {
            free(comanda);
            close(fd_req);
            close(fd_resp);
            unlink(FIFO_NAME_1);
            return 0;
        }
    }

    return 0;
}
