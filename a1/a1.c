#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


/*void listDir(char *path, int recursive, char* name_starts_with, int has_perm_execute){
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("Error open dir");
        return;
    }

    while((entry = readdir(dir)) != NULL) {

        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            if(lstat(fullPath, &statbuf) == -1) {
                perror("lstat");
                closedir(dir);
                return;
            }

            if((name_starts_with != NULL && strncmp(entry->d_name, name_starts_with, strlen(name_starts_with)) == 0) || (!(!has_perm_execute && (statbuf.st_mode & S_IXUSR)) && (statbuf.st_mode & 0100)) || (name_starts_with == NULL && has_perm_execute == 0))
                printf("%s\n", fullPath);

            if(recursive && S_ISDIR(statbuf.st_mode))
                listDir(fullPath,recursive, name_starts_with, has_perm_execute);
        }
    }

    closedir(dir);
}*/

void listDir2(char* path, int recursive, int first){
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }else{
        if(first == 0)
        {
          first = 1;
          printf("SUCCESS\n");
        }
    }
     while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            if(lstat(fullPath, &statbuf) == 0) {

                if(S_ISDIR(statbuf.st_mode)) {
                        printf("%s\n", fullPath);
                    if(recursive)
                        listDir2(fullPath, recursive, first);
                }
                else{
                    printf("%s\n", fullPath);
                }
            }
        }
    }
    closedir(dir);
}

void listDirName(char* path, int recursive, char* name_starts_with, int first){

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }else{
        if(first == 0)
        {
          first = 1;
          printf("SUCCESS\n");
        }
    }
     while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            if(lstat(fullPath, &statbuf) == 0){
                if(strncmp(entry->d_name, name_starts_with, strlen(name_starts_with)) == 0){
                    if(S_ISDIR(statbuf.st_mode))
                    {
                        if(recursive == 1)
                            listDirName(fullPath, recursive, name_starts_with, first);
                        else
                            printf("%s\n", fullPath);
                    }else{
                        printf("%s\n", fullPath);
                    }
               }
            }
        }
    }
    closedir(dir);
}

void listDirPerm(char* path, int recursive, int has_perm_execute , int first){

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }else{
        if(first == 0)
        {
          first = 1;
          printf("SUCCESS\n");
        }
    }
     while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            if(lstat(fullPath, &statbuf) == 0){
                if(statbuf.st_mode & 0100){
                    if(S_ISDIR(statbuf.st_mode))
                    {
                        if(recursive == 1)
                            listDirPerm(fullPath, recursive, has_perm_execute, first);
                        else
                            printf("%s\n", fullPath);
                    }else{
                        printf("%s\n", fullPath);
                    }
               }
            }
        }
    }
    closedir(dir);
}


void parseFile(char* path){
    int fd = -1;
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error open file");
        return;
    }

    unsigned char magic;

    if(read(fd, &magic, sizeof(unsigned char)) == -1)
    {
        perror("Error reading file");
        close(fd);
        return;
    }

    if(magic != 'D')
    {
        printf("ERROR\nwrong magic\n");
        close(fd);
        return;
    }

    unsigned short header_size, version;
    unsigned char no_of_sections;


    if(read(fd, &header_size, sizeof(unsigned short)) == -1 || read(fd, &version, sizeof(unsigned short)) == -1 || read(fd, &no_of_sections, sizeof(unsigned char)) == -1)
    {
        perror("Error reading file");
        close(fd);
        return;
    }

    if(version < 60 || version > 165)
    {
        printf("ERROR\nwrong version\n");
        close(fd);
        return;
    }

    if(no_of_sections != 2 && (no_of_sections < 7 || no_of_sections > 13))
    {
        printf("ERROR\nwrong sect_nr\n");
        close(fd);
        return;
    }

    char sect_name[18 + 1];
    unsigned short sect_type;
    unsigned int sect_offset, sect_size;

    for(int i = 0; i < no_of_sections; i++)
    {
        if(read(fd, sect_name, 18) == -1 || read(fd, &sect_type, sizeof(unsigned short)) == -1 || read(fd, &sect_offset, sizeof(unsigned int)) == -1 || read(fd, &sect_size, sizeof(unsigned int)) == -1)
        {
            perror("Error reading file");
            close(fd);
            return;
        }

        if (sect_type != 55 && sect_type != 90 && sect_type != 69 && sect_type != 73 && sect_type != 11)
        {
            printf("ERROR\nwrong sect_types\n");
            close(fd);
            return;
        }
    }

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", no_of_sections);
    lseek(fd, 0, SEEK_SET);
    lseek(fd, 6 * sizeof(unsigned char), SEEK_CUR);


    for(int i = 0; i < no_of_sections; i++)
    {
        char sect_name[18 + 1];
        unsigned short sect_type;
        unsigned int sect_offset, sect_size;

        if(read(fd, sect_name, 18) == -1 || read(fd, &sect_type, sizeof(unsigned short)) == -1 || read(fd, &sect_offset, sizeof(unsigned int)) == -1 || read(fd, &sect_size, sizeof(unsigned int)) == -1) {
            perror("Error read file");
            close(fd);
            return;
        }

        sect_name[18] = '\0';

        printf("section%d: %s %d %d\n", i + 1, sect_name, sect_type, sect_size);
    }

    close(fd);
}

