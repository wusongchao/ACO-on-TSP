#include <cfloat>
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <map>
#include <algorithm>
#include <fstream>

typedef struct Point {
	int index;
	double x;
	double y;
}Point;

constexpr int MAX_POINT_NUM = 150;

void clearAllowed(bool* allowed, int cityNum)
{
	for (int i = 1;i <= cityNum;i++) {
		allowed[i] = true;
	}
}

int calculateProbabilityAndSelect(const double Tau[][MAX_POINT_NUM], const double heuristicValue[][MAX_POINT_NUM],
	bool allowed[], int currentCityIndex, int cityNum, double alpha, double beta, std::default_random_engine& randomEngine)
{
	std::map<int, double> probability;
	
	double denominatorSum = 0;
	for (int s = 1;s <= cityNum;s++) {
		if (allowed[s] == true) {
			double val = pow(Tau[currentCityIndex][s], alpha)*pow(heuristicValue[currentCityIndex][s], beta);
			probability.insert({ s,val });
			denominatorSum += val;
		}
	}

	int selectedCityIndex = -1;
	if (probability.size() == 1) {
		//means we have no city for choosing
		selectedCityIndex = probability.begin()->first;
	}else {
		double sumSelect = 0;

		std::uniform_real_distribution<double> randomDistribution(0, 1);
		double generatedProbablity = randomDistribution(randomEngine);
		
		for (auto &r : probability) {
			r.second = r.second / denominatorSum;
			sumSelect += r.second;
			if (sumSelect >= generatedProbablity) {
				selectedCityIndex = r.first;
				break;
			}
		}
		selectedCityIndex = selectedCityIndex == -1 ? (--probability.end())->first : selectedCityIndex;
	}

	allowed[selectedCityIndex] = false;

	return selectedCityIndex;
}

void calculateDeltaTauPerAnt(double deltaTau[][MAX_POINT_NUM], int Q, std::vector<int>& path, double antPathLength)
{
	for (int k = 0, pathLength = path.size()-1;k < pathLength;k++) {
		int i = path[k];
		int j = path[k + 1];
		deltaTau[i][j] = deltaTau[j][i] += Q / antPathLength;
	}
}

void constructPath(const double graphMatrix[][MAX_POINT_NUM],const double Tau[][MAX_POINT_NUM], const double heuristicValue[][MAX_POINT_NUM],
	std::vector<int>& path, double& currentDistance, int cityNum, double alpha, double beta, std::default_random_engine& randomEngine, std::uniform_int_distribution<int>& randomDistribution)
{
	bool allowed[MAX_POINT_NUM];
	//double currentDistance = 0;
	clearAllowed(allowed, cityNum);

	int startNum = randomDistribution(randomEngine);
	path.push_back(startNum);
	allowed[startNum] = false;

	int current = startNum;
	while (path.size() < cityNum) {
		int next = calculateProbabilityAndSelect(Tau, heuristicValue, allowed, current, cityNum, alpha, beta, randomEngine);

		//add it to path and update distance
		path.push_back(next);
		currentDistance += graphMatrix[current][next];

		current = next;
	}

	currentDistance += graphMatrix[current][startNum];
}

void updateTau(double Tau[][MAX_POINT_NUM], double deltaTauTotal[][MAX_POINT_NUM], double rho, int cityNum)
{
	for (int i = 1;i <= cityNum;i++) {
		for (int j = 1;j <= cityNum;j++) {
			double val = (1.0 - rho) * Tau[i][j] + deltaTauTotal[i][j];
			Tau[i][j] = val;
		}
	}
}

int main()
{
	std::string filePath;

	std::cin >> filePath;

	std::ifstream in(filePath.c_str());
	if (!in.is_open()) {
		std::cout << "No exist!" << std::endl;
	}

	std::vector<Point> points;
	int index;
	double x, y;
	int cityNum = 0;
	while (in >> index >> x >> y) {
		points.push_back({ index,x,y });
	}

	double graphMatrix[MAX_POINT_NUM][MAX_POINT_NUM];
	double heuristicValue[MAX_POINT_NUM][MAX_POINT_NUM];//启发式因子为两点间距离的倒数

	for (auto &p : points) {
		cityNum++;
		for (auto &pr : points) {
			if (p.index != pr.index) {
				double val = sqrt((p.x - pr.x)*(p.x - pr.x) + (p.y - pr.y)*(p.y - pr.y));
				graphMatrix[p.index][pr.index] = graphMatrix[pr.index][p.index] = val;
				heuristicValue[p.index][pr.index] = heuristicValue[pr.index][p.index] = 1 / val;
			}else {
				graphMatrix[p.index][pr.index] = graphMatrix[pr.index][p.index] = 0;
				heuristicValue[p.index][pr.index] = heuristicValue[pr.index][p.index] = 0;
			}
		}
	}

	constexpr int antNum = 32;
	constexpr int iterTimes = 800;
	double alpha = 1;//信息素的重要程度
	double beta = 3.8;//启发式因子的重要程度
	double rho = 0.7;//挥发系数
	int left = 2;//信息素增量
	int Q = 400;//信息素增加数
	double Tau[MAX_POINT_NUM][MAX_POINT_NUM];//全局的信息素信息
	

	std::vector<int> bestPath(cityNum);
	double bestLength = DBL_MAX;

	for (int i = 1;i <= cityNum;i++) {
		for (int j = 1;j <= cityNum;j++) {
			Tau[i][j] = 1;
		}
	}

	std::default_random_engine randomEngine;
	std::uniform_int_distribution<int> randomDistribution(1, cityNum);

	int iterCounter = 0;
	while (iterCounter < iterTimes) {
		double deltaTauTotal[MAX_POINT_NUM][MAX_POINT_NUM];
		for (int i = 1;i <= cityNum;i++) {
			for (int j = 1;j <= cityNum;j++) {
				deltaTauTotal[i][j] = 0;
			}
		}

		for (int i = 0;i < antNum;i++) {
			
			//for each ant
			//bool allowed[MAX_POINT_NUM];
			std::vector<int> path;
			double currentDistance = 0;
			//clearAllowed(allowed, cityNum);

			constructPath(graphMatrix, Tau, heuristicValue, path, currentDistance, cityNum, alpha, beta, randomEngine, randomDistribution);

			//int startNum = randomDistribution(randomEngine);
			//path.push_back(startNum);
			//allowed[startNum] = false;

			//int current = startNum;
			//while (path.size() < cityNum) {
			//	int next = calculateProbabilityAndSelect(Tau, heuristicValue, allowed, current, cityNum, alpha, beta, randomEngine);
			//	
			//	//add it to path and update distance
			//	path.push_back(next);
			//	currentDistance += graphMatrix[current][next];

			//	current = next;
			//}

			calculateDeltaTauPerAnt(deltaTauTotal, Q, path, currentDistance);
			

			if (currentDistance < bestLength) {
				bestLength = currentDistance;
				std::copy(path.cbegin(), path.cend(), bestPath.begin());
			}
		}
		updateTau(Tau, deltaTauTotal, rho, cityNum);

		iterCounter++;
	}
	for (auto &p : bestPath) {
		std::cout << p << ' ';
	}
	std::cout << '\n' << bestLength << std::endl;
	getchar();
	getchar();
	return 0;
}