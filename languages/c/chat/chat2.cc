#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <stdlib.h>
using namespace std;

int randint(int n) {return rand()%n;}

bool old_response(string r, vector<string>v){
  for(int i = 0; i < v.size(); i++) if(r == v[i]) return true;
  return false;
}

void output(vector<string>v) {cout<< "CHATBOT:\t" << v[randint(v.size())] << endl;}

void delay(int sum_response, int sum_time) {
  if(sum_response != 0) {
    int t = time(0);
    while(true)if(time(0) == t+sum_time/sum_response)break;
  }
}

int main() {

  string name = "";
  cout<<"Please enter your name: ";
  getline(cin, name);

  vector<string> responses;

  ifstream ifs("responses.txt");
  if(ifs) {
    do {
      string s = "";
      getline(ifs, s);
      if(ifs.eof()) break;
      responses.push_back(s);
    } while(!ifs.eof());
  }

  string response = "";
  getline(cin, response);
  int time1 = time(0);
  int sum_time = 0;
  int sum_response = 0;

  do {

    if(!old_response(response, responses)) responses.push_back(response);

    delay(sum_response, sum_time);
    output(responses);

    cout << name << ":\t";
    getline(cin, response);
    sum_time += time(0)-time1;
    sum_response++;
    time1 = time(0);

  } while(response != "goodbye");

  cout<<"goodbye, " << name;

  ofstream ofs("responses.txt", ios_base::app);
  for(int i = 0; i < responses.size(); i++) ofs << responses[i] << endl;

  return 0;
}
