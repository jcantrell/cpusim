#include <string>
#include <fstream>
#include <iostream>

//using namespace std;
int main()
{
  std::string filename = "hello.mmo";
  std::ifstream in;
  in.open(filename, std::ifstream::binary);
  if (!in)
    return 1;
  uint32_t num;
  const int BUFFSIZE = 16;
  //uint32_t buf[4];
  char buf[BUFFSIZE];
  
  while ( !in.eof() ) {
    in.read((char*)&buf, sizeof(buf));

    if (in.gcount() < sizeof(buf))
      for (int i=in.gcount(); i < sizeof(buf); i++)
        buf[i] = 0;
    for (int i=0; i < BUFFSIZE; i+=2) {
      printf("%02x%02x ", buf[i] & 0xFF, buf[i+1] & 0xFF );
    }
      
    printf("\n");
  }
  in.close();
}
