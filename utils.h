//
//  utils.h
//  vkProject
//
//  Created by Markus Höglin on 2023-07-20.
//

#ifndef utils_h
#define utils_h

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct Node {
    uint32_t value;
    struct Node *left;
    struct Node *right;
}node;

typedef struct tree {
    uint32_t size;
    node *root;
}uint32Tree;

typedef struct queueNode {
    uint32_t value;
    struct queueNode *next;
}queueNode;

typedef struct queue {
    uint32_t size;
    queueNode *root;
} uint32Queue;

uint32Queue *allocQueue(void);

void enqueue(uint32Queue *queue, uint32_t value);

uint32_t dequeue(uint32Queue *queue);

uint32Tree *allocTree(void);

node *allocNode(uint32_t value);

void freeTree(uint32Tree *pTree);

void freeNode(node *pNode);

void insert(uint32Tree *pTree, uint32_t value);

uint32_t insertRecursion(node *pNode, uint32_t value);

void toArray(uint32Tree *pTree, uint32_t array[pTree->size]);

void toQueue(node *pNode, uint32Queue *queue);

void queueToArray(uint32Queue *queue, uint32_t array[queue->size]);

uint32_t boundU32(uint32_t value, uint32_t lower, uint32_t upper);

#endif /* utils_h */
