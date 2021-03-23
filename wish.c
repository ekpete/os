#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

char command[50]; // 50 is max length of a command
char command2[50]; // --"--

/* NOTE: In input(txt) and .sh files, you need to add a whitespace after the command, and no new line at the end of the file. We provided an example .txt file */ 

int main(void) { 
    int active = 1;  // flag to determine when to exit program

    while (active) { 
        char s[100];
        if (feof(stdin)) // check if we are at the end of the input file. In that case, don't read anymore, exit the child process, and continue in parent.
        {
            command[0] = '\0';
            exit(0);
        }
        else
        {   
            printf("%s hackerman$ ", getcwd(s, 100)); // print the prompt with the current path
            fgets(command, sizeof(command), stdin); // reads input

            if (strlen(command) > 0) // check if command has length > 0
            {
                command[strlen(command) - 1] = '\0'; // removes trailing newline
            }
        }
        
        memcpy(command2, command, sizeof(command)); // copys the input for later use
        char * tokens = NULL; // initializes tokens
        if((strlen(command) > 0))
        {
            tokens = strtok(command, " "); // splitting the input into tokens, splitting on " "
            // For task a), if you want to print out the command, just uncomment all the printf lines.
            //printf("Command name: %s\n", tokens); //prints the command part of the input - uncomment here

            if (!(((strcmp(&command[0], "<") == 0) || (strcmp(&command[0], ">") == 0))))
            {
                tokens = strtok(NULL, " "); // removes the first token
            }
        }

        /* The following while loop has two purposes: it prints out the command according to task a), and it sets up two arrays for later use with redirect.
            We assume every command is on the form: command - parameters - redirection(s). */

        char * input[2]; // for later use with redirect
        char * output[2]; // --"--
        input[0] = NULL;
        input[1] = NULL;
        output[0] = NULL;
        output[1] = NULL;
        while (tokens != NULL){ // iterates through the remaining tokens
            if (strcmp(tokens, "<") == 0 || strcmp(tokens, ">") == 0)
            {  // checks if token is a redirect, and if so make input and/or output lists with redirect info
                //printf("Redirection: "); // - uncomment here.
                int i = 0;
                int j = 0;
                while(tokens != NULL){
                    int res1 = strcmp(tokens, "<");
                    int res2 = strcmp(tokens, ">");
                    if (res1 == 0 || i == 1){
                        input[i++] = tokens;
                    }
                    if (res2 == 0 || j == 1){
                        output[j++] = tokens;
                    }
                    //printf("%s ", tokens); // - uncomment here
                    tokens = strtok(NULL, " ");
                }
                //printf("\n"); // - uncomment here
            }
            else{
                //printf("Command parameter: %s\n", tokens); // prints rest of the tokens (they are not command or redirect, they must be parameters) - uncomment here
                tokens = strtok(NULL, " ");
            }
        }

        char * tokens2 = NULL;
        if(strlen(command2) > 0)
        {
            tokens2 = strtok(command2, " "); // make new tokens, since strtok actually alters "command" when we are printing in task a), and we can´t access "tokens"
        }

        char* args[50];  // a list of tokens, for use with execvp
        int i = 0;
        while(tokens2 != NULL) // put the tokens in array args
        {
            args[i++] = tokens2;
            tokens2 = strtok(NULL, " ");
        }

        args[i] = NULL; // sets the element after the command to NULL, for correct format in execvp

        int script = 0; // flag for indicating a shell script
        if ((0 < i) && strstr(args[0], ".sh") != NULL) {  // the (0 < i) part is a check to ensure that you can just type return (enter) and it doesn't crash
            script = 1;
        }
        
        // int j = 0;
        // for (j = 0; j < i; j++) // this was just a test for checking that the array is on the correct form
        // {
        //      printf("%s\n", args[j]);
        // }
        
        if ((0 < i) && (0 == strcmp(args[0], "cd")))
        {
            if(chdir(args[1]) != 0) // execute the internal cd command
            {
                perror("CD error: ");
            }
        }
        else if ((0 < i) && (0 == strcmp(args[0], "exit")))
        {
            exit(0); // execute the internal exit command
        }

        else
        {  
            // fprintf(stderr, "PID: %d\n", getpid()); // just a check for debugging with child process.
            pid_t pid;
            pid = fork(); // make a child process with fork
            if (pid == 0) {
                FILE * inputfile;   
                FILE * outputfile;
                if (script) // checks if script flag is set
                {
                    char *inputname = args[0];
                    int inputfile = open(inputname, O_RDONLY, 0777); // opens the inputfile
                    if (inputfile == -1){
                        printf("The file: %s does not exist.\n", inputname);
                        exit(0); // exit the child process
                    }else{
                        dup2(inputfile, STDIN_FILENO); // sets inputfile to STDIN
                    }
                }

                if (input[0] != NULL)
                {
                    char *inputname = input[1];
                    int inputfile = open(inputname, O_RDONLY, 0777); // opens the inputfile
                    if (inputfile == -1){
                        printf("The file: %s does not exist.\n", inputname);
                        exit(0); // exit the child process
                    }else{
                        dup2(inputfile, STDIN_FILENO); // sets inputfile to STDIN
                    }
                }
                if (output[0] != NULL)
                {
                    char *outputname = output[1];
                    //strcat(outputname, ".txt"); // you can uncomment this if you don't want the user to type .txt
                    int outputfile = open(outputname, O_WRONLY | O_CREAT, 0777); // permissions(0777 - everyone has access)
                    int file2 = dup2(outputfile, STDOUT_FILENO); //sets outputfile to STDOUT (standard output)
                    
                }

                if (output[0] == NULL && input[0] == NULL && script == 0){
                    if (execvp(args[0], args) < 0) { // using execvp to execute the command
                        printf("Error executing command.\n");
                        exit(0); // exit the child process
                    }
                }
            }
            else if (pid > 0) {
                wait(NULL); // lets the parent wait for the child process to finish
            }
            else{
                printf("An error!\n");
            }
        }

        fflush(stdout);  // to ensure that whatever is written to a file/the console is indeed written
    }
    return 0;
}
