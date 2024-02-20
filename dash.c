#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

// Function prototype
char *findPath(char *command);
void errorHandler();
void path(int argc, char *argv[]);

// On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
void cd(int argc, char *argv[])
{
    // printf("this is-%s-\n", argv[1]);

    if (argc == 2)
    {
        if (chdir(argv[1]) == 0)
        {
            // printf("Changed directory to: %s\n", argv[1]);
        }
        else
        {
            // printf("No such directory\n");
        }
    }
    else
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        // printf("there\n");
    }
}

void redirection(char *file, char *argv[])
{
    // printf("file: -%s-\n", file);
    //  char *argv = ["ls","-la","/tmp"];    ls -la /tmp > output

    FILE *outputFile;
    outputFile = fopen(file, "w");

    if (outputFile == NULL)
    {
        // error : No Permission
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }

    // switch to stdout
    if (freopen(file, "w", stdout) == NULL)
    {
        // error : Output Fail
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }

    char *cmdPath = findPath(argv[0]);
    execv(cmdPath, argv);

    fclose(outputFile);
    return;
}

void execute(int argc, char *argv[])
{

    // int argc = 3;
    // char *argv = ["ls","-la","/tmp"];   // ls -la /tmp > output

    char *cmdPath;

    cmdPath = findPath(argv[0]);
    if (cmdPath != NULL)
    {
        execv(cmdPath, argv);
    }
    else
    {
        // printf("exeThere\n");
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

int cmdParser(char *input)
{

    // printf("cmdParser\n");
    char *str1 = strdup(input);
    char *str2, *str3, *megatoken, *token, *subtoken, *saveptr1, *saveptr2, *saveptr3;
    int j, k;

    pid_t childPIDs[256]; // Store all child PIDs
    int numChilds = 0;

    // Parsing
    for (k = 0;; k++, str1 = NULL)
    {
        megatoken = strtok_r(str1, ";", &saveptr1);
        if (megatoken == NULL)
        {
            break;
        }

        if (strncmp(megatoken, "&", 1) == 0)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }

        if (strncmp(megatoken, "exit", 4) == 0)
        {
            size_t length = strlen(megatoken);
            if (length > 4)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                // printf("exit bad\n");
                continue;
            }
            else
            {
                // printf("normal exit\n");
                exit(0);
            }
        }
        // int t = 0;
        for (str2 = megatoken;; j++, str2 = NULL)
        {

            // Layer 1: tokenize multiple command lines
            // Split using &: "ls -la & echo"  -> {"ls -la", "echo"}
            token = strtok_r(str2, "&", &saveptr2);
            if (token == NULL)
            {
                break;
            }

            /* char* test = strdup(token);
            printf("test%d : %s\n", t++, test); */
            int redirect = 0;

            // Layer 2: for each command lines, parse the arguments
            // Split using " ": "ls -la" -> {"ls", "-la", NULL}
            int inputidx = 0;
            int cmdidx = 0;
            char *cmd[256];
            char *outputFile = NULL;
            for (str3 = token;; inputidx++, str3 = NULL)
            {
                subtoken = strtok_r(str3, " ", &saveptr3);

                if (subtoken != NULL)
                {

                    // \r\n truncation
                    size_t tokenLength = strlen(subtoken);
                    if (subtoken[tokenLength - 2] == '\r')
                    {
                        subtoken[tokenLength - 2] = '\0';
                    }
                    else if (subtoken[tokenLength - 1] == '\r' || subtoken[tokenLength - 1] == '\n')
                    {
                        subtoken[tokenLength - 1] = '\0';
                    }
                    // Output filestream case
                    if (strcmp(subtoken, ">") == 0)
                    {
                        outputFile = strtok_r(NULL, " ", &saveptr3);
                        redirect += 1;
                        if (strtok_r(NULL, " ", &saveptr3) || redirect > cmdidx)
                        {
                            char error_message[30] = "An error has occurred\n";
                            // printf("test");
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            return 0;
                        }
                    }
                    // Normal Command case
                    else
                    {
                        cmd[cmdidx] = subtoken;
                        cmdidx++;
                    }
                }
                if (subtoken == NULL)
                {
                    break;
                }
            }
            cmd[cmdidx] = NULL;

            // cd case: early detection
            if (strcmp(cmd[0], "cd") == 0)
            {
                cd(cmdidx, cmd);
            }
            else if (strcmp(cmd[0], "path") == 0)
            {
                path(cmdidx, cmd);
            }
            // non-cd case: forkable execution
            else
            {
                // Fork parallel process
                pid_t pid;
                pid = fork();
                if (pid == 0)
                {
                    if (outputFile != NULL)
                    {
                        // printf("Filestream execution...\n");
                        if (redirect != 1)
                        {
                            // printf("pid==0\n");
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            // return 0;
                            exit(0);
                        }
                        redirection(outputFile, cmd);
                    }
                    else if (redirect)
                    {
                        // printf("pid=0\n");
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        // return 0;
                        exit(0);
                    }
                    else
                    {
                        execute(cmdidx, cmd);
                        exit(0);
                    }
                }
                else if (pid < 0)
                {
                    // printf("pid<0\n");
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    // return 0;
                    exit(0);
                }
                else
                {
                    // printf("Parent PID:%d -> Child PID: %d\n", (int) getpid(), pid);
                    childPIDs[numChilds] = pid;
                    numChilds++;
                }
            }
        }

        // All child process should be done before the function call ends
        int i;
        for (i = 0; i < numChilds; i++)
        {

            int status; // status buffer
            pid_t terminatedPid = waitpid(childPIDs[i], &status, 0);
            if (terminatedPid > 0)
            {
                if (WIFEXITED(status))
                {
                    // int exitStatus = WEXITSTATUS(status);
                    // printf("Child process %d exited with status %d\n", terminatedPid, exitStatus);
                }
                else
                {
                    // printf("Child process %d exited abnormally\n", terminatedPid);
                }
            }
        }
    }

    // Release memory
    free(str1);
    return 0;
}

