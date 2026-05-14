#include "sscdef.h"

struct TYPE *intp, *charp;

struct SCOPE {
  struct ID *id_root;
  struct SCOPE *snextp;
} *scope_root;

struct SCOPE *get_snode(struct SCOPE *sp) {
  struct SCOPE *p;

  p = (struct SCOPE *)mmalloc(sizeof(struct SCOPE));
  p->id_root = NULL;
  p->snextp = sp;
  return (p);
}

struct TYPE *get_tnode(int tt, int ts, int as, struct TYPE *tp) {
  struct TYPE *p;

  p = (struct TYPE *)mmalloc(sizeof(struct TYPE));
  p->ttype = tt;
  p->tsize = ts;
  p->asize = as;
  p->ttp = tp;
  return (p);
}

struct ID *get_inode(char *id, struct TYPE *tp, int at, int ad) {
  struct ID *p;

  p = (struct ID *)mmalloc(sizeof(struct ID));
  p->sid = id;
  p->itp = tp;
  p->attr = at;
  p->addr = ad;
  p->lidp = p->ridp = NULL;
  return (p);
}

void push_scope() { scope_root = get_snode(scope_root); }

void pop_scope() { scope_root = scope_root->snextp; }

struct ID **sub_search_id(char *id, struct ID **root) {
  int i;

  if (*root == NULL || (i = strcmp(id, (*root)->sid)) == 0)
    return (root);
  else if (i < 0)
    return (sub_search_id(id, &((*root)->lidp)));
  return (sub_search_id(id, &((*root)->ridp)));
}

struct ID *search_id(char *id) {
  struct SCOPE *sp;
  struct ID *idp;

  for (sp = scope_root; sp != NULL; sp = sp->snextp) {
    idp = *sub_search_id(id, &(sp->id_root));
    if (idp != NULL)
      return (idp);
  }
  return (NULL);
}

void sub_insert_id(struct ID **root, char *id, struct TYPE *tp, int at,
                   int ad) {
  struct ID **idpp;

  idpp = sub_search_id(id, root);
  if (*idpp == NULL)
    *idpp = get_inode(id, tp, at, ad);
  else if ((*idpp)->attr == EXTERNAL && at == GLOBAL) {
    (*idpp)->attr = GLOBAL;
    if ((*idpp)->itp->ttype != tp->ttype || (*idpp)->itp->tsize != tp->tsize)
      error(" Double typed Id is found");
    (*idpp)->itp = tp;
  } else
    error("Double defined Id.");
}

void insert_id(char *id, struct TYPE *tp, int at, int ad) {
  sub_insert_id(&(scope_root->id_root), id, tp, at, ad);
}

void size_check(struct TYPE *tp) {
  switch (tp->ttype) {
  case TARRAY:
    size_check(tp->ttp);
    if (tp->ttp->tsize == 0)
      error("Data size is Undefined");
    tp->tsize = tp->asize * tp->ttp->tsize;
    break;
  case TFUNCT:
    size_check(tp->ttp);
    if (tp->ttp->ttype == TFUNCT || tp->ttp->ttype == TARRAY ||
        tp->ttp->ttype == TSTRUCT)
      error("Illegal return type");
    break;
  case TPOINTER:
    size_check(tp->ttp);
    break;
  }
}

struct TYPE *tpspec() {
  char *tag_name, *id;
  struct ID *idp;
  struct TYPE *tsp, *tps, *tp;

