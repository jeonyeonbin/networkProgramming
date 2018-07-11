#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define BUF_SIZE 30
typedef struct userInfo{
	int num;
	char id[10];
	char pwd[10];
}userInfo;
typedef struct cache{
	char ip[30];
	char name[20];
	char alias[10];
	int hit;
}cache;

void clearBufferRead() 		    //버퍼에잇는거지우기
{
	 while(getchar() !='\n');
}
int search_cache_Domain(cache* nt,char* domain);
void caching(FILE *f,cache* nt);
void searchAll(cache* nt);
void searchDomain(cache* nt,char* domain);
int search_cache_IP(cache* nt,char* IP);
int one_login(userInfo* user);
void userSearch(FILE *f,userInfo* user);
int login_user_admin(char* id,char* pwd,userInfo* user);
void userMenu(cache* ch);
void adminMenu();
void clnt_receive_menu(int num,cache* ch);

int main(int argc,char* argv[])
{
	FILE* fcache;
	FILE* fuser;

	cache ch[5];
	userInfo user[5];

	//캐싱하기
	caching(fcache,ch);
	//user불러오기
	userSearch(fuser,user);

	while(1){
		clnt_receive_menu(one_login(user),ch);
	}

	return 0;
}

//파일에잇는 모든 회원들을 끌어와서 사용
//userInfo구조체에 저장
void userSearch(FILE *f,userInfo* user)
{
	int i = 0;
	
	if((f=fopen("userInfo.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==5)
		   break;
		fscanf(f,"%d %s %s", &user[i].num, user[i].id, user[i].pwd);
		i++;
	}

	
	fclose(f);
}

//id와 비밀번호 입력시에 Admin or User or NoN-user인지 판별
//0이면 admin 1이면 회원 -1이면 비회원
int login_user_admin(char* id,char* pwd,userInfo* user)
{
	for(int i =0; i<5; i++)
	{	
		if(strcmp(id,user[i].id)==0 && strcmp(pwd,user[i].pwd)==0){
			if(user[i].num==0)
				return 0;
			else 
				return 1;			
		}
	}
	return -1;	

}

//로그인시 아이디비번 입력
int one_login(userInfo* user)
{
	char id[10];
	char pwd[10];
	printf("id:");
	scanf("%s",id);
	clearBufferRead();
	printf("pwd:");
	scanf("%s",pwd);

	return login_user_admin(id,pwd,user);
}

//userMenu
void userMenu(cache* ch)
{
	int num;
	char buf[BUF_SIZE];
	printf("===============================================\n");
	printf("1. ip로 검색 2. 도메인으로 검색 3.top 10 출력\n");
	printf("================================================\n");
	printf("번호 입력:");
	scanf("%d",&num);
	clearBufferRead();
	
	if(num ==1){
		scanf("%s",buf);
		printf("buf : %s\n",buf);
		if(search_cache_IP(ch,buf)==0)
			printf("없습니다\n");
	}else if(num==2){
		memset(buf,0,BUF_SIZE); //먼저버퍼 초기화
		scanf("%s",buf);
		printf("buf : %s\n",buf);
		if(search_cache_Domain(ch,buf)==0)
			printf("없습니다\n");		
	}else if(num==3){
		searchAll(ch);
	}
		

}
//adminMenu
void adminMenu()
{
	int num;
	printf("======================================================================\n");
	printf("1.수정 ip주소 2.수정 도메인주소 3.수정 회원정보 4.추가 ip주소 5.top 10 출력\n");
	printf("======================================================================\n");

}

//메뉴를 클라이언트한테 출력하는것
void clnt_receive_menu(int num,cache* ch)
{
	if(num ==0)
		adminMenu();
	else if(num == 1)
		userMenu(ch);
	else
		printf("아이디와비밀번호가맞지않습니다!!!\n");

}

//cache하기
void caching(FILE *f,cache* nt)
{
	int i = 0;
	
	if((f=fopen("addr.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==5)
		   break;
		fscanf(f,"%s\t%s\t%s\t%d", nt[i].name, nt[i].alias, nt[i].ip, &nt[i].hit);
		i++;
	}

	
	fclose(f);
}


//캐시에잇는 모든것들 출력
void searchAll(cache* nt)
{
	for(int i=0;i<5;i++){
			printf("%s\t%s\t%s\t%d\n", nt[i].name, nt[i].alias, nt[i].ip, nt[i].hit);

	}
}

//캐시에있는거 도메인 
//없으면 0 메모리에서 찾아야댄다!!
int search_cache_Domain(cache* nt,char* domain)
{
	for(int i=0;i<5;i++){
		if(strcmp(nt[i].name,domain)==0){
			printf("%s\t%s\t%s\t%d\n", nt[i].name, nt[i].alias, nt[i].ip, nt[i].hit);
			return 1;
		}
	}
	return 0;
}


//캐시에있는 ip
//없으면 0 메모리에서 찾아야댄다!!
int search_cache_IP(cache* nt,char* IP)
{
	for(int i=0;i<5;i++){
		if(strcmp(nt[i].ip,IP)==0){
			printf("%s\t%s\t%s\t%d\n", nt[i].name, nt[i].alias, nt[i].ip, nt[i].hit);
			return 1;
		}
	}
	return 0;
}
