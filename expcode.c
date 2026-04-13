#include "sscdef.h"

int reg;

void cload(int t, int s, int d, int w)
 {
  if(t == TCHAR) {
    gen_code0i(" CLR.W R%d",w);
    gen_code0ii(" MOV.B (R%d),R%d",s,w);
    gen_code0ii(" MOV.W R%d,R%d",w,d);
   }
   else gen_code0ii(" MOV.W (R%d),R%d",s,d);
 }


void cstore(int t, int s, int d)
 {
  if(t == TCHAR)
    gen_code0ii(" MOV.B R%d,(R%d)",s,d);
   else gen_code0ii(" MOV.W R%d,(R%d)",s,d);
 }


void cident(struct ENODE *ep, int lvf)
 {
  struct ID * idp;

  idp = (struct ID *)ep->elp;
  if(idp->attr == LOCAL)
    gen_code0ii(" LEA   %d(FP),R%d",idp->addr,reg++);
   else gen_code0pi(" LEA   $%s,R%d",idp->sid,reg++);
  if(lvf != LV && idp->itp->ttype != TARRAY)
    cload(idp->itp->ttype,reg-1,reg-1,reg);
 }


void cstr(struct ENODE *ep)
 {
  int i;

  i = get_inlabel();
  gen_code0(" .DATA");
  gen_code0i("L%d:",i);
  gen_code0p(" DC    \"%s\"",(char *)ep->elp);
  gen_code0(" .TEXT");
  gen_code0ii(" LEA   L%d,R%d",i,reg++);
 }


void cconst(struct ENODE *ep)
 {
  gen_code0ii(" MOV.W #%d,R%d",ep->elpi,reg++);
 }


void cscomma(struct ENODE *ep, int *cp)
 {
  gen_ecode(ep->erp,RV);
  gen_code0(" MOV.W R0,-(SP)");
  (*cp)++;
  reg = 0;
  if(ep->elp->opr == OSCOMMA) cscomma(ep->elp,cp);
   else {
    gen_ecode(ep->elp,RV);
    gen_code0(" MOV.W R0,-(SP)");
    (*cp)++;
    reg = 0;
   }
 }


void cfcall(struct ENODE *ep)
 {
  int oreg, count;

  oreg = reg;
  count = 0;
  if(reg > 1)  gen_code0i(" MOVEM.W R0-R%d,-(SP)",reg-1);
   else if(reg == 1) gen_code0(" MOV.W R0,-(SP)");
  reg = 0;
  if(ep->erp != NULL) {
    if(ep->erp->opr == OSCOMMA) cscomma(ep->erp,&count);
     else {
      gen_ecode(ep->erp,RV);
      gen_code0(" MOV.W R0,-(SP)");
      count++;
      reg = 0;
     }
   }
  gen_ecode(ep->elp,LV);
  gen_code0(" JSR (R0)");
  if(count > 0) gen_code0i(" ADD.W #%d,SP",count << 2);
  if(oreg != 0) gen_code0i(" MOV.W R0,R%d",oreg);
  if(oreg > 1) gen_code0i(" MOVEM.W (SP)+,R0-R%d",oreg-1);
   else if(oreg == 1) gen_code0(" MOV.W (SP)+,R0");
  reg = oreg + 1;
 }


void csaccess(struct ENODE *ep, int lvf, int lr)
 {
  struct ID *idp;

  gen_ecode(ep->elp,lr);
  idp = (struct ID *)ep->erp;
  gen_code0ii(" ADD.W #%d,R%d",idp->addr,reg-1);
  if(lvf == RV && idp->itp->ttype != TARRAY)
    cload(idp->itp->ttype,reg-1,reg-1,reg);
 }


void caaccess(struct ENODE *ep, int lvf)
 {
  gen_ecode(ep->elp,RV);
  gen_ecode(ep->erp,RV);
  gen_code0ii(" MULS.W #%d,R%d",ep->etp->tsize,reg-1);
  gen_code0ii(" ADD.W R%d,R%d",reg-1,reg-2);
  reg--;
  if(lvf == RV && ep->etp->ttype != TARRAY)
    cload(ep->etp->ttype,reg-1,reg-1,reg);
 }


void cindirect(struct ENODE *ep,int lvf)
 {
  gen_ecode(ep->elp,RV);
  if(lvf == RV && ep->etp->ttype != TARRAY)
    cload(ep->etp->ttype,reg-1,reg-1,reg);
 }


void cuop(struct ENODE *ep, char *op)
 {
  gen_ecode(ep->elp,RV);
  gen_code0pi(" %s.W R%d",op,reg-1);
 }


void clnot(struct ENODE *ep)
 {
  gen_ecode(ep->elp,RV);
  gen_code0i(" TST.W R%d",reg-1);
  gen_code0i(" SEQ.W R%d",reg-1);
  gen_code0i(" NEG.W R%d",reg-1);
 }


