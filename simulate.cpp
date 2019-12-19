#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <queue>
#include <tuple>
#define rep(i,n) for (int i=0; i<n;++i)
using namespace std;

int GuestCountPerHour; // 1時間あたり何人くるか
double easyOrder; // 簡単な注文の確率

int currentTime; // 現在時刻
typedef pair<int, bool> P; //firstは客番号、secondはオーダーが難しいかどうか
typedef pair<int, int> P2; //firstはサービス終了予定時刻、secondは客番号
queue<P> comingQue; // 来る人たち。
queue<P> waitQue; // 列に並んだとき(自分含めて)何人並ぶことになるか（サービスをすぐに受ける場合は考えない）。
priority_queue<P2, vector<P2>, greater<P2> > serviceQue; // サービスを受けている人立ち。サービスが終わった人から抜けるのでpriority_queueで管理。
int result[1000][5]; // 人数(=totalCustomers), 列に並び始めた時刻/サービス開始時刻/サービス終了時刻/並んだときに待っていた人数

double getNextArrivingTime() {
  random_device seed_gen;
  double lambda = (double) GuestCountPerHour / 60.0;
  exponential_distribution<> dist(lambda);
  default_random_engine engine(seed_gen());
  double ret = dist(engine);
  return ret * 60;
}

double getServiceTime(bool isDifficultOrder) {
  random_device seed_gen;
  double lambda = isDifficultOrder? 0.8: 1.4;
  exponential_distribution<> dist(lambda);
  default_random_engine engine(seed_gen());
  double ret = dist(engine);
  return ret * 60;
}

int initialize(int MAX_TIME) {
  int cT = 0;
  int idx = 0;
  random_device rd; 
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, 99);
  while (MAX_TIME > cT) {
    int time = (int)getNextArrivingTime();
    cT += time;
    if (cT >=  MAX_TIME) break; // MAX_TIMEが来た段階で打ち切り
    result[idx][0] = cT;
    bool isDifficult = dis(gen) >= easyOrder * 100; // 100回中easyOrder回は注文の時間が短い
    result[idx][4] = isDifficult;
    comingQue.push(make_pair(idx, isDifficult));
    idx++;
  }
  return idx;
}

void enteringShop() {
  if (comingQue.empty()) return;
  P p = comingQue.front();
  int idx = p.first;
  while (result[idx][0] < currentTime) {
    comingQue.pop();
    bool isDifficultOrder = p.second;
    result[idx][3] = waitQue.size();
    waitQue.push(P(idx, isDifficultOrder));
    if (comingQue.empty()) return;
    p = comingQue.front();
    idx = p.first;
  }
}

void removeCustomerFromService() {
  P2 p = serviceQue.top();
  int serviceEndTime = p.first;
  int idx = p.second;
  if (serviceEndTime <= currentTime) {
    result[idx][2] = serviceEndTime;
    serviceQue.pop();
  }
}

void endService() {
  if (serviceQue.empty()) return;
  removeCustomerFromService();
  if (serviceQue.empty()) return;
  removeCustomerFromService();
}

void addCustomerToService() {
  P p = waitQue.front();
  int idx = p.first;
  int serviceTime = (int)getServiceTime(p.second);
  serviceTime = serviceTime == 0 ? 1: serviceTime;
  serviceQue.push(P2(currentTime+serviceTime, idx));
  result[idx][1] = currentTime;
  waitQue.pop();
}

void startService() {
  if (waitQue.empty()) return;
  if (serviceQue.size() == 2) return;
  if (serviceQue.empty()) addCustomerToService();
  if (serviceQue.size() == 1 && waitQue.size() >= 3) addCustomerToService();
}

int main() {
  int MAX_TIME = 60 * 37; // 計測時間(単位: 秒)
  easyOrder = 0.3; 
  GuestCountPerHour = 100;
  int totalCustomers = initialize(MAX_TIME);
  cout << totalCustomers << "customers" << "\n";
  cout << "-------------------------" << "\n";
  currentTime = 0;
  while (currentTime <= MAX_TIME || !waitQue.empty() || !serviceQue.empty()) {
    endService(); // サービスが終わる関数
    startService(); // 待ち行列の中の人がサービスを受ける
    enteringShop(); // 客が来る関数
    startService(); // 新しく来た人がサービスを受ける
    currentTime++;
  }
  rep(i, totalCustomers) {
    cout << result[i][0] << ", ";
    cout << result[i][1] << ", ";
    cout << result[i][2] << ", ";
    cout << result[i][3] << ", ";
    cout << result[i][4] << "\n";
  }

  ofstream file("result.csv");
  rep(i, totalCustomers) {
    file << result[i][0] << ", ";
    file << result[i][1] - result[i][0] << ", ";
    file << result[i][2] - result[i][1] << ", ";
    file << result[i][3] << ", ";
    file << result[i][4] << "\n";
  }
  
  return 0;
}