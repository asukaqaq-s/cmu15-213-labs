#include <iostream>
#include <algorithm>

using namespace std;

int allOddBits(int x) {
  int a = x;
  int b = a >> 1;
  int minus_one = ~(0x0);
  cout << minus_one << ' ' << a << ' ' << b << endl;
  return !((a + b) ^ minus_one);
}
int getbit(int x) {
    string s;
    while(x) {
        s = s + char((x&1) + '0');
        cout << x << endl;
        
        x >>= 1;
        // return 0;
    }
    reverse(s.begin(),s.end());
    cout << s << endl;
    return 0;
}
int main()
{
  int k = (0xAA);
  int c = k;
  k = (k << 8) + c;
  c = k;
  k = (k << 8) + c;
  c = k;
  k = (k << 8) + c;
  getbit(k);
    return 0;
}