  switch (tok) {
  case SCHAR:
    tok = scan();
    return (charp);
  case SINT:
    tok = scan();
    return (intp);
  case SSTRUCT:
    tok = scan();
    if (tok == SIDENT) {
      tag_name = scan_str;
      tok = scan();
      if (tok != SLBRACE) {
        idp = search_id(tag_name);
        if (idp == NULL || idp->attr != TAG) {
          tsp = get_tnode(TSTRUCT, 0, 0, NULL);
          insert_id(tag_name, tsp, TAG, 0);
        }
        return (idp->itp);
      }
      tsp = get_tnode(TSTRUCT, 0, 0, NULL);
      insert_id(tag_name, tsp, TAG, 0);
    } else {
      tag_name = NULL;
      if (tok != SLBRACE)
        error("Struct body Expected");
      tsp = get_tnode(TSTRUCT, 0, 0, NULL);
    }
    tok = scan();
    while ((tps = tpspec()) != NULL) {
      while (tok != SSEMI) {
        tp = subdecls(tps, &id);
        size_check(tp);
        if (id == NULL)
          error(" Identifier Expected");
        if (tp->ttype == TFUNCT)
          error("Function is not member");
        if (tp->tsize == 0)
          error("Member size is Undefined");
        sub_insert_id((struct ID **)&(tsp->ttp), id, tp, MEMBER, tsp->tsize);
        tsp->tsize += (((tp->tsize + 3) >> 2) << 2);
        if (tok == SCOMMA) {
          tok = scan();
          if (tok == SSEMI)
            error("Illegal comma");
        }
      }
      tok = scan();
    }
    if (tok != SRBRACE)
      error(" '}' Expected");
    tok = scan();
    return (tsp);
  }
  return (NULL);
}

struct PARA {
  char *pid;
  struct TYPE *ptp;
} para_tab[PARA_TAB_SIZE];
int para_off;

struct TYPE *subdecls(struct TYPE *tp, char **id) {
  struct TYPE *ctp, *p;

  while (tok == SSTAR) {
    tok = scan();
    tp = get_tnode(TPOINTER, 4, 0, tp);
  }

  if (tok == SLPAREN) {
    tok = scan();
    ctp = subdecls(NULL, id);
    if (tok != SRPAREN)
      error(" ')' Expected");
    tok = scan();
  } else if (tok == SIDENT) {
    *id = scan_str;
    ctp = NULL;
    tok = scan();
  } else
    ctp = NULL;

  if (ctp != NULL)
    for (p = ctp; p->ttp != NULL; p = p->ttp)
      ;

  if (tok == SLPAREN) {
    tok = scan();
    if (ctp == NULL) {
      para_off = 0;
      while (tok == SIDENT) {
        para_tab[para_off].pid = scan_str;
        para_tab[para_off++].ptp = NULL;
        tok = scan();
        if (tok == SCOMMA) {
          tok = scan();
          if (tok != SIDENT)
            error("Illegal comma");
        }
      }
    }
    if (tok != SRPAREN)
      error(" ')' Expected");
    tok = scan();
    if (ctp == NULL)
      p = ctp = get_tnode(TFUNCT, 0, 0, NULL);
    else {
      p->ttp = get_tnode(TFUNCT, 0, 0, NULL);
      p = p->ttp;
    }
  } else if (tok == SLSQP) {
    tok = scan();
    if (tok == SCONST) {
      if (ctp == NULL)
        p = ctp = get_tnode(TARRAY, 0, scan_const, NULL);
      else {
        p->ttp = get_tnode(TARRAY, 0, scan_const, NULL);
        p = p->ttp;
      }
      tok = scan();
    } else if (ctp == NULL)
      p = ctp = get_tnode(TARRAY, 0, 0, NULL);
    else
      error("Array index Expected");
    if (tok != SRSQP)
      error(" ']' Expected");
    tok = scan();
    while (tok == SLSQP) {
      tok = scan();
      if (tok == SCONST) {
        p->ttp = get_tnode(TARRAY, 0, scan_const, NULL);
        p = p->ttp;
        tok = scan();
      } else
        error("Array index Expected");
      if (tok != SRSQP)
        error(" ']' Expected");
      tok = scan();
    }
  }
  if (ctp == NULL)
    ctp = tp;
  else
    p->ttp = tp;
  return (ctp);
}

