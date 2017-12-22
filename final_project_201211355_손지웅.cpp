// hpe_finalproject.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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
	void setAdjAndDist() {							// 미리 인덱스 다 저장, 길이 초기화
		pq = new Vertex[num * 2];					// 넉넉하게 잡았지만 아주 극히 드문 경우 문제가 생길 수 있음 이때 간단히 num * 2 부분을 num * 더 큰 숫자로 변경
		dist = new int[num];
		for (int i = 0; i < num; i++) {
			dist[i] = MAX_VALUE;
		}
		adj_size = new int[num];
		for (int i = 0; i < num; i++) {
			adj_size[i] = adj[i].size();
		}
	}
	void dijkstra(int s) {						// csv파일은 대칭
		int end = 1;		// 마지막 다음 요소
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

		// 여기부터 실제 탐색
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
			size = adj_size[index];					// 미리 인덱스 저장
			#pragma omp parallel for	 				// stl은 omp parallel for 불안전
			for (int i = 0; i < size; i++) {
				int n = adj[index][i].first;
				int v = adj[index][i].second + temp;
				if (dist[n] > v) {
					dist[n] = v;
					#pragma omp critical(for_queue)	
					{
						pq[end++] = Vertex(n, v);		// stl 대신 배열을 써서 빠른효과(메모리 재할당을 하지 않음)
					}
					m[n] = index;
				}
			}
		}
		total_end = omp_get_wtime();
		cout << "\ntime : " << total_end - total_begin << " sec\n" << endl;
	}
	void printIndexMinDis(int index) {
		//인덱스별 최소거리
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
	getline(ifs, temp);				// 첫줄은 노드의 나열 노드 추가만을 위해 사용
	char* c_temp;
	char* cstr = new char[temp.length() + 1];
	strcpy(cstr, temp.c_str());				// strtok의 활용을 위해
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
		strcpy(cstr, temp.c_str());				// strtok의 활용을 위해

		strtok(cstr, ",\n");
		c_temp = strtok(NULL, ",\n");
		while (c_temp != NULL) {
			if (c_temp[0] == 'M' || c_temp[0] == '0') {}
			else {
				string str(c_temp);
				gp.addAdj(i, j, stoi(str));						// visual studio 2013에 존재하는 함수(stoi)
			}
			j++;
			c_temp = strtok(NULL, ",\n");
		}
		delete[] cstr;
	}
	ifs.close();

	omp_set_num_threads(4);				// 4개로 정함

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