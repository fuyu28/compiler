#include "sscdef.h"

struct ENODE *get_enode(int o, struct TYPE *tp, int lvf, struct ENODE *lp,
                        struct ENODE *rp) {
  struct ENODE *p;

  p = (struct ENODE *)mmalloc(sizeof(struct ENODE));
  p->opr = o;
  p->etp = tp;
  p->lv = lvf;
  p->elp = lp;
  p->elpi = 0;
  p->erp = rp;
  return (p);
}

struct ENODE *get_enodei(int o, struct TYPE *tp, int lvf, int i,
                         struct ENODE *rp) {
  struct ENODE *p;

  p = (struct ENODE *)mmalloc(sizeof(struct ENODE));
  p->opr = o;
  p->etp = tp;
  p->lv = lvf;
  p->elp = NULL;
  p->elpi = i;
  p->erp = rp;
  return (p);
}

struct ENODE *mkpexp() {
  struct ENODE *p, *q, *r;
  struct ID *idp;

  switch (tok) {
  case SIDENT:
    idp = search_id(scan_str);
    if (idp == NULL)
      if (nscan() == SLPAREN)
        idp = get_inode(scan_str, get_tnode(TFUNCT, 0, 0, intp), GLOBAL, 0);
      else
        error("Undefined Identifier");
    if (idp->attr == TAG)
      error("TAG in expression");
    p = get_enode(OIDENT, idp->itp, LV, (struct ENODE *)idp, NULL);
    tok = scan();
    break;
  case SSTR:
    p = get_enode(OSTR, get_tnode(TPOINTER, 4, 0, charp), RV,
                  (struct ENODE *)scan_str, NULL);
    tok = scan();
    break;
  case SCONST:
    p = get_enodei(OCONST, intp, RV, scan_const, NULL);
    tok = scan();
    break;
  case SLPAREN:
    tok = scan();
    p = mkexp();
    if (tok != SRPAREN)
      error(" ')' Expected");
    tok = scan();
    break;
  default:
    error("Illegal Symbol in expression");
  }

  for (;;) {
    switch (tok) {
    case SLPAREN:
      if (p->etp->ttype != TFUNCT)
        error("Illegal type at Functin call");
      tok = scan();
      q = NULL;
      while (tok != SRPAREN) {
        r = mkasexp();
        if (q == NULL)
          q = r;
        else
          q = get_enode(OSCOMMA, NULL, RV, q, r);
        if (tok == SCOMMA) {
          tok = scan();
          if (tok == SRPAREN)
            error("Illegal Comma");
        }
      }
      tok = scan();
      p = get_enode(OFCALL, p->etp->ttp, RV, p, q);
      break;
    case SLSQP:
      if (p->etp->ttype != TARRAY && p->etp->ttype != TPOINTER)
        error("Illegal type at array access");
      tok = scan();
      q = mkexp();
      if (q->etp->ttype != TCHAR && q->etp->ttype != TINT)
        error("Illegal type at array index");
      if (tok != SRSQP)
        error(" ']' Expected");
      tok = scan();
      p = get_enode(OAACCESS, p->etp->ttp, LV, p, q);
      break;
    case SDOT:
      if (p->etp->ttype != TSTRUCT)
        error("Illegal type at struct access");
      tok = scan();
      if (tok != SIDENT)
        error("Identifier Expected");
      idp = *sub_search_id(scan_str, (struct ID **)&p->etp->ttp);
      if (idp == NULL)
        error("Undefined member");
      p = get_enode(OSACCESS, idp->itp, LV, p, (struct ENODE *)idp);
      tok = scan();
      break;
    case SPOINTTO:
      if (p->etp->ttype != TPOINTER || p->etp->ttp->ttype != TSTRUCT)
        error("Illegal type at struct access");
      tok = scan();
      if (tok != SIDENT)
        error("Identifier Expected");
      idp = *sub_search_id(scan_str, (struct ID **)&p->etp->ttp->ttp);
      if (idp == NULL)
        error("Undefined member");
      p = get_enode(OPOINTTO, idp->itp, LV, p, (struct ENODE *)idp);
      tok = scan();
      break;
    default:
      return (p);
    }
  }
  return (p); /* Never Executed Return */
}