void interactiveMode()
{
    // printf("interactiveMode!\n");
    char *cmd_buffer = NULL;
    size_t buffersize = 0;
    size_t characters;

    while (1)
    {
        printf("dash > ");

        characters = getline(&cmd_buffer, &buffersize, stdin);

        if (characters == -1)
        {
            break; // Exit on EOF or error
        }
        if (characters > 0 && cmd_buffer[characters - 1] == '\n')
        {
            cmd_buffer[characters - 1] = '\0';
        }

        // Early dropout
        if (strcmp(cmd_buffer, "&") == 0)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }
        if (strcmp(cmd_buffer, "exit") == 0)
        {
            // printf("You activate exit!!!\n");
            exit(0);
        }
        while (isspace((unsigned char)*(cmd_buffer)))
            cmd_buffer++;
        if (*cmd_buffer == 0)
        {
            continue;
        }

        // Parse the command
        cmdParser(cmd_buffer);
    }

    // Free memory for line
    free(cmd_buffer);
}

void batchMode(char *argv)
{

    FILE *inputFile;
    inputFile = fopen(argv, "r");

    if (inputFile == NULL)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }

    char *cmd_buffer = NULL;
    char *result_buffer = NULL;
    size_t buffersize = 0;
    size_t characters;

    while ((characters = getline(&cmd_buffer, &buffersize, inputFile)) != -1)
    {

        if (cmd_buffer[characters - 2] == '\r')
        {
            cmd_buffer[characters - 2] = '\0';
        }
        else if (cmd_buffer[characters - 1] == '\n')
        {
            cmd_buffer[characters - 1] = '\0';
        }

        while (isspace((unsigned char)*(cmd_buffer)))
            cmd_buffer++;
        if (*cmd_buffer == 0)
        {
            continue;
        }

        // printf("batchMode input: %s\n", cmd_buffer);

        if (result_buffer == NULL)
        { // First line copy
            result_buffer = strdup(cmd_buffer);
        }
        else
        {                                                               // Following realloc
            size_t expandSize = strlen(result_buffer) + characters + 2; // 2 for ";\0"

            result_buffer = (char *)realloc(result_buffer, expandSize);
            strcpy(result_buffer, result_buffer);

            // Add ";" to detected newline
            strcat(result_buffer, ";");
            strcat(result_buffer, cmd_buffer);
        }

        // printf("Result progress ... %s\n", result_buffer);
    }

    cmdParser(result_buffer);
    free(cmd_buffer);
    free(result_buffer);
    fclose(inputFile);

    return;
}

