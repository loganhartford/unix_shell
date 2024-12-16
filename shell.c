#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 1000
#define MAX_TOKENS 100
#define TOKEN_ERROR ((char *)-1)

char *custom_strtok(char *str, const char *delim, char **saveptr)
{
    char *token;
    int in_quotes = 0;
    char quote_char = '\0';

    /* If str is NULL, start from where we left off */
    if (str == NULL)
    {
        str = *saveptr;
    }

    /* Skip leading delimiters */
    while (*str && strchr(delim, *str))
    {
        str++;
    }

    /* No more tokens */
    if (*str == '\0')
    {
        *saveptr = str;
        return NULL;
    }

    token = str;

    /* Find the end of the token */
    while (*str)
    {
        /* Toggle in_quotes if we find a quote character */
        if (*str == '"' || *str == '\'')
        {
            /* End of quoted section */
            if (in_quotes && *str == quote_char)
            {
                in_quotes = 0;
                *str = '\0'; /* Remove the closing quote */
            }
            /* Start of quoted section */
            else if (!in_quotes)
            {
                in_quotes = 1;
                quote_char = *str;
                token = str + 1; /* Start the token after the quote */
            }
        }
        /* If we are not in quotes and we find a delimiter, break */
        else if (!in_quotes && strchr(delim, *str))
        {
            break;
        }
        str++;
    }

    /* Detect mismatched quotes */
    if (in_quotes)
    {
        fprintf(stderr, "error: mismatched quotes\n");
        return TOKEN_ERROR;
    }

    /* Terminate the token */
    if (*str)
    {
        *str = '\0';
        str++;
    }

    /* Save the position for the next call */
    *saveptr = str;
    return token;
}

int main(void)
{
    char input[MAX_INPUT_LENGTH];

    while (1)
    {
        /* Use stderr for the prompt to keep it separate from command output */
        fprintf(stderr, "$ ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            /* EOF received, exit */
            break;
        }

        /* Check if the input length exceeds the buffer size */
        if (input[strlen(input) - 1] != '\n' && !feof(stdin))
        {
            fprintf(stderr, "error: input too long\n");
            /* Clear the input buffer */
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            continue;
        }

        /* Properly terminate the input string */
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        if (strncmp(input, "exit", 4) == 0)
        {
            break;
        }

        /* Tokenize the input */
        char *tokens[MAX_TOKENS];
        int token_count = 0;
        char *saveptr;
        char *token = custom_strtok(input, " ", &saveptr);
        while (token != NULL && token != TOKEN_ERROR && token_count < MAX_TOKENS)
        {
            tokens[token_count++] = token;
            token = custom_strtok(NULL, " ", &saveptr);
        }
        tokens[token_count] = NULL;

        /* If there was a mismatched quote error, skip execution */
        if (token == TOKEN_ERROR)
        {
            continue;
        }

        /* If the number of tokens exceeds MAX_TOKENS, print an error and skip execution */
        if (token_count >= MAX_TOKENS)
        {
            fprintf(stderr, "error: too many arguments\n");
            continue;
        }

        /* Execute the command */
        if (token_count > 0)
        {
            /* Handle cd command */
            if (strcmp(tokens[0], "cd") == 0)
            {
                if (token_count < 2)
                {
                    fprintf(stderr, "error: cd requires a directory argument\n");
                }
                else if (chdir(tokens[1]) != 0)
                {
                    fprintf(stderr, "error: cd failed\n");
                }
                continue;
            }

            pid_t pid = fork();
            if (pid == 0) /* Child */
            {
                execvp(tokens[0], tokens);
                /* execvp only returns if there is an error */
                perror("error");
                exit(1);
            }
            else /* Parent */
            {
                int child_status;
                waitpid(pid, &child_status, 0);
                if (WIFEXITED(child_status) && WEXITSTATUS(child_status) != 0)
                {
                    fprintf(stderr, "error: command exited with code: %d\n", WEXITSTATUS(child_status));
                }
            }
        }
    }

    return 0;
}
