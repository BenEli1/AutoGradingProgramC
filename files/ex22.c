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
#include <sys/stat.h>
#include <sys/types.h>

//reads a line from conf file.
int readLineFromConf(char *buffer, size_t size, int fd) {
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

//compiles student file using gcc
int compileFile(char *filePath) {
    pid_t pid;
    int status, ret_code;
    pid = fork();
    if (pid == 0) {
        char *args[] = {"gcc", filePath, NULL};
        if (execv("gcc", args) < 0) {
            perror("Error in: exec");
            return -1;
        }
    } else {
        waitpid(pid, &status, 0);
        if (status < 0) {
            return -1;
        }
    }
    return 1;
}

int executeAOut(char *filePath) {
    pid_t pid;
    int status, ret_code;
    pid = fork();
    if (pid == 0) {
        perror(filePath);
        ret_code = execvp(filePath, NULL);
        if (ret_code < 0) {
            perror("Error in: exec");
            return -1;
        }
    } else {
        waitpid(pid, &status, 0);
        if (status < 0) {
            return -1;
        }
    }
    return 1;
}

int executeCompOut(char *filePath,char *tempOutput,char* outputFilePath) {
    pid_t pid;
    int status, ret_code;
    pid = fork();
    if (pid == 0) {
        char *args[] = {tempOutput, outputFilePath, NULL};
        ret_code = execvp(filePath, args);
        if (ret_code < 0) {
            perror("Error in: exec");
            return -1;
        }
    } else {
        waitpid(pid, &status, 0);
        if (status < 0) {
            return -1;
        }
    }
    return 1;
}


int main(int argc, char *argv[]) {
    pid_t pid;
    int waited, status, ret_code;
    int confFileRead;
    int score = 0;
    char stringScore[50] = {};
    int in, out, error, results;//fd for 0 1 2 and results.csv
    //check number of args
    if (argc < 2) {
        exit(-1);
    }

    //open config file and read from it
    confFileRead = open(argv[1], O_RDWR);
    if (confFileRead == -1) {
        perror("Error in: open");
        exit(-1);
    }

    char mainFolderPath[500] = {0};
    char inputFilePath[500] = {0};
    char outputFilePath[500] = {0};
    char tempOutput[500] = {0};

    readLineFromConf(mainFolderPath, sizeof mainFolderPath, confFileRead);
    readLineFromConf(inputFilePath, sizeof inputFilePath, confFileRead);
    readLineFromConf(outputFilePath, sizeof outputFilePath, confFileRead);

    close(confFileRead);


    char s1[500] = {};
    char s2[500] = {};
    char s3[500] = {};
    char s4[500] = {};

    chdir("..");
    chdir("files");//#TODO delete this shit
    if (mainFolderPath[0] != '/') {
        strcat(getcwd(s1, 150), "/");
        strcat(s1, mainFolderPath);
        strcpy(mainFolderPath, s1);
    }
    if (inputFilePath[0] != '/') {
        strcat(getcwd(s2, 150), "/");
        strcat(s2, inputFilePath);
        strcpy(inputFilePath, s2);
    }
    if (outputFilePath[0] != '/') {
        strcat(getcwd(s3, 150), "/");
        strcat(s3, outputFilePath);
        strcpy(outputFilePath, s3);
    }
    strcat(getcwd(s4, 150), "/");
    strcat(s4, "output.txt");
    strcpy(tempOutput, s4);

    //checking that the paths indeed are legit
    struct stat statbuf;
    if (stat(mainFolderPath, &statbuf) >= 0) {
        if (!S_ISDIR(statbuf.st_mode)) {
            perror("Not a valid directory");
            write(1, "Not a valid directory\n", 23);
            exit(-1);
        }
    }
    if (stat(inputFilePath, &statbuf) >= 0) {
        if (!S_ISREG(statbuf.st_mode)) {
            write(1, "Input file not exist\n", 22);
            exit(-1);
        }
    }
    if (stat(outputFilePath, &statbuf) >= 0) {
        if (!S_ISREG(statbuf.st_mode)) {
            write(1, "Output file not exist\n", 23);
            exit(-1);
        }
    }

    //duping input to 0 , output to 1,and error.txt to 2
    in = open(inputFilePath, O_RDONLY, 0777);
    if (in == -1) {
        perror("Error in: open");
        exit(-1);
    }
    // replace standard input with input file
    dup2(in, 0);

    out = open("output.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWGRP | S_IWUSR);
    if (out == -1) {
        perror("Error in: open");
        exit(-1);
    }
    // replace standard output with output file
    dup2(out, 1);
    // replace error output with error file
    error = open("errors.txt", O_CREAT | O_WRONLY, 0777);
    if (error == -1) {
        perror("Error in: open");
        exit(-1);
    }
    dup2(error, 2);
    close(in);
    close(out);
    results = open("results.csv", O_CREAT | O_WRONLY, 0777);
    if (results == -1) {
        perror("Error in: open");
        exit(-1);
    }


    DIR *dip;
    DIR *dip2;
    struct dirent *dit;
    struct dirent *dit2;
    int flag = 0;
    char s[250];
    char mainFolderPath2[500] = {0};
    char mainFolderPath3[500] = {0};
    char mainFolderPath4[500] = {0};
    char mainFolderPath5[500] = {0};
    if ((dip = opendir(mainFolderPath)) == NULL) {
        perror("opendir");
        return 0;
    }

    while ((dit = readdir(dip)) != NULL) {
        strcpy(mainFolderPath2, mainFolderPath);
        strcat(mainFolderPath2, "/");
        strcat(mainFolderPath2, dit->d_name);
        if ((dip2 = opendir(mainFolderPath2)) == NULL) {
            perror("opendir");
            return 0;
        }
        while ((dit2 = readdir(dip2)) != NULL) {
            if (strlen(dit2->d_name) > 2 && dit2->d_name[strlen(dit2->d_name) - 2] == '.' &&
                dit2->d_name[strlen(dit2->d_name) - 1] == 'c') {
                flag = 1;
                strcpy(mainFolderPath3, mainFolderPath2);
                strcat(mainFolderPath3, "/");
                strcat(mainFolderPath3, dit2->d_name);
                ret_code = compileFile(mainFolderPath3);
                if (ret_code < 0) {
                    score = 10;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name,score,"COMPILATION_ERROR\n");
                    write(results, stringScore, 150);
                    perror("Error in : gcc");
                    break;
                }
                strcpy(mainFolderPath4, mainFolderPath2);
                strcat(mainFolderPath4, "/");
                strcat(mainFolderPath4, "a.out");
                status = executeAOut(mainFolderPath4);
                strcpy(mainFolderPath5, mainFolderPath2);
                strcat(mainFolderPath5, "/");
                strcat(mainFolderPath5, "comp.out");
                status = executeCompOut(mainFolderPath5, tempOutput, outputFilePath);
                if (status == 1) {
                    score = 100;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name,score,"EXCELLENT\n");
                    write(results, stringScore, 150);

                }
                if (status == 2) {
                    score = 50;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name,score,"WRONG\n");
                    write(results, stringScore, 150);
                }
                if (status == 3) {
                    score = 75;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name,score,"SIMILAR\n");
                    write(results, stringScore, 150);
                }
            }
        }
                if (closedir(dip2) == -1) {
                    perror("closedir");
                    return 0;
                }
            }

if (closedir(dip)== -1) {
perror("closedir");
return 0;
}

// close unused file descriptors
close(error);
close(results);
exit(0);
}
