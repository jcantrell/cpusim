#include <iostream>
#include "../../model/cpu/morsel.h"
class TestMorsel : public Morsel
{
	public:
	bool testCount()
	{
		dynamic_bitset<> tbs(64,0ul);
		return tbs.count() == count();
	}
};

int main()
{
	TestMorsel tm;
	if (tm.testCount())
	{
		std::cout << "Test passed" << endl;
	} else {
		std::cout << "Test failed" << endl;
	}
	return 0;
}
