// Ben Eli 319086435
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>

int readLine(char *buffer, size_t size,int fd) {
    size_t cnt = 0;
    char c;

    if (buffer == NULL || size == 0)
        return -1;

    while (read(fd, &c, 1) == 1 && cnt < size - 1) {
        if (c == '\n') {
            buffer[cnt] = 0;
            return 1;
        }

        buffer[cnt++] = c;
    }

    buffer[cnt] = 0;
    return 0;
}

//S_ISDIR access
int main(int argc, char *argv[]) {
    pid_t pid;
    int waited, stat, ret_code;
    int confFileRead;
    int results;
    int score;
    int in, out, error;
    if (argc < 2) {
        exit(-1);
    }
    confFileRead = open(argv[1], O_RDWR);
    if (confFileRead == -1) {
        perror("Error in: open");
        exit(-1);
    }
    char mainFolderPath[200];
    char inputFilePath[200];
    char outputFilePath[200];

    readLine(mainFolderPath, sizeof mainFolderPath,confFileRead);
    readLine(inputFilePath, sizeof inputFilePath,confFileRead);
    readLine(outputFilePath, sizeof outputFilePath,confFileRead);

    close(confFileRead);

    int isDir;
//    if (access(mainFolderPath, 'S_ISDIR')) {
//        perror("Error in: access");
//        exit(-1);
//    }
//    if (access(inputFilePath, 'S_ISDIR')) {
//        perror("Error in: access");
//        exit(-1);
//    }
//    if (access(outputFilePath, 'S_ISDIR')) {
//        perror("Error in: access");
//        exit(-1);
//    }
    in = open(inputFilePath, O_RDONLY);
    if (in == -1) {
        perror("Error in: open");
        exit(-1);
    }
    // replace standard input with input file
    dup2(in, 0);

    out = open("output.txt", O_RDWR | O_CREAT, 0777);
    if (out == -1) {
        perror("Error in: open");
        exit(-1);
    }
    // replace standard output with output file
    dup2(out, 1);
    // replace error output with error file
    error = open("errors.txt", O_CREAT | O_RDWR, 0777);
    dup2(error, 2);
    results = open("results.csv", O_RDWR | O_CREAT, 0777);
    if (results == -1) {
        perror("Error in: open");
        exit(-1);
    }




    DIR *dip;
    struct dirent *dit;
    int i = 0;
    int readFile;
    if ((dip = opendir(mainFolderPath)) == NULL) {
        perror("opendir");
        return 0;
    }

    while ((dit = readdir(dip)) != NULL) {
        i++;
        readFile = readdir(dit->d_name);
        if (readFile < 0) {
            perror("Error in:read");
            exit(-1);
        }
        if (dit->d_name[-2] == '.' && dit->d_name[-1] == 'c') {
            pid = fork();
            if (pid == 0) { /* Child */
                char *args[] = {"gcc", NULL};
                execv(readFile, args);
                if (ret_code == -1) {
                    perror("Error in:exec");
                    exit(-1);
                }
            } else { /* Parent */
                waited = wait(&stat);
                if (waited < 0) {
                    score=10;
                    write(results,("%d",score),150);
                    perror("Error in : gcc");
                }
                pid = fork();
                if (pid == 0) { /* Child */
                    char *args[] = {"./a.out",NULL};
                    execvp(readFile, args);
                    if (ret_code == -1) {
                        perror("Error in:exec");
                        exit(-1);
                    }
                } else { /* Parent */
                    waited = wait(&stat);
                    if (waited < 0) {
                        perror("Error in : ex21");
                    }
                    pid = fork();
                    if (pid == 0) { /* Child */
                        char *args[] = {"./comp.out", "output.txt", outputFilePath, NULL};
                        execvp(readFile, args);
                        if (ret_code == -1) {
                            perror("Error in:exec");
                            exit(-1);
                        }
                    } else { /* Parent */
                        waited = wait(&stat);
                        if (waited < 0) {
                            perror("Error in : ex21");
                        }
                        if(stat==1){
                            score=100;
                        }
                        if(stat==2){
                            score=50;
                        }
                        if(stat==3){
                            score=75;
                        }
                        write(results,("%d",score),150);
                    }
                }
            }
        }
    }

    if (closedir(dip) == -1) {
        perror("closedir");
        return 0;
    }

    // close unused file descriptors
    close(in);
    close(out);
    close(error);
    close(results);
}