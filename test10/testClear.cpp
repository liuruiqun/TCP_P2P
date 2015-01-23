#include <iostream>
#include <list>

typedef struct {
	int i;
} test_t;

using std::cout;
using std::endl;
using std::list;

int main() {
	list<test_t> myList;

	test_t a, b;
	a.i = 0;
	b.i = 1;
	myList.push_back(a);
	myList.push_back(b);

	for(auto it = myList.begin(); it != myList.end(); it++) {
		cout << it->i << endl;
	}

	myList.clear();

	test_t c, d;
	c.i = 2;
	d.i = 3;
	myList.push_back(c);
	myList.push_back(d);

	for(auto it = myList.begin(); it != myList.end(); it ++) {
		cout << it->i << endl;
	}


	list<test_t> *list_ptr;
	list_ptr = &myList;

	list<test_t> newList(*list_ptr);

	for(auto it = newList.begin(); it != newList.end(); it++) {
		cout << it->i << endl;
	}
	auto it = newList.begin();
	it++;
	cout << it->i << endl;
	return 0;
}