void para_decls() {
  char *id;
  struct TYPE *tps, *tp;
  int i, off;

  while ((tps = tpspec()) != NULL) {
    while (tok != SSEMI) {
      tp = subdecls(tps, &id);
      size_check(tp);
      if (id == NULL)
        error(" Identifier Expected");
      switch (tp->ttype) {
      case TCHAR:
        tp = intp;
        break;
      case TARRAY:
        tp->ttype = TPOINTER;
        tp->tsize = 4;
        tp->asize = 0;
        break;
      case TFUNCT:
      case TSTRUCT:
        error("Function and Struct is not Parameter");
      }
      for (i = 0; i < para_off; i++) {
        if (strcmp(id, para_tab[i].pid) == 0) {
          if (para_tab[i].ptp != NULL)
            error("Double Defined Parameter Type");
          para_tab[i].ptp = tp;
          break;
        }
      }
      if (i == para_off)
        error("It is not Decl. of parameter");
      if (tok == SCOMMA) {
        tok = scan();
        if (tok == SSEMI)
          error("Illegal comma");
      }
    }
    tok = scan();
  }

  for (off = 8, i = 0; i < para_off; i++) {
    if (para_tab[i].ptp == NULL)
      para_tab[i].ptp = intp;
    insert_id(para_tab[i].pid, para_tab[i].ptp, LOCAL, off);
    off += para_tab[i].ptp->tsize;
  }
}

void init_decls() {
  scope_root = NULL;
  mark();
  push_scope();
  intp = get_tnode(TINT, 4, 0, NULL);
  charp = get_tnode(TCHAR, 1, 0, NULL);
}

int local_size, max_size;

void local_decls() {
  char *id;
  struct TYPE *tp, *tps;

  mark();
  push_scope();
  while ((tps = tpspec()) != NULL) {
    while (tok != SSEMI) {
      tp = subdecls(tps, &id);
      size_check(tp);
      if (tp->ttype == TFUNCT)
        error("Function is local");
      if (tp->tsize == 0)
        error("Local var. size is Undefined");
      if (id == NULL)
        error(" Identifier Expected");
      local_size += (((tp->tsize + 3) >> 2) << 2);
      insert_id(id, tp, LOCAL, -local_size);
      if (tok == SCOMMA) {
        tok = scan();
        if (tok == SSEMI)
          error("Illegal comma");
      }
    }
    tok = scan();
  }
  if (local_size > max_size)
    max_size = local_size;
}

void end_block() {
  pop_scope();
  release();
}

void ext_decls() {
  int at, startlab;
  char *id;
  struct TYPE *tp, *tps;

  gen_code0(" .DATA");
  while (tok != SEOF) {
    if (tok == SEXTERN) {
      at = EXTERNAL;
      tok = scan();
    } else
      at = GLOBAL;
    if ((tps = tpspec()) == NULL)
      tps = intp;
    while (tok != SEOF && tok != SSEMI) {
      tp = subdecls(tps, &id);
      size_check(tp);
      if ((tp->ttype == TARRAY && at == GLOBAL || tp->ttype == TSTRUCT) &&
          tp->tsize == 0)
        error("Size of Array or Struct is Undefined");
      if (id == NULL)
        error(" Identifier Expected");
      insert_id(id, tp, at, 0);
      if (tok == SCOMMA) {
        tok = scan();
        if (tok == SSEMI)
          error("Illegal comma");
      }
      if (at == GLOBAL) {
        if (tp->ttype != TFUNCT) {
          gen_code0p("$%s:", id);
          gen_code0i(" DS   %d", (tp->tsize + 3) >> 2);
        } else {
          mark();
          push_scope();
          para_decls();
          if (tok == SLBRACE) {
            local_size = max_size = 0;
            init_label();
            gen_code0(" .TEXT");
            startlab = get_inlabel();
            gen_code0i("L%d:", startlab);
            st(0, 0);
            check_label();
            gen_code0(" UNLK   FP");
            gen_code0(" RTS");
            gen_code0p("$%s:", id);
            gen_code0i(" LINK  FP,#%d", -max_size);
            gen_code0i(" BRA   L%d", startlab);
            gen_code0(" .DATA");
          } else
            error("Function body Expected");
          pop_scope();
          release();
          goto L;
        }
      }
    }
    if (tok == SSEMI)
      tok = scan();
    else
      error(" ';' Expected");
  L:;
  }
}
