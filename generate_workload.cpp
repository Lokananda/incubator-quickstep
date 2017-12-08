#include<iostream> 
#include<string>
using namespace std;

int main() {
	int i = 0;
	for (i = 0; i < 10000000; i++) {
		cout << "INSERT INTO lineorder14 VALUES (20, 6, " << i << ", 'high');" << endl;
	}
	return 0;
}