int istype() {
  int t;

  t = nscan();
  return (tok == SLPAREN && (t == SCHAR || t == SINT || t == SSTRUCT));
}

struct ENODE *mkuexp() {
  struct ENODE *p;
  char *id;
  struct TYPE *tp;
  int t, to;

  switch (tok) {
  case SSTAR:
    tok = scan();
    p = mkuexp();
    if (p->etp->ttype != TPOINTER)
      error("Illegal type at '*'");
    p = get_enode(OINDIRECT, p->etp->ttp, LV, p, NULL);
    break;
  case SBAND:
    tok = scan();
    p = mkuexp();
    p = get_enode(OADDRESS, get_tnode(TPOINTER, 4, 0, p->etp), RV, p, NULL);
    break;
  case SMINUS:
  case SLNOT:
  case SBNOT:
  case SINC:
  case SDEC:
    if (tok == SMINUS)
      to = OUMINUS;
    else if (tok == SLNOT)
      to = OLNOT;
    else if (tok == SBNOT)
      to = OBNOT;
    else if (tok == SINC)
      to = OLINC;
    else
      to = OLDEC;
    tok = scan();
    p = mkuexp();
    t = p->etp->ttype;
    if (t == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at Unary left opr. for Integral Type");
    p = get_enode(to, intp, RV, p, NULL);
    break;
  case SSIZEOF:
    tok = scan();
    if (istype()) {
      tok = scan();
      tp = tpspec();
      tp = subdecls(tp, &id);
      size_check(tp);
      if (tok != SRPAREN)
        error(" ')' Expected");
      tok = scan();
    } else {
      p = mkuexp();
      tp = p->etp;
    }
    p = get_enodei(OCONST, intp, RV, tp->tsize, NULL);
    break;
  default:
    if (istype()) {
      tok = scan();
      tp = tpspec();
      tp = subdecls(tp, &id);
      size_check(tp);
      if (tok != SRPAREN)
        error(" ')' Expected");
      tok = scan();
      p = mkuexp();
      p = get_enode(OTCONV, tp, RV, p, NULL);
    } else {
      p = mkpexp();
      if (tok == SINC || tok == SDEC) {
        if (tok == SINC)
          to = ORINC;
        else
          to = ORDEC;
        tok = scan();
        t = p->etp->ttype;
        if (t == TFUNCT || t == TARRAY || t == TSTRUCT)
          error("Illegal type at '++' or '--'");
        p = get_enode(to, p->etp, RV, p, NULL);
      }
    }
    break;
  }
  return (p);
}

struct ENODE *mkmulexp() {
  struct ENODE *p, *q;
  int op;

