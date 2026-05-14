#include "sscdef.h"

void compound_st(int blab, int clab) {
  int size;

  size = local_size;
  local_decls();
  while (tok != SRBRACE)
    st(blab, clab);
  tok = scan();
  end_block();
  local_size = size;
}

void if_st(int blab, int clab) {
  int tlab, flab;

  if (tok != SLPAREN)
    error(" '(' Expected");
  tok = scan();
  expcode();
  flab = get_inlabel();
  gen_code0(" TST.W R0");
  gen_code0i(" BEQ   L%d", flab);
  if (tok != SRPAREN)
    error(" ')' Expected");
  tok = scan();
  st(blab, clab);
  if (tok == SELSE) {
    tok = scan();
    tlab = get_inlabel();
    gen_code0i(" BRA   L%d", tlab);
    gen_code0i("L%d:", flab);
    st(blab, clab);
    gen_code0i("L%d:", tlab);
  } else
    gen_code0i("L%d:", flab);
}

void while_st() {
  int blab, clab;

  if (tok != SLPAREN)
    error(" '(' Expected");
  tok = scan();
  clab = get_inlabel();
  gen_code0i("L%d:", clab);
  expcode();
  blab = get_inlabel();
  gen_code0(" TST.W R0");
  gen_code0i(" BEQ   L%d", blab);
  if (tok != SRPAREN)
    error(" ')' Expected");
  tok = scan();
  st(blab, clab);
  gen_code0i(" BRA   L%d", clab);
  gen_code0i("L%d:", blab);
}

void do_st() {
  int blab, clab, tlab;

  tlab = get_inlabel();
  gen_code0i("L%d:", tlab);
  blab = get_inlabel();
  clab = get_inlabel();
  st(blab, clab);
  if (tok != SWHILE)
    error(" 'while' Expected");
  tok = scan();
  if (tok != SLPAREN)
    error(" '(' Expected");
  tok = scan();
  gen_code0i("L%d:", clab);
  expcode();
  gen_code0(" TST.W R0");
  gen_code0i(" BNE   L%d", tlab);
  gen_code0i("L%d:", blab);
  if (tok != SRPAREN)
    error(" ')' Expected");
  tok = scan();
  if (tok != SSEMI)
    error(" ';' Expected");
  tok = scan();
}

void repeat_st() {
  int blab, clab, tlab;

  tlab = get_inlabel();
  gen_code0i("L%d:", tlab);
  blab = get_inlabel();
  clab = get_inlabel();
  st(blab, clab);
  if (tok != SUNTIL)
    error(" 'until' Expected");
  tok = scan();
  if (tok != SLPAREN)
    error(" '(' Expected");
  tok = scan();
  gen_code0i("L%d:", clab);
  expcode();
  gen_code0(" TST.W R0");
  gen_code0i(" BEQ   L%d", tlab);
  gen_code0i("L%d:", blab);
  if (tok != SRPAREN)
    error(" ')' Expected");
  tok = scan();
  if (tok != SSEMI)
    error(" ';' Expected");
  tok = scan();
}

void for_st() {
  int tlab, slab, blab, clab;

  tlab = get_inlabel();
  slab = get_inlabel();
  blab = get_inlabel();
  clab = get_inlabel();
  if (tok != SLPAREN)
    error(" '(' Expected");
  tok = scan();
  if (tok != SSEMI)
    expcode();
  if (tok != SSEMI)
    error(" ';' Expexted");
  tok = scan();
  gen_code0i("L%d:", tlab);
  if (tok != SSEMI) {
    expcode();
    gen_code0(" TST.W R0");
    gen_code0i(" BEQ   L%d", blab);
    if (tok != SSEMI)
      error(" ';' Expected");
  }
  tok = scan();
  gen_code0i(" BRA   L%d", slab);
  gen_code0i("L%d:", clab);
  if (tok != SRPAREN)
    expcode();
  if (tok != SRPAREN)
    error(" ')' Expected");
  tok = scan();
  gen_code0i(" BRA   L%d", tlab);
  gen_code0i("L%d:", slab);
  st(blab, clab);
  gen_code0i(" BRA   L%d", clab);
  gen_code0i("L%d:", blab);
}

