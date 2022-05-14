// Ben Eli 319086435
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    //return value starts with 1 assuming they are equal
    int returnValue = 1;
    //if argc is lower than 3 then return error
    if (argc < 3) {
        exit(-1);
    }
    //file1 - file2 are the opening fd,file1read-file2read are the reading fd.
    //ch1,ch2 are for checking byte byte between the files.
    int file1Read, file2Read, file1, file2;
    char ch1, ch2;

    //first trying to open the files.
    file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) {
        perror("Error in: open");
        exit(-1);
    }
    file2 = open(argv[2], O_RDONLY);
    if (file2 == -1) {
        close(file1);
        //exit without _ should close every open files/sockets/fd.
        perror("Error in: open");
        exit(-1);
    }
    do {
        //then trying to read the files
        file1Read = read(file1, &ch1, 1);
        if (file1Read < 0) {
            perror("Error in: read");
            exit(-1);
        }

        file2Read = read(file2, &ch2, 1);

        if (file2Read < 0) {
            perror("Error in: read");
            exit(-1);
        }

        //if they are not equal then we cannot return 1 anymore, so its either 3 if they are similar or 2 if not equal.
        if (ch1 != ch2) {
            while ((ch1 == '\n' || ch1 == ' ') && file1Read == 1) {
                file1Read = read(file1, &ch1, 1);
                if (file1Read < 0) {
                    perror("Error in: read");
                    exit(-1);
                }
            }
            while ((ch2 == '\n' || ch2 == ' ') && file2Read == 1) {
                file2Read = read(file2, &ch2, 1);
                if (file2Read < 0) {
                    perror("Error in: read");
                    exit(-1);
                }
            }
            if (toupper(ch1) == toupper(ch2) || ch1 == ch2 || file1Read == 0 && file2Read == 0) {
                returnValue = 3;
            } else {
                close(file1);
                close(file2);
                return 2;
            }
        }
    } while (file1Read > 0 || file2Read > 0);

    close(file1);
    close(file2);

    return returnValue;
}