  p = mkuexp();
  if (tok != SSTAR && tok != SDIV && tok != SMOD)
    return (p);
  if (p->etp->ttype != TCHAR && p->etp->ttype != TINT)
    error("Illegal type at multiply operator");
  for (;;) {
    if (tok == SSTAR && nscan() != SASSIGN)
      op = OMUL;
    else if (tok == SDIV && nscan() != SASSIGN)
      op = ODIV;
    else if (tok == SMOD && nscan() != SASSIGN)
      op = OMOD;
    else
      break;
    tok = scan();
    q = mkuexp();
    if (q->etp->ttype != TCHAR && q->etp->ttype != TINT)
      error("Illegal type at multiply operator");
    p = get_enode(op, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkaddexp() {
  struct ENODE *p, *q;
  int op, tq;

  p = mkmulexp();
  if (tok != SPLUS && tok != SMINUS)
    return (p);
  if (p->etp->ttype == TFUNCT || p->etp->ttype == TSTRUCT)
    error("Illegal type at aditive operator");
  for (;;) {
    if (tok == SPLUS && nscan() != SASSIGN)
      op = OADD;
    else if (tok == SMINUS && nscan() != SASSIGN)
      op = OSUB;
    else
      break;
    tok = scan();
    q = mkmulexp();
    tq = q->etp->ttype;
    if (tq == TFUNCT || tq == TSTRUCT)
      error("Illegal type at aditive operator");
    if (op == OADD) {
      if (tq == TPOINTER || tq == TARRAY)
        error("Illegal type at '+' operator");
    } else if (tq == TPOINTER || tq == TARRAY)
      if (p->etp->ttype == TPOINTER || p->etp->ttype == TARRAY) {
        p = get_enode(op, intp, RV, p, q);
        continue;
      } else
        error("Illegal type at '-' operator");
    if (p->etp->ttype == TARRAY)
      p = get_enode(op, get_tnode(TPOINTER, 4, 0, p->etp->ttp), RV, p, q);
    else
      p = get_enode(op, p->etp, RV, p, q);
  }
  return (p);
}

struct ENODE *mksexp() {
  struct ENODE *p, *q;
  int op;

  p = mkaddexp();
  if (tok != SLSHIFT && tok != SRSHIFT)
    return (p);
  if (p->etp->ttype != TCHAR && p->etp->ttype != TINT)
    error("Illegal type at shift operator");
  for (;;) {
    if (tok == SLSHIFT && nscan() != SASSIGN)
      op = OLSHIFT;
    else if (tok == SRSHIFT && nscan() != SASSIGN)
      op = ORSHIFT;
    else
      break;
    tok = scan();
    q = mkaddexp();
    if (q->etp->ttype != TCHAR && q->etp->ttype != TINT)
      error("Illegal type at shift operator");
    p = get_enode(op, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkueexp() {
  struct ENODE *p, *q;
  int op;

  p = mksexp();
  if (tok != SLESS && tok != SGREAT && tok != SLESSEQ && tok != SGREATEQ)
    return (p);
  if (p->etp->ttype == TFUNCT || p->etp->ttype == TSTRUCT)
    error("Illegal type at unequality operator");
  for (;;) {
    if (tok == SLESS)
      op = OLESS;
    else if (tok == SGREAT)
      op = OGREAT;
    else if (tok == SLESSEQ)
      op = OLESSEQ;
    else if (tok == SGREATEQ)
      op = OGREATEQ;
    else
      break;
    tok = scan();
    q = mksexp();
    if (q->etp->ttype == TFUNCT || q->etp->ttype == TSTRUCT)
      error("Illegal type at unequality operator");
    p = get_enode(op, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkeexp() {
  struct ENODE *p, *q;
  int op;

  p = mkueexp();
  if (tok != SEQUAL && tok != SNOTEQ)
    return (p);
  if (p->etp->ttype == TFUNCT || p->etp->ttype == TSTRUCT)
    error("Illegal type at equality operator");
  for (;;) {
    if (tok == SEQUAL)
      op = OEQUAL;
    else if (tok == SNOTEQ)
      op = ONOTEQ;
    else
      break;
    tok = scan();
    q = mkueexp();
    if (q->etp->ttype == TFUNCT || q->etp->ttype == TSTRUCT)
      error("Illegal type at equality operator");
    p = get_enode(op, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkbaexp() {
  struct ENODE *p, *q;
  int t;

  p = mkeexp();
  if (tok != SBAND)
    return (p);
  if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
    error("Illegal type at '&'");
  while (tok == SBAND && nscan() != SASSIGN) {
    tok = scan();
    q = mkeexp();
    if ((t = q->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at '&'");
    p = get_enode(OBAND, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkbeexp() {
  struct ENODE *p, *q;
  int t;

  p = mkbaexp();
  if (tok != SBEOR)
    return (p);
  if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
    error("Illegal type at '~'");
  while (tok == SBEOR && nscan() != SASSIGN) {
    tok = scan();
    q = mkbaexp();
    if ((t = q->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at '~'");
    p = get_enode(OBEOR, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkboexp() {
  struct ENODE *p, *q;
  int t;

  p = mkbeexp();
  if (tok != SBOR)
    return (p);
  if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
    error("Illegal type at '|'");
  while (tok == SBOR && nscan() != SASSIGN) {
    tok = scan();
    q = mkbeexp();
    if ((t = q->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at '|'");
    p = get_enode(OBOR, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mklaexp() {
  struct ENODE *p, *q;
  int t;

  p = mkboexp();
  if (tok != SLAND)
    return (p);
  if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
    error("Illegal type at '&&'");
  while (tok == SLAND) {
    tok = scan();
    q = mkboexp();
    if ((t = q->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at '&&'");
    p = get_enode(OLAND, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkloexp() {
  struct ENODE *p, *q;
  int t;

  p = mklaexp();
  if (tok != SLOR)
    return (p);
  if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
    error("Illegal type at '||'");
  while (tok == SLOR) {
    tok = scan();
    q = mklaexp();
    if ((t = q->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at '||'");
    p = get_enode(OLOR, intp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkcexp() {
  struct ENODE *p, *q, *r;
  int t;

  p = mkloexp();
  if (tok != SQUEST)
    return (p);
  if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
    error("Illegal type at '?'");
  if (tok == SQUEST) {
    tok = scan();
    q = mkcexp();
    if (tok != SCOLON)
      error(" ':' Expected");
    tok = scan();
    r = mkcexp();
    if (q->etp->ttype != r->etp->ttype || q->etp->tsize != r->etp->tsize)
      error("Type Mismatch at ':'");
    q = get_enode(OCOLON, q->etp, RV, q, r);
    p = get_enode(OQUEST, q->etp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkasexp() {
  struct ENODE *p, *q;
  int op, t;

  op = SASSIGN;
  p = mkcexp();
  switch (tok) {
  case SPLUS:
  case SMINUS:
  case SSTAR:
  case SDIV:
  case SMOD:
  case SLSHIFT:
  case SRSHIFT:
  case SBAND:
  case SBEOR:
  case SBOR:
    if (nscan() != SASSIGN)
      break;
    op = tok;
    tok = scan();
  case SASSIGN:
    tok = scan();
    if ((t = p->etp->ttype) == TFUNCT || t == TARRAY || t == TSTRUCT)
      error("Illegal type at Assign");
    q = mkasexp();
    if ((t = q->etp->ttype) == TFUNCT || t == TSTRUCT ||
        op != SASSIGN && (t == TARRAY || t == TPOINTER))
      error("Illegal type at Assign");
    switch (op) {
    case SPLUS:
      op = OADDAS;
      break;
    case SMINUS:
      op = OSUBAS;
      break;
    case SSTAR:
      op = OMULAS;
      break;
    case SDIV:
      op = ODIVAS;
      break;
    case SMOD:
      op = OMODAS;
      break;
    case SLSHIFT:
      op = OLSHIFTAS;
      break;
    case SRSHIFT:
      op = ORSHIFTAS;
      break;
    case SBAND:
      op = OBANDAS;
      break;
    case SBEOR:
      op = OBEORAS;
      break;
    case SBOR:
      op = OBORAS;
      break;
    case SASSIGN:
      op = OASSIGN;
      break;
    }
    p = get_enode(op, p->etp, RV, p, q);
  }
  return (p);
}

struct ENODE *mkexp() {
  struct ENODE *p, *q;

  p = mkasexp();
  while (tok == SCOMMA) {
    tok = scan();
    q = mkasexp();
    p = get_enode(OCOMMA, q->etp, RV, p, q);
  }
  return (p);
}
