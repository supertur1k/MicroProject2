#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <thread>

using namespace std;


/*
* Замилов А.Р. БПИ193 6 вариант
* Есть три процесса-курильщика и один процесс-посредник. Курильщик
* непрерывно скручивает сигареты и курит их. Чтобы скрутить сигарету, нужны
* табак, бумага и спички. У одного процесса курильщика есть табак, у второго –
* бумага, а у третьего – спички. Посредник кладет на стол по два разных случайных
* компонента. Тот процесс курильщик, у которого есть третий компонент, забирает
* компоненты со стола, скручивает сигарету и курит. Посредник дожидается, пока
* курильщик закончит, затем процесс повторяется. Создать многопоточное
* приложение, моделирующее поведение курильщиков и посредника.
*/

vector<int> smoking_table;
sem_t filling; // семафор посредника
sem_t smoking; // семафор курильщика
pthread_mutex_t mtx; 
bool notSmoking = false; // индикатор для продолжения выполнения программы (завершен ли процесс?)
const int n_smokers = 3; 

string getName(int i) {
	if (i == 1) return "табак";
	if (i == 2) return "бумага";
	if (i == 3) return "спички";
	return "exc";
}

string getName2(int i) {
	if (i == 1) return "табак";
	if (i == 2) return "бумага";
	if (i == 3) return "спички";
	return "exc";
}

bool tableIsNotClear(vector<int> items, int item)
{
	for (int i : items)
	{
		if (item == i)
		{
			return true;
		}
	}
	return false;
}

// Процесс курения
void* smokingProcess(void* param)
{
	int smokerNumber = *((int*)param);
	while (!notSmoking)
	{
		sem_wait(&smoking);
		if (smoking_table.size() == 0 || tableIsNotClear(smoking_table, smokerNumber))
		{
			continue;
		}
		pthread_mutex_lock(&mtx);
		cout << "Курильщик " << smokerNumber << " делает сигарету и курит ее. На столе " << getName2(smoking_table[0]) << " и " << getName2(smoking_table[1]) << endl;
		this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 1000));
		sem_post(&filling);
		pthread_mutex_unlock(&mtx);
	}
	return nullptr;
}

// Процесс заполнения стола
void* intermediary(void* param)
{
	int n = *((int*)param);
	for (int i = 0; i < n; i++)
	{
		int first = 1 + rand() % 3;
		int second = 1 + rand() % 3;
		if (first == second)
		{
			if (first == 1)
			{
				second = rand() % 2 + 2;
			}
			if (first == 2)
			{
				if (rand() % 2 == 0) second = 1;
				else second = 3;
			}
			if (first == 3)
			{
				second = 1 + rand() % 2;
			}
		}
		smoking_table.clear();
		smoking_table.push_back(first);
		smoking_table.push_back(second);
		cout << "Посредник ждет. На столе " << getName(smoking_table[0]) << " и " << getName(smoking_table[1]) << endl;
		this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 1000));
		sem_post(&smoking);
		sem_post(&smoking);
		sem_post(&smoking);
		sem_wait(&filling);
	}
	notSmoking = true;
	smoking_table.clear();
	sem_post(&smoking);
	sem_post(&smoking);
	sem_post(&smoking);
	return nullptr;
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	int N;
	cout << "Введите количество повторов . . . " << endl;
	cin >> N;
	if (N < 0 || N >= INT_MAX)
	{
		cout << "Некорректный ввод. " << endl;
		return 0;
	}
	sem_init(&filling, 0, 0);
	sem_init(&smoking, 0, 0);

	pthread_mutex_init(&mtx, NULL);

	pthread_t mediary;
	pthread_create(&mediary, NULL, intermediary, (void*)&N);

	pthread_t smokers[n_smokers - 1];
	int thread_nums[n_smokers];
	for (int i = 0; i < n_smokers - 1; i++)
	{
		thread_nums[i] = i + 1;
		pthread_create(&smokers[i], NULL, smokingProcess, (void*)&thread_nums[i]);
	}
	thread_nums[n_smokers - 1] = n_smokers;

	smokingProcess((void*)&thread_nums[n_smokers - 1]);
	return 0;
}