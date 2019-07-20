#include "mylist.h"
#include <iostream>
int main()
{
	MyList<int> aa;
	aa.push_back(1);
	aa.push_back(1);
	aa.clear();
	int n = aa.size();
	cout << n << endl;
}
