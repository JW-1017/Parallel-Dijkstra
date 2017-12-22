// hpe_finalproject.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <stack>
#include <vector>
#include <omp.h>

#define MAX_VALUE 2147483647			

using namespace std;

class Vertex {
public:
	int index;
	int dist;
	Vertex() {}
	Vertex(int index, int dist) :index(index), dist(dist) {
	}
	Vertex(int index) :index(index) {
		dist = MAX_VALUE;
	}
};
class Graph {
public:
	int num = 0;
	int* dist;
	int* adj_size;
	vector<Vertex> vt;
	vector<pair<int, int>>* adj;
	Vertex* pq;
	int* m;

	void setStart(int index) {
		vt[index].dist = 0;
	}
	void addVertex(int index) {
		vt.push_back(Vertex(index));
		num++;
	}
	void setAdj() {
		adj = new vector<pair<int, int>>[num];
	}
	void addAdj(int u, int v, int w) {
		adj[u].push_back(make_pair(v, w));
	}
	void setAdjAndDist() {							// �̸� �ε��� �� ����, ���� �ʱ�ȭ
		pq = new Vertex[num * 2];					// �˳��ϰ� ������� ���� ���� �幮 ��� ������ ���� �� ���� �̶� ������ num * 2 �κ��� num * �� ū ���ڷ� ����
		dist = new int[num];
		for (int i = 0; i < num; i++) {
			dist[i] = MAX_VALUE;
		}
		adj_size = new int[num];
		for (int i = 0; i < num; i++) {
			adj_size[i] = adj[i].size();
		}
	}
	void dijkstra(int s) {						// csv������ ��Ī
		int end = 1;		// ������ ���� ���
		setAdjAndDist();
		dist[s] = 0;
		setStart(s);
		pq[0] = vt[s];
		m = new int[num];
		m[s] = -1;

		int min_dist;
		int min_index;
		int index, temp, cost, size;
		double total_begin, total_end;

		// ������� ���� Ž��
		total_begin = omp_get_wtime();
		while (end) {
			min_dist = pq[0].dist;
			min_index = 0;
			#pragma omp parallel for reduction(min : min_dist)
			for (int i = 1; i < end; i++) {
				if (min_dist > pq[i].dist) {
					min_dist = pq[i].dist;
					min_index = i;
				}
			}
			index = pq[min_index].index;
			temp = dist[index];
			cost = min_dist;
			pq[min_index] = pq[end - 1];
			end--;
			if (temp < cost) {
				continue;
			}
			size = adj_size[index];					// �̸� �ε��� ����
			#pragma omp parallel for	 				// stl�� omp parallel for �Ҿ���
			for (int i = 0; i < size; i++) {
				int n = adj[index][i].first;
				int v = adj[index][i].second + temp;
				if (dist[n] > v) {
					dist[n] = v;
					#pragma omp critical(for_queue)	
					{
						pq[end++] = Vertex(n, v);		// stl ��� �迭�� �Ἥ ����ȿ��(�޸� ���Ҵ��� ���� ����)
					}
					m[n] = index;
				}
			}
		}
		total_end = omp_get_wtime();
		cout << "\ntime : " << total_end - total_begin << " sec\n" << endl;
	}
	void printIndexMinDis(int index) {
		//�ε����� �ּҰŸ�
		if (dist[index] != MAX_VALUE) {
			cout << dist[index] << "\n";
		}
		else {
			cout << "MAX\n";
		}
	}
	void printMinRoute(int index) {
		if (dist[index] != MAX_VALUE) {
			stack<int> st;
			int next = m[index];
			while (next != -1) {
				st.push(next);
				next = m[next];
			}
			while (!st.empty()) {
				printf("%d - ", st.top());
				st.pop();
			}
			printf("%d\n", index);
		}
	}
};
int _tmain(int argc, _TCHAR* argv[])
{
	int i = -1, j = 0;
	int start, end;
	Graph gp;
	ifstream ifs;
	ifs.open("mapDist10000.csv");

	string temp = "";
	getline(ifs, temp);				// ù���� ����� ���� ��� �߰����� ���� ���
	char* c_temp;
	char* cstr = new char[temp.length() + 1];
	strcpy(cstr, temp.c_str());				// strtok�� Ȱ���� ����
	strtok(cstr, " ,\n");
	gp.addVertex(j++);
	while (strtok(NULL, " ,\n") != NULL) {
		gp.addVertex(j++);
	}
	delete[] cstr;

	gp.setAdj();
	while (getline(ifs, temp))
	{
		i++;
		j = 0;
		char* cstr = new char[temp.length() + 1];
		strcpy(cstr, temp.c_str());				// strtok�� Ȱ���� ����

		strtok(cstr, ",\n");
		c_temp = strtok(NULL, ",\n");
		while (c_temp != NULL) {
			if (c_temp[0] == 'M' || c_temp[0] == '0') {}
			else {
				string str(c_temp);
				gp.addAdj(i, j, stoi(str));						// visual studio 2013�� �����ϴ� �Լ�(stoi)
			}
			j++;
			c_temp = strtok(NULL, ",\n");
		}
		delete[] cstr;
	}
	ifs.close();

	omp_set_num_threads(4);				// 4���� ����

	cout << "The Shortest Path Algorithm < DIJKSTRA'S ALGORITHM >\n" << endl;
	cout << "Enter The source : ";
	cin >> start;
	cout << "Enter The target : ";
	cin >> end;

	gp.dijkstra(start);
	gp.printMinRoute(end);
	cout << "\nThe Shortest Path : ";
	gp.printIndexMinDis(end);

	return 0;
}