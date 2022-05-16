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
    size_t counter = 0;
    char c;
    if (buffer == NULL || size == 0)
        return -1;
    while (read(fd, &c, 1) == 1 && counter < size - 1) {
        if (c == '\n') {
            buffer[counter] = 0;
            return 1;
        }
        buffer[counter++] = c;
    }
    buffer[counter] = 0;
    return 0;
}

//compiles student file using gcc
int compileFile(char *filePath) {
    pid_t pid;
    int status;
    pid = fork();
    if(pid<0) {
        perror("Error in: fork");
        return -1;
    }
    if (pid == 0) {
        char *args[] = {"gcc", filePath, NULL};
        if (execvp("gcc", args) < 0) {
            perror("Error in: execvp");
            return -1;
        }
    } else {
        waitpid(pid, &status, 0);
        int exitStatus = WEXITSTATUS(status);
        if (exitStatus != 0) {
            return -1;
        }
        return 1;
    }
}

int executeAOut(char *filePath) {
    pid_t pid;
    int status, ret_code;
    pid = fork();
    if(pid<0) {
        perror("Error in: fork");
        return -1;
    }
    if (pid == 0) {
        char *args[] = {NULL};
        ret_code = execvp(filePath, args);
        if (ret_code < 0) {
            perror("Error in: execvp");
            return -1;
        }
    } else {
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) != 0) {
            return -1;
        }
        return 1;
    }
}

int executeCompOut(char *filePath, char *tempOutput, char *outputFilePath) {
    pid_t pid;
    int status2, ret_code;
    pid = fork();
    if(pid<0) {
        perror("Error in: fork");
        return -1;
    }
    if (pid == 0) {
        char *args[] = {filePath, tempOutput, outputFilePath, NULL};
        ret_code = execvp(filePath, args);
        if (ret_code < 0) {
            perror("Error in: execvp");
            return -1;
        }
    } else {
//        waitpid(pid, &status2, 0);
        wait(&status2);

        int exitStatus = WEXITSTATUS(status2);
        if (exitStatus > 3) {
            return -1;
        }
        return exitStatus;
    }
}


int main(int argc, char *argv[]) {
    int status, ret_code,confFileRead,score=0,in, out, error, results;//fd for 0 1 2 and results.csv
    char stringScore[500] = {};
    //check number of args
    if (argc < 2) {
        exit(-1);
    }
    error = open("errors.txt", O_CREAT | O_WRONLY, 0777);
    if (error == -1) {
        perror("Error in: open");
        exit(-1);
    }
    dup2(error, 2);

    //open config file and read from it
    confFileRead = open(argv[1], O_RDWR);
    if (confFileRead == -1) {
        perror("Error in: open");
        exit(-1);
    }

    char mainFolderPath[500] = {0};
    char studentPath[500] = {0};
    char inputFilePath[500] = {0};
    char outputFilePath[500] = {0};
    char tempOutput[500] = {0};
    char compOut[500] = {0};

    readLineFromConf(studentPath, sizeof studentPath, confFileRead);
    readLineFromConf(inputFilePath, sizeof inputFilePath, confFileRead);
    readLineFromConf(outputFilePath, sizeof outputFilePath, confFileRead);

    close(confFileRead);

    char s2[500] = {};
    char s3[500] = {};

    getcwd(mainFolderPath, 150);

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
    strcpy(tempOutput, mainFolderPath);
    strcat(tempOutput, "/output.txt");
    strcpy(compOut, mainFolderPath);
    strcat(compOut, "/comp.out");

    //checking that the paths indeed are legit
    struct stat statbuf;
    if (stat(studentPath, &statbuf) >= 0) {
        if (!S_ISDIR(statbuf.st_mode)) {
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

//    duping input to 0 , output to 1,and error.txt to 2
    in = open(inputFilePath, O_RDONLY, 0777);
    if (in == -1) {
        perror("Error in: open");
        exit(-1);
    }
    // replace standard input with input file
    dup2(in, 0);

    out = open("output.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWGRP | S_IWUSR);
    if (out == -1) {
        perror("Error in: open");
        exit(-1);
    }
    // replace standard output with output file
    dup2(out, 1);
    // replace error output with error file
    close(in);
    close(out);
    results = open("results.csv", O_CREAT | O_WRONLY, 0777);
    if (results == -1) {
        perror("Error in: open");
        exit(-1);
    }

    //dip for students folder,dip2 for each student
    DIR *dip,*dip2;
    struct dirent *dit,*dit2;
    int flag = 0;
    if ((dip = opendir(studentPath)) == NULL) {
        perror("opendir");
        return 0;
    }
    chdir(studentPath);
    while ((dit = readdir(dip)) != NULL) {
        if (dit->d_name[0] == '.') {
            continue;
        }
        if ((dip2 = opendir(dit->d_name)) == NULL) {
            perror("opendir");
            return 0;
        }
        chdir(dit->d_name);
        while ((dit2 = readdir(dip2)) != NULL) {
            if (strlen(dit2->d_name) > 2 && dit2->d_name[strlen(dit2->d_name) - 2] == '.' &&
                dit2->d_name[strlen(dit2->d_name) - 1] == 'c') {
                if (stat(dit2->d_name, &statbuf) >= 0) {
                    if (!S_ISREG(statbuf.st_mode)) {
                        continue;
                    }
                }
                flag = 1;
                ret_code = compileFile(dit2->d_name);
                if (ret_code < 0) {
                    score = 10;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name, score, "COMPILATION_ERROR\n");
                    write(results, stringScore, strlen(stringScore));
                    break;
                }
                remove(tempOutput);
                out = open(tempOutput, O_CREAT | O_WRONLY, S_IRUSR | S_IWGRP | S_IWUSR);
                if (out == -1) {
                    perror("Error in: open");
                    exit(-1);
                }
                // replace standard output with output fi le
                dup2(out, 1);
                lseek(0, 0, SEEK_SET);
                status = executeAOut("./a.out");
                remove("a.out");
                status = executeCompOut(compOut, tempOutput, outputFilePath);
                if (status == 1) {
                    score = 100;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name, score, "EXCELLENT\n");
                    write(results, stringScore, strlen(stringScore));

                }
                if (status == 2) {
                    score = 50;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name, score, "WRONG\n");
                    write(results, stringScore, strlen(stringScore));
                }
                if (status == 3) {
                    score = 75;
                    sprintf(stringScore, "%s,%d,%s", dit->d_name, score, "SIMILAR\n");
                    write(results, stringScore, strlen(stringScore));
                }
            }
        }
        if (flag == 0) {
            score = 0;
            sprintf(stringScore, "%s,%d,%s", dit->d_name, score, "NO_C_FILE\n");
            write(results, stringScore, strlen(stringScore));
        }
        flag = 0;
        if (closedir(dip2) == -1) {
            perror("closedir");
            exit(-1);
        }
        chdir("..");
    }

    if (closedir(dip) == -1) {
        perror("closedir");
        exit(-1);
    }
    remove(tempOutput);

// close unused file descriptors
    close(error);
    close(results);
    exit(0);
}