void extractLine(char* path, int section, int line){


    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    unsigned char magic;
    if (read(fd, &magic, sizeof(unsigned char)) == -1) {
        perror("Error reading file");
        close(fd);
        return;
    }

    if (magic != 'D') {
        printf("ERROR\ninvalid file\n");
        close(fd);
        return;
    }

    unsigned short header_size, version;
    unsigned char no_of_sections;

    if (read(fd, &header_size, sizeof(unsigned short)) == -1 ||
        read(fd, &version, sizeof(unsigned short)) == -1 ||
        read(fd, &no_of_sections, sizeof(unsigned char)) == -1) {
        perror("Error reading file");
        close(fd);
        return;
    }

    if (version < 60 || version > 165 ||
        (no_of_sections != 2 && (no_of_sections < 7 || no_of_sections > 13))) {
        printf("ERROR\ninvalid file\n");
        close(fd);
        return;
    }

    if (section < 0 || section > no_of_sections) {
        printf("ERROR\ninvalid section\n");
        close(fd);
        return;
    }

    char sect_name[18 + 1];
    unsigned short sect_type;
    unsigned int sect_offset, sect_size;

    for(int i = 0; i < no_of_sections; i++)
    {
        if(read(fd, sect_name, 18) == -1 || read(fd, &sect_type, sizeof(unsigned short)) == -1 || read(fd, &sect_offset, sizeof(unsigned int)) == -1 || read(fd, &sect_size, sizeof(unsigned int)) == -1)
        {
            perror("Error reading file");
            close(fd);
            return;
        }

        if (sect_type != 55 && sect_type != 90 && sect_type != 69 && sect_type != 73 && sect_type != 11)
        {
            printf("ERROR\nwrong sect_types\n");
            close(fd);
            return;
        }

        if(i + 1 == section)
            break;
    }



    if (lseek(fd, sect_offset, SEEK_SET) == -1) {
        perror("Error lseek 2");
        close(fd);
        return;
    }



    int nrline = 0;
    int lung = 0;
    unsigned char ch;
    int size_total = 0;
    int last = 0;

    while (nrline <line && size_total <= sect_size) {
        lung = 0;
        while(read(fd, &ch, sizeof(unsigned char)) > 0 && ch != 0x0a) {
            size_total++;
            if(size_total >= sect_size)
            {
                last = 1;
                break;
            }
            lung++;
        }
        if(ch == 0x0a)
            size_total++;

        nrline++;
    }

    if (nrline != line) {
        printf("ERROR\ninvalid line\n");
        close(fd);
        return;
    } else {
        if(last == 1)
            printf("SUCCESS\n");
        else
            printf("SUCCESS");
        lseek(fd, -1, SEEK_CUR);
        if (read(fd, &ch, sizeof(unsigned char)) == -1) {
            perror("Error reading file");
            close(fd);
            return;
        }
        printf("%c", ch);
        lseek(fd, -2, SEEK_CUR);
        while (read(fd, &ch, sizeof(unsigned char)) > 0 && lung > 0) {
            printf("%c", ch);
            lung--;
            if (lseek(fd, -2, SEEK_CUR) == -1) {
                close(fd);
                return;
            }
        }
    }
    printf("\n");
    close(fd);

}
int validSF(char* path){

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 0;
    }

    unsigned char magic;
    if (read(fd, &magic, sizeof(unsigned char)) == -1) {
        perror("Error reading file");
        close(fd);
        return 0;
    }

    if (magic != 'D') {
        close(fd);
        return 0;
    }

    unsigned short header_size, version;
    unsigned char no_of_sections;

    if(read(fd, &header_size, sizeof(unsigned short)) == -1 || read(fd, &version, sizeof(unsigned short)) == -1 || read(fd, &no_of_sections, sizeof(unsigned char)) == -1)
    {
        perror("Error reading file");
        close(fd);
        return 0;
    }

    if(version < 60 || version > 165)
    {
        close(fd);
        return 0;
    }

    if(no_of_sections != 2 && (no_of_sections < 7 || no_of_sections > 13))
    {
        close(fd);
        return 0;
    }

    int sect_69 = 0;
    char sect_name[18 + 1];
    unsigned short sect_type;
    unsigned int sect_offset, sect_size;

    for(int i = 0; i < no_of_sections; i++)
    {
        if(read(fd, sect_name, 18) == -1 || read(fd, &sect_type, sizeof(unsigned short)) == -1 || read(fd, &sect_offset, sizeof(unsigned int)) == -1 || read(fd, &sect_size, sizeof(unsigned int)) == -1)
        {
            perror("Error reading file");
            close(fd);
            return 0;
        }

        if (sect_type != 55 && sect_type != 90 && sect_type != 69 && sect_type != 73 && sect_type != 11)
        {
            close(fd);
            return 0;
        }

        if(sect_type == 69){
            sect_69++;
            if(sect_69 >= 2)
            {
                close(fd);
                return 1;
            }
        }


    }

    close(fd);
    return 0;

}

