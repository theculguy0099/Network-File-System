#include "naming_server.h"

int value_of_index(char c)
{
    if (c >= 'A' && c <= 'z')
    {
        return c - 'A';
    }
    else if (c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }
    else if (c == '/')
    {
        return 62;
    }
    else if (c == '_')
    {
        return 63;
    }
    else if (c == '-')
    {
        return 64;
    }
    else if (c == '.')
    {
        return 65;
    }
}
PtrTrie MakeNode()
{
    PtrTrie temp;
    temp = (PtrTrie)malloc(sizeof(trienode));
    temp->end_flag = 0;
    temp->ss_id = -1;
    for (int i = 0; i < 66; i++)
    {
        temp->Children[i] = NULL;
    }
    return temp;
}

Trie Insert(Trie T, char *str, int ss_id)
{
    if (T == NULL)
    {
        T = MakeNode();
    }
    int length = strlen(str);
    PtrTrie trav;
    trav = T;
    for (int i = 0; i < length; i++)
    {
        if (trav->Children[value_of_index(str[i])] == NULL)
        {
            trav->Children[value_of_index(str[i])] = MakeNode();
        }
        trav = trav->Children[value_of_index(str[i])];
    }
    trav->end_flag = 1;
    trav->ss_id = ss_id;
    return T;
}

int Search(Trie T, char *str)
{
    if (T == NULL)
    {
        return -1;
    }
    int length = strlen(str);
    PtrTrie trav;
    trav = T;
    for (int i = 0; i < length; i++)
    {
        int index = value_of_index(str[i]);
        if (trav->Children[index] == NULL)
            return -1;
        trav = trav->Children[index];
    }
    if (trav->end_flag == 1)
    {
        return trav->ss_id;
    }
}

void Delete(Trie T, char *str)
{
    int length = strlen(str);
    if (T->end_flag == 1)
    {
        T->end_flag = 0;
        return;
    }
    if (length == 0)
    {
        T->end_flag = 0;
        return;
    }

    Delete(T->Children[value_of_index(str[0])], str + 1);

    int flag = 0;
    PtrTrie temp = T->Children[value_of_index(str[0])];
    for (int i = 0; i < 66; i++)
    {
        if (temp->Children[i] != NULL)
        {
            flag = 1;
        }
    }
    if (flag == 0)
    {
        T->Children[value_of_index(str[0])] = NULL;
        free(temp);
        return;
    }
}