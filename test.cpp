#include<iostream>
#include<vector>
#include<functional>
using namespace std;

int main()
{
    //将c1的元素复制到以c2.end()为终点的目标区间
	vector<int>c1 = { 1,2,3,4,5 };
	vector<int>c2 = {11, 22, 33};
	copy_backward(c1.begin(), c1.end(), c2.end());
    for(auto value : c2)
    {
        std::cout << value << std::endl;
    }
}
