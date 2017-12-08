#include<iostream> 
#include<string>
using namespace std;

int main() {
	int i = 0;
	
	cout << "INSERT INTO lineorder14 VALUES ";
	for (i = 0; i < 10; i++) {
		cout << "(20, 6, " << i << ", 'high'),";
		
	}
	return 0;
}
