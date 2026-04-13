#include "sscdef.h"

int tok;

int main(int num, char *np[])
 {
  if(num < 3) error("Compiled file is not specified");
  iscan(np[1]);
  igen(np[2]);
  init_decls();
  tok = scan();
  ext_decls();
  escan();
  egen();
  return 0;
 }

