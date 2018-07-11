#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct DNSTable{
	char ip[30];
	char name[20];
	char alias[20];
	int hit;
}DNSTable;

typedef char* KeyType;
typedef DNSTable ValueType;

typedef struct tagNode{

	KeyType Key;			
	ValueType Value;			//\BD\C7\C1\A6 \B0\AA
	struct tagNode* NextNode;	//\B4\D9\C0\BD \B3\EB\B5\E5
		
}Node;



typedef Node* LinkedList;

typedef struct tagHashTable{

	LinkedList* Table;
	int TableSize;
}HashTable;


HashTable* CreateHash(int TableSize);								/* \C7콬 \C5\D7\C0̺\ED \B8\B8\B5\E9\B1\E2 */
int Hash_Function(KeyType _key, int TableSize);						/* \B9\AE\C0ڿ\AD \C7ؽ\AC \C7Լ\F6 */
int Hash_Function(KeyType _key, int TableSize);						/* \B9\AE\C0ڿ\AD \C7ؽ\AC \C7Լ\F6 */
Node* CreateNode(KeyType Key, ValueType Value);						/* \B3\EB\B5\E5 \B8\B8\B5\E9\B1\E2 */
void Data_set(HashTable* _HashTable, KeyType Key, ValueType Value);	/* \B5\A5\C0\CC\C5\CD \B3ֱ\E2 */
ValueType Data_get(HashTable* _HashTable, KeyType Key);				/* \B5\A5\C0\CC\C5\CD ã\B1\E2 */

void main(){

	HashTable* HashTable = CreateHash(100);

	DNSTable d[5];
	strcpy(d[0].ip,"123.23.233.23");
	strcpy(d[0].name,"123.23.233.23");
	strcpy(d[0].alias,"123.23.233.23");
	d[0].hit = 0;



	Data_set(HashTable, d[0].ip, d[0]);
	//Data_set(HashTable, "JAVA", "Sun Microsystems");
	//printf("Key : %s, Value : %s\n", "NOVL", Data_get(HashTable, "NOVL"));


}

HashTable* CreateHash(int TableSize){

	HashTable* NewHash = (HashTable*)malloc(sizeof(HashTable));
	NewHash->Table = (LinkedList*)malloc(sizeof(LinkedList)*TableSize);
	NewHash->TableSize = TableSize;

	memset(NewHash->Table,0, sizeof(LinkedList) * TableSize);

	return NewHash;
}

int Hash_Function(KeyType _key, int TableSize){

	int i = 0, KeyValue = 0;
	int KeyLength = strlen(_key);

	for(i=0; i<KeyLength; i++){
		
		KeyValue = (KeyValue << 3) + _key[i];

	}

	return KeyValue%TableSize;
}

Node* CreateNode(KeyType Key, ValueType Value){

	Node* NewNode = (Node*)malloc(sizeof(Node));
	NewNode->Key = Key;
	NewNode->Value = Value;
	NewNode->NextNode = NULL;

	return NewNode;
}

void Data_set(HashTable* _HashTable, KeyType Key, ValueType Value){

	int Address = Hash_Function(Key, _HashTable->TableSize);
	Node* NewNode = CreateNode(Key, Value);

	if(_HashTable->Table[Address] == NULL){
		_HashTable->Table[Address] = NewNode;
	}
	else{
		NewNode->NextNode = _HashTable->Table[Address];
		_HashTable->Table[Address] = NewNode;

	}

	

}

ValueType Data_get(HashTable* _HashTable, KeyType Key){

	int Address = Hash_Function(Key, _HashTable->TableSize);
	
	LinkedList List = _HashTable->Table[Address];
	
	if(List == NULL)
		return NULL;

	while(List != NULL){

		if(strcmp(List->Key, Key) == 0)
			break;

		List = List->NextNode;

	}


	return List->Value;

}