char *PATH = "/bin"; // globalization

void path(int argc, char *argv[])
{
    if (argc < 2)
    {
        PATH = NULL;
    }
    else
    {

        if (PATH == NULL)
        {
            int i;
            size_t newLength = 0;

            for (i = 1; i < argc; i++)
            {
                newLength += strlen(argv[i]) + 1; // plus number of : we need to add in-between
            }
            newLength--; // number of : needed is #words-1

            char *newPATH = (char *)malloc(newLength + 1); // terminator null

            if (newPATH)
            {
                strcpy(newPATH, argv[1]);

                int j = 0;
                for (j = 2; j < argc; j++)
                {
                    strcat(newPATH, ":");
                    strcat(newPATH, argv[j]);
                }

                PATH = newPATH;
            }
            else
            {
                printf("Memory allocation error\n");
            }
        }
        else
        {
            int i;
            size_t newLength = (PATH ? strlen(PATH) : 0);

            for (i = 1; i < argc; i++)
            {
                newLength += strlen(argv[i]) + 1; // plus number of : we need to add in-between
            }

            char *newPATH = (char *)malloc(newLength + 1); // terminator null

            if (newPATH)
            {
                strcpy(newPATH, PATH);

                int j = 0;
                for (j = 1; j < argc; j++)
                {
                    strcat(newPATH, ":");
                    strcat(newPATH, argv[j]);
                }

                PATH = newPATH;
            }
            else
            {
                printf("Memory allocation error\n");
            }
        }
    }
}

char *findPath(char *command)
{

    char *path, *pathDup, *path_token, *file_path;
    int commandLength, dirLength;

    // path = getenv("PATH");
    path = PATH;

    if (path)
    {
        pathDup = strdup(path);
        commandLength = strlen(command);
        path_token = strtok(pathDup, ":");

        // Within every path of {echo $PATH}, demimited by ":", test and search
        while (path_token != NULL)
        {

            dirLength = strlen(path_token);

            // Reserve space
            file_path = malloc(commandLength + dirLength + 2); // add 2 for "/" and "\0"
            // Concat
            strcpy(file_path, path_token);
            strcat(file_path, "/");
            strcat(file_path, command);
            strcat(file_path, "\0");

            // Test path and executable
            int okFlag = access(file_path, F_OK);
            if (okFlag == 0)
            {
                // printf("Executable path found: %s\n", file_path);
                free(pathDup); // Since pathDup is dynamically allocated, better free it before next asignment
                return file_path;
            }
            else
            {
                free(file_path);
                path_token = strtok(NULL, ":");
            }
        }
    }
    else
    {
        return NULL;
    }
    int okFlag = access(command, F_OK);
    if (okFlag == 0)
    {
        // printf("Executable path as given: %s", command);
        return command;
    }
    else
    {
        // printf("Executable path NOT found...\n");
        // errorHandler();
        return NULL;
    }
}

void errorHandler()
{
    // printf("errorHand\n");
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        interactiveMode();
    }
    else if (argc == 2)
    {
        batchMode(argv[1]);
    }
    else
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }

    exit(EXIT_SUCCESS);
    return 0;
}