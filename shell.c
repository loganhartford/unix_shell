#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 1000
#define MAX_TOKENS 100

void trim_whitespace(char *str)
{
    char *end;
    char *start = str;

    while (*start == ' ')
    {
        start++;
    }

    // Move the start of the trimmed string
    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }

    // Empty string
    if (*str == '\0')
        return;

    end = str + strlen(str) - 1;
    while (end > str && *end == ' ')
    {
        end--;
    }

    // Add null terminator
    *(end + 1) = '\0';
}

int main(void)
{
    char input[MAX_INPUT_LENGTH];

    while (1)
    {
        // Use stderr for the prompt to keep it separate from command output
        fprintf(stderr, "$ ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // EOF received, exit
            break;
        }

        // Remove the newline character
        input[strcspn(input, "\n")] = '\0';

        trim_whitespace(input);

        if (strlen(input) == 0)
        {
            continue;
        }

        printf("Input: %s\n", input);

        if (strncmp(input, "exit", 4) == 0)
        {
            break;
        }

        // Tokenize the input
        char *tokens[MAX_TOKENS];
        int token_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && token_count < MAX_TOKENS)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, " ");
        }
        tokens[token_count] = NULL;

        // If we have tokens, execute the first one
        if (token_count > 0)
        {
            pid_t pid = fork();
            if (pid == 0) // Child
            {
                execvp(tokens[0], tokens);
                // execvp only returns if there is an error
                perror("error");
                exit(EXIT_FAILURE);
            }
            else // Parent
            {
                wait(NULL);
            }
        }
    }

    return 0;
}
