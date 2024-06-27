#include "client.h"
#include "read.h"
#include "write.h"
#include "retrieve.h"
#include "makedir.h"
#include "delete.h"
#include "copy.h"
int main()
{
    char command[4096];
    fgets(command, 4096, stdin);
    char *args[MAX_TOKENS];
    int arg_count = 0;

    // Tokenize the command
    char *token = strtok(command, " \t");
    while (token != NULL)
    {
        args[arg_count] = token;
        arg_count++;
        token = strtok(NULL, " \t");
    }
    args[arg_count] = NULL;
    int i = 0;
    while (args[i] != NULL)
    {
        if (!strcmp("CREATE", args[i]))
        {
            makedirs(args);
        }
        else if (!strcmp("READ", args[i]))
        {
            reads(args);
            break;
        }
        else if (!strcmp("WRITE", args[i]))
        {
            writes(args);
            break;
        }
        else if (!strcmp("RETRIEVE", args[i]))
        {
            retrieves(args);
            break;
        }
        else if (!strcmp("DELETE", args[i]))
        {

            deletes(args);
            break;
        }
        else if (!strcmp("COPY", args[i]))
        {

            copys(args);
            break;
        }
        i++;
    }
}
