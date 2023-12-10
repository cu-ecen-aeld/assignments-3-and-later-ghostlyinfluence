// Colin Burgin 12/10/2023

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    // Open LOG_USER
    openlog(NULL,0,LOG_USER);

    // Verify that exactly 2 arguments are provided
    if (argc != 3) {
        syslog(LOG_ERR, "ERROR: Expected 2 arguments, was given %d", argc);
        return EXIT_FAILURE; //Return 1
    }

    char *writefile = argv[1];
    char *writestr = argv[2];

    // Assume that the directory exists already based on assignment instructions
    FILE *file = fopen(writefile, "w"); //Overwrite based on instructions
    if (file == NULL) {
        syslog(LOG_ERR, "ERROR: Unable to open file %s", writefile);
        return EXIT_FAILURE; //Return 1
    }

    // Write the contents to the file
    fprintf(file, "%s\n", writestr);
    syslog(LOG_DEBUG, "DEBUG: Writing %s to %s", writestr, writefile);

    // Close the file
    fclose(file);
    syslog(LOG_DEBUG, "DEBUG: File closed");
    return EXIT_SUCCESS; //Return 0
}