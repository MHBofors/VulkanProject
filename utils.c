//
//  utils.c
//  vkProject
//
//  Created by Markus Höglin on 2023-07-20.
//

#include "utils.h"

uint32Queue *allocQueue(void)
{
    uint32Queue *queue = malloc(sizeof(uint32Queue));
    queue->size = 0;
    queue->root = NULL;
    return queue;
}

void enqueue(uint32Queue *queue, uint32_t value)
{
    queueNode *newNode = malloc(sizeof(queueNode));
    newNode->value = value;
    if(queue->root == NULL)
    {
        queue->root = newNode;
        newNode->next = newNode;
    }
    else
    {
        newNode->next = queue->root->next;
        queue->root->next = newNode;
        queue->root = newNode;
    }
    queue->size++;
}

uint32_t dequeue(uint32Queue *queue)
{
    queueNode *root = queue->root;
    queueNode *head = root->next;
    uint32_t value = head->value;
    
    if(head == root)
    {
        free(queue->root);
        queue->root = NULL;
    }
    else
    {
        queue->root->next = head->next;
        free(head);
    }
    
    queue->size--;
    return value;
}

uint32Tree *allocTree(void)
{
    uint32Tree *newTree = malloc(sizeof(uint32Tree));
    newTree->size = 0;
    newTree->root = NULL;
    return newTree;
}

void freeTree(uint32Tree *pTree)
{
    freeNode(pTree->root);
    free(pTree);
}

void freeNode(node *pNode)
{
    if(pNode != NULL)
    {
        freeNode(pNode->left);
        freeNode(pNode->right);
        free(pNode);
    }
}

node *allocNode(uint32_t value)
{
    node *newNode = malloc(sizeof(node));
    if(newNode != NULL)
    {
        newNode->value = value;
        newNode->left = NULL;
        newNode->right = NULL;
        return newNode;
    }
    else
    {
        printf("\nfailed to allocate memory");
        exit(1);
    }
}

void insert(uint32Tree *pTree, uint32_t value)
{
    if(pTree->root == NULL)
    {
        pTree->root = allocNode(value);
        pTree->size++;
    }
    else
    {
        pTree->size += insertRecursion(pTree->root, value);
    }
}

uint32_t insertRecursion(node *pNode, uint32_t value)
{
    if(value < pNode->value)
    {
        if(pNode->left == NULL)
        {
            pNode->left = allocNode(value);
            return 1;
        }
        else
        {
            return insertRecursion(pNode->left, value);
        }
    }
    if(pNode->value < value)
    {
        if(pNode->right == NULL)
        {
            pNode->right = allocNode(value);
            return 1;
        }
        else
        {
            return insertRecursion(pNode->right, value);
        }
    }
    return 0;
}

void toArray(uint32Tree *pTree, uint32_t array[pTree->size])
{
    uint32Queue *queue = allocQueue();
    toQueue(pTree->root, queue);
    queueToArray(queue, array);
    freeTree(pTree);
}

void toQueue(node *pNode, uint32Queue *queue)
{
    if(pNode != NULL)
    {
        toQueue(pNode->left, queue);
        enqueue(queue, pNode->value);
        toQueue(pNode->right, queue);
    }
}

void queueToArray(uint32Queue *queue, uint32_t *array)
{
    uint32_t size = queue->size;
    for(int i = 0; i < size; i++)
    {
        array[i] = dequeue(queue);
    }
    free(queue);
}

uint32_t boundU32(uint32_t value, uint32_t lower, uint32_t upper)
{
    if(value < lower)
    {
        return lower;
    }
    else
    {
        if(upper < value)
        {
            return upper;
        }
        else
        {
            return value;
        }
    }
}

size_t readFile(const char *fileName, char **buffer)
{
    FILE *pFile = fopen(fileName, "rb");
    
    if(pFile == NULL)
    {
        printf("Failed to open file: %s!\n", fileName);
        exit(1);
    }

    size_t fileSize;
    
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    
    *buffer = malloc(fileSize);

    size_t bytesRead = fread(*buffer, 1, fileSize, pFile);
    if(fileSize != bytesRead)
    {
        printf("Failed to read file: %s!\nMissing %lu bytes", fileName, fileSize-bytesRead);
        exit(1);
    }
    fclose(pFile);
    
    return fileSize;
}
