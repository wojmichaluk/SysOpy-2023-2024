#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct pak{
 int i;
 char lit;
};

int main (int lpar, char *tab[]){
  int w1;
  struct pak ob1;
// 1) utworzyc potok nazwany 'potok1'
//

  while (1){
// 2) wyswietlic obiekt otrzymany z potoku
//
//
    printf("otrzymano: %d %c\n",ob1.i,ob1.lit); fflush(stdout);
  }

  return 0;
}