void clid(struct ENODE *ep, char *op)
 {
  int s;

  gen_ecode(ep->elp,LV);
  cload(ep->etp->ttype,reg-1,reg,reg+1);
  if(ep->etp->ttype == TPOINTER || ep->etp->ttype == TARRAY)
    s = ep->etp->ttp->tsize;
   else s = 1;
  gen_code0pii(" %s.W #%d,R%d",op,s,reg);
  cstore(ep->etp->ttype,reg,reg-1);
  gen_code0ii(" MOV.W R%d,R%d",reg,reg-1);
 }


void crid(struct ENODE *ep, char *op)
 {
  int s;

  gen_ecode(ep->elp,LV);
  cload(ep->etp->ttype,reg-1,reg,reg+1);
  gen_code0ii(" MOV.W R%d,R%d",reg,reg+1);
  if(ep->etp->ttype == TPOINTER || ep->etp->ttype == TARRAY)
    s = ep->etp->ttp->tsize;
   else s = 1;
  gen_code0pii(" %s.W #%d,R%d",op,s,reg+1);
  cstore(ep->etp->ttype,reg+1,reg-1);
  gen_code0ii(" MOV.W R%d,R%d",reg,reg-1);
 }


void cbop(struct ENODE *ep, char *op)
 {
  gen_ecode(ep->elp,RV);
  gen_ecode(ep->erp,RV);
  gen_code0pii(" %s.W R%d,R%d",op,reg-1,reg-2);
  reg--;
 }


void cadd(struct ENODE *ep)
 {
  gen_ecode(ep->elp,RV);
  gen_ecode(ep->erp,RV);
  if(ep->etp->ttype == TPOINTER)
    gen_code0ii(" MULS.W #%d,R%d",ep->etp->ttp->tsize,reg-1);
  gen_code0ii(" ADD.W R%d,R%d",reg-1,reg-2);
  reg--;
 }


void csub(struct ENODE *ep)
 {
  gen_ecode(ep->elp,RV);
  gen_ecode(ep->erp,RV);
  if(ep->etp->ttype == TINT && (ep->elp->etp->ttype == TPOINTER ||
                                ep->elp->etp->ttype == TARRAY)) {
    gen_code0ii(" SUB.W R%d,R%d",reg-1,reg-2);
    gen_code0ii(" DIVS.W #%d,R%d",ep->elp->etp->ttp->tsize,reg-2);
   }
   else {
    if(ep->etp->ttype == TPOINTER)
      gen_code0ii(" MULS.W #%d,R%d",ep->etp->ttp->tsize,reg-1);
    gen_code0ii(" SUB.W R%d,R%d",reg-1,reg-2);
   }
  reg--;
 }


void ccop(struct ENODE *ep, char *op)
 {
  gen_ecode(ep->elp,RV);
  gen_ecode(ep->erp,RV);
  gen_code0ii(" CMP.W R%d,R%d",reg-1,reg-2);
  gen_code0pi(" S%s.W R%d",op,reg-2);
  gen_code0i(" NEG.W R%d",reg-2);
  reg--;
 }


void clop(struct ENODE *ep, char *op)
 {
  int i;

  i = get_inlabel();
  gen_ecode(ep->elp,RV);
  gen_code0i(" TST.W R%d",reg-1);
  gen_code0pi(" B%s.W L%d",op,i);
  reg--;
  gen_ecode(ep->erp,RV);
  gen_code0i(" TST.W R%d",reg-1);
  gen_code0i("L%d:",i);
  gen_code0i(" SNE.W R%d",reg-1);
  gen_code0i(" NEG.W R%d",reg-1);
 }


void cquest(struct ENODE *ep)
 {
  int i, j;

  i = get_inlabel();
  j = get_inlabel();
  gen_ecode(ep->elp,RV);
  gen_code0i(" TST.W R%d",reg-1);
  gen_code0i(" BEQ   L%d",i);
  reg--;
  gen_ecode(ep->erp->elp,RV);
  gen_code0i(" BRA   L%d",j);
  gen_code0i("L%d:",i);
  reg--;
  gen_ecode(ep->erp->erp,RV);
  gen_code0i("L%d:",j);
 }


void casop(struct ENODE *ep, char *op)
 {
  gen_ecode(ep->erp,RV);
  gen_ecode(ep->elp,LV);
  cload(ep->elp->etp->ttype,reg-1,reg,reg+1);
  gen_code0pii(" %s.W R%d,R%d",op,reg-2,reg);
  gen_code0ii(" MOV.W R%d,R%d",reg,reg-2);
  cstore(ep->elp->etp->ttype,reg-2,reg-1);
  reg--;
 }


