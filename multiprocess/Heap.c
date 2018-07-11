#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_QUEUE_SIZE 100
#define PQ_SIZE 5
typedef int KeyType;
typedef int element;
typedef struct
{
	element key; //우선순위(priority)
	int value;			 //부속 데이터 필드 추가
} HeapElement;
//힙 구조체 정의
typedef struct{
	HeapElement heap[MAX_QUEUE_SIZE];
	int size;
} HeapType;

typedef struct DNSTable{
	char ip[30];
	char name[20];
	char alias[20];
	int hit;
}DNSTable;
typedef struct cache{
	char ip[30];
	char name[20];
	char alias[20];
	int hit;
}cache;

//max-heap을 위한 비교함수
int compare(HeapElement a, HeapElement b) {
	if (a.key >= b.key) return -1;
	if (a.key < b.key) return 1;
	return 0;
	//혹은 return b.key – a.key;
}
//힙 초기화 함수

void init_pq(HeapType *H) {
	H->size = 0;
}

//힙에서의 삽입 함수
void insert(HeapType *H, HeapElement item)
{
	int k = ++(H->size);  //힙크기를 1증가시키고, k가 마지막 노드를 가리키게 한다.
						  //힙을 올라가며, 부모노드와 값을 비교하여 필요하면 교환!

	while (k != 1 && compare(H->heap[k / 2], item) >= 0)
	{
		H->heap[k] = H->heap[k / 2];
		k /= 2;
	}
	//k번째가 item의 위치!
	H->heap[k] = item;
	
	printf("insert (%d, %d) \n", item.key, item.value);
	//printf("size : %d\n",k);
}

//힙에서의 삭제 함수
HeapElement remove_pq(HeapType *H) {
	HeapElement item, tmp;
	int parent = 1, child = 2;
	//printf("compare size %d ", H->size);
	if (H->size == 0)
	{
		fprintf(stderr, "An emtpy heap!\n");
		exit(1);
	}
	item = H->heap[1];  //반환할 항목
	tmp = H->heap[H->size];  //마지막 항목을 tmp에 복사
	H->size--;  //힙 사이즈 1감소
	while (child <= H->size)
	{
		//최대힙 :자식노드 중 더 큰 노드(최소힙 : 더 작은 노드)를 child로!!!
		if (child + 1 <= H->size)  //만일 오른쪽 자식도 존재하면
		{
			if (compare(H->heap[child], H->heap[child + 1]) > 0) //만일 오른쪽이면
				child++;
		}
		if (compare(tmp, H->heap[child]) <= 0) break;  //tmp의 자리를 찾음!
		H->heap[parent] = H->heap[child];
		parent = child;
		child *= 2;
	}
	H->heap[parent] = tmp;
	//printf("remove (%d, %d) \n", item.key, item.value);
	return item;
}

//힙에서의 삭제 함수
HeapElement removed(HeapType *H)
{
	HeapElement item, tmp;
	int parent = 1, child =2;
	if(H->size == 0)
	{
		fprintf(stderr, "An emtpy heap!\n");
		exit(1);
	}
	item = H->heap[1];  //반환할 항목
	tmp = H->heap[H->size];  //마지막 항목을 tmp에 복사
	H->size--;  //힙 사이즈 1감소
	while(child <= H->size)
	{
		//최대힙 :자식노드 중 더 큰 노드(최소힙 : 더 작은 노드)를 child로!!!
		if(child + 1 <= H->size)  //만일 오른쪽 자식도 존재하면
		{
			if(compare(H->heap[child], H->heap[child+1]) > 0) //만일 오른쪽이면
				child++;
		}
		if(compare(tmp, H->heap[child]) <= 0) break;  //tmp의 자리를 찾음!
		H->heap[parent] = H->heap[child];
		parent = child;
		child *= 2;
	}
	H->heap[parent] = tmp;
	//printf(" key : %d value : %d\n",item.key,item.value);
	return item;
}

HeapElement minvalue(HeapType *H) {
	return H->heap[1];
}

int is_empty_pq(HeapType H) {

	return (H.size == 0);
}
int line_check()
{
	int Num_of_Line = 0;
	char ch;

	FILE *f;

	if((f=fopen("KeyValue.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while (1) {// 반복문을 시작합니다. 
		ch = (char)fgetc(f);

		if (ch == '\n') Num_of_Line++;  //한줄씩 띄어질경우 줄수 증가한다.

		if (feof(f) != 0) // 파일을 끝까지 읽었는지 체크합니다. 
			break;
	}
	fclose(f);
	//printf("Num_Of_Line : %d\n",Num_of_Line);
	return Num_of_Line;
}


void load_keyvalue(int line,HeapElement *e)
{
	
	int i = 0;
	FILE *f;
	if((f=fopen("KeyValue.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==line)
		   break;
		fscanf(f,"%d %d",&e[i].value,&e[i].key);
		i++;
	}

	
	fclose(f);
}
void load_all_list(DNSTable* nt,int line)
{
	int i = 0;
	FILE *f;
	if((f=fopen("addr.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==line)
		   break;
		fscanf(f,"%17s%12s%19s%3d", nt[i].name, nt[i].alias, nt[i].ip, &nt[i].hit);
		i++;
	}

	
	fclose(f);
}
void Heaping(int line,DNSTable* dns,cache* ch)
{
	HeapElement h[line];
	HeapType Heap;

	init_pq(&Heap);


	load_keyvalue(line,h);
	int i;
	int key =0;
	
	printf("h[0].key =%d\n",h[0].key);
	for(i= 0; i<line; i++)
	{
		key = minvalue(&Heap).key;
		insert(&Heap,h[i]);
	}
	i =0;
	int index;
	
	for(int i =0; i<PQ_SIZE; i++)
	{
		index =minvalue(&Heap).value;
		strcpy(ch[i].name,dns[index].name);
		strcpy(ch[i].alias,dns[index].alias);
		strcpy(ch[i].ip,dns[index].ip);
		ch[i].hit = dns[index].hit;
		remove_pq(&Heap);
	}

	
}

int main(int argc,char **argv)
{
	int line = line_check();
	DNSTable dns[line];    //공용 DNS 테이블
	cache ch[PQ_SIZE];     //Cache용 테이블
	//DNS테이블 업데이트용
	load_all_list(dns,line);

	
	for(int i =0; i<line; i++)
	{
		printf("%17s%12s%19s%3d\n", dns[i].name, dns[i].alias, dns[i].ip, dns[i].hit);
	}
	
	//PQ만들어주기!!
	Heaping(line,dns,ch);
	for(int i =0; i<PQ_SIZE; i++)
	{
		printf("%17s%12s%19s%3d\n", ch[i].name, ch[i].alias, ch[i].ip, ch[i].hit);
	}
	printf("line_check:%d\n",line_check());

	return 0;
}
