#include "sscdef.h"

int tok;

int main(int num, char *np[])
 {
  printf("This compiler is modified by Yu Yamaguchi.\n");
  if(num < 3) error("Compiled file is not specified");
  iscan(np[1]);
  igen(np[2]);
  init_decls();
  tok = scan();
  gen_code0("This code is modified by Yu Yamaguchi");
  ext_decls();
  escan();
  egen();
  return 0;
 }

