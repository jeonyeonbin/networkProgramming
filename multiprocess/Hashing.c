#include <stdio.h>

#include <string.h>

#include <stdlib.h>

 

//struct hash *hashTable = NULL;
int eleCount = 0;
struct node {
    char key[100]; 
    int hit;
    char alias[100];
    char name[100];
    struct node *next;

};
struct hash {

    struct node *head;

    int count;

};
struct node * createNode(char* key, char *name, int hit,char *alias) {
    struct node *newnode;
    newnode = (struct node *) malloc(sizeof(struct node));
    strcpy(newnode->key,key);
    newnode->hit = hit;
    strcpy(newnode->name, name);
    strcpy(newnode->alias,alias);
    newnode->next = NULL;
    return newnode;
}

 

void insertToHash(char* key, char *name, int hit,char* alias,struct hash* hashTable) {
    int hashIndex = atoi(key) % eleCount;
    struct node *newnode = createNode(key, name, hit,alias);

    /* head of list for the bucket with index "hashIndex" */

    if (!hashTable[hashIndex].head) {
        hashTable[hashIndex].head = newnode;
        hashTable[hashIndex].count = 1;
        return;

    }
    /* adding new node to the list */

    newnode->next = (hashTable[hashIndex].head);

    /*

     * update the head of the list and no of

     * nodes in the current bucket

     */
    hashTable[hashIndex].head = newnode;
    hashTable[hashIndex].count++;

    return;

}

 

void deleteFromHash(char* key,struct hash* hashTable) {

    /* find the bucket using hash index */
    int hashIndex = atoi(key) % eleCount, flag = 0;

    struct node *temp, *myNode;

    /* get the list head from current bucket */

    myNode = hashTable[hashIndex].head;

    if (!myNode) {
        printf("Given data is not present in hash Table!!\n");

        return;
    }

    temp = myNode;

    while (myNode != NULL) {
        /* delete the node with given key */

        if (strcmp(myNode->key,key)==0) {

            flag = 1;

            if (myNode == hashTable[hashIndex].head)
                hashTable[hashIndex].head = myNode->next;

            else
                temp->next = myNode->next;


            hashTable[hashIndex].count--;

            free(myNode);
            break;
        }

        temp = myNode;

        myNode = myNode->next;

    }

    if (flag)

        printf("Data deleted successfully from Hash Table\n");

    else

        printf("Given data is not present in hash Table!!!!\n");

    return;

}

 

void searchInHash(char* key,struct hash* hashTable) {

    int hashIndex = atoi(key) % eleCount, flag = 0;

    struct node *myNode;

    myNode = hashTable[hashIndex].head;

    if (!myNode) {

        printf("Search element unavailable in hash table\n");

        return;

    }

    while (myNode != NULL) {

        if (strcmp(myNode->key,key)==0) {

            printf("VoterID  : %s\n", myNode->key);

            printf("Name     : %s\n", myNode->name);

            printf("Hit      : %d\n", myNode->hit);
	    printf("Alias    : %s\n", myNode->alias);

            flag = 1;

            break;

        }

        myNode = myNode->next;

    }

    if (!flag)

        printf("Search element unavailable in hash table\n");

    return;

}

 

void display(struct hash* hashTable) {

    struct node *myNode;

    int i;

    for (i = 0; i < eleCount; i++) {

        if (hashTable[i].count == 0)

            continue;

        myNode = hashTable[i].head;

        if (!myNode)

            continue;

        printf("\nData at index %d in Hash Table:\n", i);

        printf("VoterID     Name          Age       alias\n");

        printf("------------------------------------------\n");

        while (myNode != NULL) {

            printf("%-12s", myNode->key);

            printf("%-15s", myNode->name);

            printf("%d", myNode->hit);
	    printf("%12s\n", myNode->alias);

            myNode = myNode->next;

        }

    }

    return;

}
int main() {

    char key[100];
    int ch, hit;

    char name[100];
    char alias[100];

    eleCount = 100;

    struct hash *hashTable = NULL;
    /* create hash table with "n" no of buckets */

    hashTable = (struct hash *) calloc(100, sizeof(struct hash));
   

    while (1) {

        printf("\n1. Insertion\t2. Deletion\n");

        printf("3. Searching\t4. Display\n5. Exit\n");

        printf("Enter your choice:");

        scanf("%d", &ch);

        switch (ch) {

        case 1:


            break;

 

        case 2:

            printf("Enter the key to perform deletion:");

            scanf("%s", key);

            /* delete node with "key" from hash table */

            deleteFromHash(key,hashTable);

            break;
 

        case 3:

            printf("Enter the key to search:");

            scanf("%s", key);

            searchInHash(key,hashTable);

            break;

        case 4:

            display(hashTable);

            break;

        case 5:

            exit(0);

        default:

            printf("U have entered wrong option!!\n");

            break;

        }

    }

    return 0;

}