void findSF(char *path, int first) {

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("Could not open directory");
        return;
    }else{
        if(first == 0)
        {
          first = 1;
          printf("SUCCESS\n");
        }
    }
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(S_ISDIR(statbuf.st_mode)) {
                    findSF(fullPath, first);
                }
                else if(S_ISREG(statbuf.st_mode)){
                    if(validSF(fullPath))
                    {
                        printf("%s\n", fullPath);
                    }
                }
            }
        }
    }
    closedir(dir);

}

int main(int argc, char **argv)
{

    int recursive = 0;
    char* name_starts_with = NULL;
    int has_perm_execute = 0;
    char* path;
    int list = 0;
    int parse = 0;
    int section = -1;
    int line = -1;
    int extract = 0;
    int findall = 0;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[1], "variant") == 0) {
            printf("14043\n");
            return 0;
        }
        else if(strcmp(argv[i], "recursive") == 0){
            recursive = 1;
        }
        else if(strncmp(argv[i], "name_starts_with=", 17) == 0){
            name_starts_with = argv[i] + 17;
        }
        else if(strcmp(argv[i], "has_perm_execute") == 0){
            has_perm_execute = 1;
        }
        else if(strncmp(argv[i], "path=", 5) == 0){
            path = argv[i] + 5;
        }
        else if(strcmp(argv[1], "list") == 0){
            list = 1;
        }
        else if(strcmp(argv[1], "parse") == 0){
            parse = 1;
        }
        else if(strncmp(argv[i], "section=", 8) == 0){
            if(sscanf(argv[i] + 8,"%d", &section) != 1)
                return -1;
        }
        else if(strncmp(argv[i], "line=", 5) == 0){
            if(sscanf(argv[i] + 5,"%d", &line) != 1)
                return -1;
        }
        else if(strcmp(argv[1], "extract") == 0){
            extract = 1;
        }
        else if(strcmp(argv[i], "findall") == 0){
            findall = 1;
        }
    }

    if(list == 1 && name_starts_with == NULL && has_perm_execute == 0){
        listDir2(path, recursive, 0);
    }
    else if(parse == 1){
        parseFile(path);
    }
    else if(extract == 1){
        extractLine(path, section, line);
    }
    else if(findall == 1){
        findSF(path, 0);
    }
    else if(list == 1 && name_starts_with != NULL)
    {
        listDirName(path, recursive, name_starts_with, 0);
    }
    else if(list == 1 && has_perm_execute == 1)
    {
        listDirPerm(path, recursive, has_perm_execute, 0);
    }

    return 0;
}


