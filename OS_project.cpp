// g++ -o out B.cpp -lpthread
// sleep -> s
// usleep -> ms
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <time.h>

using namespace std;

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

pthread_t thread_[5];
int cups = 0;
int leave[4] = { 100 };//B,M,S,C
int material[4] = { 0 };
char material_name[] = { 'B','M','S','C' };
string name[] = { "咖啡豆","牛奶","焦糖","肉桂" };
bool other_wait = false;
bool B_wait_M = false;

void* child_CM(void* data)
{
	int index;
	srand(time(NULL));
	while (cups < 100)
	{
		do {
			index = rand() % 4;
		} while (material[index] == 1);
		pthread_mutex_lock(&mutex1);
		if (cups < 100)
		{
			material[index] = 1;
			cout << "CM: " << material_name[index] << endl;
		}
		pthread_mutex_unlock(&mutex1);
	}
	pthread_exit(NULL);
}

void* child_FC(void* data)
{
	int curIndex = 0;
	char cur;
	cur = *(char*)data;
	int need[4] = { 0 };
	for (int i = 0; i < 4; i++)
		if (cur == material_name[i])
		{
			need[i] = -1;
			curIndex = i;
			leave[i] = 100;
			break;
		}

	while (cups < 100)
	{
		if (B_wait_M && curIndex == 0)
		{
			if (leave[0] <= leave[1])
				continue;
		}
		for (int i = 0; i < 4; i++)
			need[i] = 0;
		need[curIndex] = -1;
		pthread_mutex_lock(&mutex1);
		for (int i = 0; i < 4; i++)
		{
			if (need[i] != -1 && material[i])
				need[i] = 1;
		}
		pthread_mutex_unlock(&mutex1);
		bool make_coffee = true;
		for (int i = 0; i < 4; i++)
			if (need[i] == 0)
			{
				make_coffee = false;
				break;
			}

		if (make_coffee)
		{
			make_coffee = false;
			pthread_mutex_lock(&mutex1);
			if (!other_wait)
			{
				make_coffee = true;
				other_wait = true;
			}
			pthread_mutex_unlock(&mutex1);
			if (make_coffee)
			{
				pthread_mutex_lock(&mutex1);
				for (int i = 0; i < 4; i++)
					if (need[i] == 1)
						material[i] = 0;
				cups++;
				leave[curIndex]--;
				cout << "FC(" << material_name[curIndex] << "): 第 " << cups << " 杯特製咖啡 " << endl;
				other_wait = false;
				pthread_mutex_unlock(&mutex1);
			}
			else
				usleep(5);
		}
		else
			usleep(5);
	}
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
	bool has_B = false,has_M = false;
	int max = 0;
	if (argv[1][0] == '0')
		max = 3;
	if (argv[1][0] == '1')
		max = 4;
	if (argv[1][0] == '2')
	{
		max = 3;
		for (int i = 2; i < argc; i++)
		{
			if (argv[i][0] == 'B')
				has_B = true;
			if (argv[i][0] == 'M')
				has_M = true;
		}
		if (has_B && has_M)
			B_wait_M = true;
	}
	pthread_mutex_init(&mutex1, NULL); //初始化互斥鎖
	//pthread_mutex_init(&mutex2, NULL);
	pthread_create(&thread_[0], NULL, child_CM, NULL);//創CM
	for (int i = 1; i <= max; i++)//創FC
	{
		pthread_create(&thread_[i], NULL, child_FC, argv[i+1]);//創CM
	}

	for (int i = 0; i <= max; i++)
	{
		pthread_join(thread_[i], NULL);
	}
	for (int j = 1; j <= max; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			if (argv[j+1][0] == material_name[i])
				cout << "FC(" << material_name[i] << "): 做出 " << 100 - leave[i] << "杯，剩 " << leave[i] << " 份" << name[i] << endl;

		}
	}
	return 0;
}