void caddas(struct ENODE *ep)
 {
  gen_ecode(ep->erp,RV);
  gen_ecode(ep->elp,LV);
  cload(ep->elp->etp->ttype,reg-1,reg,reg+1);
  if(ep->etp->ttype == TPOINTER)
    gen_code0ii(" MULS.W #%d,R%d",ep->etp->ttp->tsize,reg-2);
  gen_code0ii(" ADD.W R%d,R%d",reg-2,reg);
  cstore(ep->elp->etp->ttype,reg,reg-1);
  gen_code0ii(" MOV.W R%d,R%d",reg,reg-2);
  reg--;
 }


void csubas(struct ENODE *ep)
 {
  gen_ecode(ep->erp,RV);
  gen_ecode(ep->elp,LV);
  cload(ep->elp->etp->ttype,reg-1,reg,reg+1);
  if(ep->etp->ttype == TINT && (ep->elp->etp->ttype == TPOINTER ||
                                ep->elp->etp->ttype == TARRAY)) {
    gen_code0ii(" SUB.W R%d,R%d",reg-2,reg);
    gen_code0ii(" DIVS.W #%d,R%d",ep->elp->etp->ttp->tsize,reg);
   }
   else {
    if(ep->etp->ttype == TPOINTER)
      gen_code0ii(" MULS.W #%d,R%d",ep->etp->ttp->tsize,reg-2);
    gen_code0ii(" SUB.W R%d,R%d",reg-2,reg);
   }
  cstore(ep->elp->etp->ttype,reg,reg-1);
  gen_code0ii(" MOV.W R%d,R%d",reg,reg-2);
  reg--;
 }


void cassign(struct ENODE *ep)
 {
  gen_ecode(ep->erp,RV);
  gen_ecode(ep->elp,LV);
  cstore(ep->elp->etp->ttype,reg-2,reg-1);
  reg--;
 }


void ccomma(struct ENODE *ep)
 {
  gen_ecode(ep->elp,RV);
  reg--;
  gen_ecode(ep->erp,RV);
 }


void gen_ecode(struct ENODE *ep, int lvf)
 {
  if(lvf == LV && ep->lv == RV) error("Not Lvalue");
  switch(ep->opr) {
    case OIDENT   : cident(ep,lvf);       break;
    case OSTR     : cstr(ep);             break;
    case OCONST   : cconst(ep);           break;
    case OFCALL   : cfcall(ep);           break;
    case OAACCESS : caaccess(ep,lvf);     break;
    case OSACCESS : csaccess(ep,lvf,LV);  break;
    case OPOINTTO : csaccess(ep,lvf,RV);  break;
    case OINDIRECT: cindirect(ep,lvf);    break;
    case OADDRESS : gen_ecode(ep->elp,LV);break;
    case OUMINUS  : cuop(ep,"NEG");       break;
    case OLNOT    : clnot(ep);            break;
    case OBNOT    : cuop(ep,"NOT");       break;
    case OLINC    : clid(ep,"ADD");       break;
    case OLDEC    : clid(ep,"SUB");       break;
    case OTCONV   : gen_ecode(ep->elp,RV);break;
    case ORINC    : crid(ep,"ADD");       break;
    case ORDEC    : crid(ep,"SUB");       break;
    case OMUL     : cbop(ep,"MULS");      break;
    case ODIV     : cbop(ep,"DIVS");      break;
    case OMOD     : cbop(ep,"MODS");      break;
    case OADD     : cadd(ep);             break;
    case OSUB     : csub(ep);             break;
    case OLSHIFT  : cbop(ep,"LSL");       break;
    case ORSHIFT  : cbop(ep,"LSR");       break;
    case OLESS    : ccop(ep,"LT");        break;
    case OGREAT   : ccop(ep,"GT");        break;
    case OLESSEQ  : ccop(ep,"LE");        break;
    case OGREATEQ : ccop(ep,"GE");        break;
    case OEQUAL   : ccop(ep,"EQ");        break;
    case ONOTEQ   : ccop(ep,"NE");        break;
    case OBAND    : cbop(ep,"AND");       break;
    case OBEOR    : cbop(ep,"EOR");       break;
    case OBOR     : cbop(ep,"OR");        break;
    case OLAND    : clop(ep,"EQ");        break;
    case OLOR     : clop(ep,"NE");        break;
    case OQUEST   : cquest(ep);           break;
    case OADDAS   : caddas(ep);           break;
    case OSUBAS   : csubas(ep);           break;
    case OMULAS   : casop(ep,"MULS");     break;
    case ODIVAS   : casop(ep,"DIVS");     break;
    case OMODAS   : casop(ep,"MODS");     break;
    case OLSHIFTAS: casop(ep,"LSL");      break;
    case ORSHIFTAS: casop(ep,"LSR");      break;
    case OBANDAS  : casop(ep,"AND");      break;
    case OBEORAS  : casop(ep,"EOR");      break;
    case OBORAS   : casop(ep,"OR");       break;
    case OASSIGN  : cassign(ep);          break;
    case OCOMMA   : ccomma(ep);           break;
   }
 }


void expcode() {
  mark();
  reg = 0;
  gen_ecode(mkexp(),RV);
  release();
 }