void switch_st(int clab) {
  int blab, nlab, jlab, dlab, size;

  if (tok != SLPAREN)
    error(" '(' Expected");
  tok = scan();
  expcode();
  nlab = get_inlabel();
  gen_code0i(" BRA   L%d", nlab);
  if (tok != SRPAREN)
    error(" ')' Expected");
  tok = scan();
  if (tok != SLBRACE)
    error(" '{' Expected");
  tok = scan();
  size = local_size;
  local_decls();
  blab = get_inlabel();
  dlab = 0;
  while (tok != SRBRACE) {
    if (tok == SDEFAULT) {
      tok = scan();
      if (tok != SCOLON)
        error(" ':' Expected");
      tok = scan();
      if (dlab != 0)
        error("Find Second Default");
      dlab = get_inlabel();
      gen_code0i("L%d:", dlab);
    } else if (tok == SCASE) {
      jlab = get_inlabel();
      gen_code0i(" BRA   L%d", jlab);
      gen_code0i("L%d:", nlab);
      do {
        tok = scan();
        if (tok != SCONST)
          error("Constant Expected");
        gen_code0i(" COMP.W #%d R0", scan_const);
        gen_code0i(" BEQ    L%d", jlab);
        tok = scan();
        if (tok != SCOLON)
          error(" ':' Expected");
        tok = scan();
      } while (tok == SCASE);
      nlab = get_inlabel();
      gen_code0i(" BRA   L%d", nlab);
      gen_code0i("L%d:", jlab);
    } else
      st(blab, clab);
  }
  tok = scan();
  if (dlab != 0) {
    gen_code0i(" BRA   L%d", blab);
    gen_code0i("L%d:", nlab);
    gen_code0i(" BRA   L%d", dlab);
  } else
    gen_code0i("L%d:", nlab);
  gen_code0i("L%d:", blab);
  end_block();
  local_size = size;
}

void break_cont_st(int lab) {
  if (tok != SSEMI)
    error(" ';' Expected");
  tok = scan();
  if (lab == 0)
    error("Illegal position of break or continue st.");
  gen_code0i(" BRA   L%d", lab);
}

void return_st() {
  if (tok != SSEMI)
    expcode();
  if (tok != SSEMI)
    error(" ';' Expected");
  tok = scan();
  gen_code0(" UNLK  FP");
  gen_code0(" RTS");
}

void goto_st() {
  if (tok != SIDENT)
    error("Label Expected");
  gen_code0i(" BRA   L%d", search_label(scan_str, REF));
  tok = scan();
  if (tok != SSEMI)
    error(" ';' Expected");
  tok = scan();
}

void labeled_st(int blab, int clab) {
  gen_code0i("L%d:", search_label(scan_str, DEF));
  tok = scan();
  tok = scan();
  st(blab, clab);
}

void exp_st() {
  expcode();
  if (tok != SSEMI)
    error(" ';' Expected");
  tok = scan();
}

void st(int blab, int clab) {
  switch (tok) {
  case SLBRACE:
    tok = scan();
    compound_st(blab, clab);
    break;
  case SIF:
    tok = scan();
    if_st(blab, clab);
    break;
  case SWHILE:
    tok = scan();
    while_st();
    break;
  case SDO:
    tok = scan();
    do_st();
    break;
  case SREPEAT:
    tok = scan();
    repeat_st();
    break;
  case SFOR:
    tok = scan();
    for_st();
    break;
  case SSWITCH:
    tok = scan();
    switch_st(clab);
    break;
  case SBREAK:
    tok = scan();
    break_cont_st(blab);
    break;
  case SCONTINUE:
    tok = scan();
    break_cont_st(clab);
    break;
  case SRETURN:
    tok = scan();
    return_st();
    break;
  case SSEMI:
    tok = scan();
    break;
  case SGOTO:
    tok = scan();
    goto_st();
    break;
  case SIDENT:
    if (nscan() == SCOLON) {
      labeled_st(blab, clab);
      break;
    }
  default:
    exp_st();
  }
}
