/**************************************************************************/
/*                                                                        */
/*  Sample C compiler                                                     */
/*    Original version was made for the text-book "Compiler" (1996)       */
/*      coded by Yoshihiro Tsujino in 1995, and                           */
/*    lastly updated for the text-book "Compiler" second edition (2019)   */
/*      at Dec 13, 2019 by Yoshihiro Tsujino.                             */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tokens */
#define SNULL 1
#define SLNOT 2
#define SDQUOTE 3
#define SMOD 4
#define SBAND 5
#define SSQUOTE 6
#define SLPAREN 7
#define SRPAREN 8
#define SSTAR 9
#define SPLUS 10
#define SCOMMA 11
#define SMINUS 12
#define SDOT 13
#define SDIV 14
#define SDIGIT 15
#define SCOLON 16
#define SSEMI 17
#define SLESS 18
#define SASSIGN 19
#define SGREAT 20
#define SQUEST 21
#define SALPHA 22
#define SLSQP 23
#define SRSQP 24
#define SBEOR 25
#define SLBRACE 26
#define SBOR 27
#define SRBRACE 28
#define SBNOT 29
#define SEOF 30
#define SIDENT 31
#define SSTR 32
#define SCONST 33
#define SLAND 34
#define SLOR 35
#define SEQUAL 36
#define SDEC 37
#define SINC 38
#define SPOINTTO 39
#define SLSHIFT 40
#define SLESSEQ 41
#define SRSHIFT 42
#define SGREATEQ 43
#define SNOTEQ 44
#define SBREAK 45
#define SCASE 46
#define SCHAR 47
#define SCONTINUE 48
#define SDEFAULT 49
#define SDO 50
#define SELSE 51
#define SEXTERN 52
#define SFOR 53
#define SGOTO 54
#define SIF 55
#define SINT 56
#define SRETURN 57
#define SSIZEOF 58
#define SSTRUCT 59
#define SSWITCH 60
#define SWHILE 61

/* Types */
#define TCHAR 100
#define TINT 101
#define TARRAY 102
#define TSTRUCT 103
#define TPOINTER 104
#define TFUNCT 105

/* Attributes */
#define LOCAL 200
#define GLOBAL 201
#define EXTERNAL 202
#define TAG 203
#define MEMBER 204

/* Flags */
#define DEF 300
#define REF 301
#define LV 302
#define RV 303

/* Operators */
#define OIDENT 400
#define OSTR 401
#define OCONST 402
#define OSCOMMA 403
#define OFCALL 404
#define OAACCESS 405
#define OSACCESS 406
#define OPOINTTO 407
#define OINDIRECT 408
#define OADDRESS 409
#define OUMINUS 410
#define OLNOT 411
#define OBNOT 412
#define OLINC 413
#define OLDEC 414
#define OTCONV 415
#define ORINC 416
#define ORDEC 417
#define OMUL 418
#define ODIV 419
#define OMOD 420
#define OADD 421
#define OSUB 422
#define OLSHIFT 423
#define ORSHIFT 424
#define OLESS 425
#define OGREAT 426
#define OLESSEQ 427
#define OGREATEQ 428
#define OEQUAL 429
#define ONOTEQ 430
#define OBAND 431
#define OBEOR 432
#define OBOR 433
#define OLAND 434
#define OLOR 435
#define OCOLON 436
#define OQUEST 437
#define OADDAS 438
#define OSUBAS 439
#define OMULAS 440
#define ODIVAS 441
#define OMODAS 442
#define OLSHIFTAS 443
#define ORSHIFTAS 444
#define OBANDAS 445
#define OBEORAS 446
#define OBORAS 447
#define OASSIGN 448
#define OCOMMA 449

/* Sizes */
#define KEY_SIZE 17
#define INSTRSIZE 512
#define PARA_TAB_SIZE 32
/* Max num. of parameters*/

/* Types */
struct TYPE {
  int ttype, tsize, asize;
  struct TYPE *ttp;
};

struct ID {
  char *sid;
  struct TYPE *itp;
  int attr, addr;
  struct ID *lidp, *ridp;
};

struct ENODE {
  int opr;
  struct TYPE *etp;
  int lv;
  struct ENODE *elp, *erp;
  int elpi; /* value in the case of constant */
};

/* Function expcode*/

/*
void cload(int t, int s, int d, int w);
void cstore(int t, int s, int d);
void cident(struct ENODE *ep, int lvf);
void cstr(struct ENODE *ep);
void cconst(struct ENODE *ep);
void cscomma(struct ENODE *ep, int *cp);
void cfcall(struct ENODE *ep);
void csaccess(struct ENODE *ep, int lvf, int lr);
void caaccess(struct ENODE *ep, int lvf);
void cindirect(struct ENODE *ep,int lvf);
void cuop(struct ENODE *ep, char *op);
void clnot(struct ENODE *ep);
void clid(struct ENODE *ep, char *op);
void crid(struct ENODE *ep, char *op);
void cbop(struct ENODE *ep, char *op);
void cadd(struct ENODE *ep);
void csub(struct ENODE *ep);
void ccop(struct ENODE *ep, char *op);
void clop(struct ENODE *ep, char *op);
void cquest(struct ENODE *ep);
void casop(struct ENODE *ep, char *op);
void caddas(struct ENODE *ep);
void csubas(struct ENODE *ep);
void cassign(struct ENODE *ep);
void ccomma(struct ENODE *ep);
*/

void gen_ecode(struct ENODE *ep, int lvf);
void expcode();

/* Function expres */

/*
struct ENODE *get_enode(int o, struct TYPE *tp,int lvf, struct ENODE *lp, struct
ENODE *rp); struct ENODE *mkpexp(); int istype(); struct ENODE *mkuexp(); struct
ENODE *mkmulexp(); struct ENODE *mkaddexp(); struct ENODE *mksexp(); struct
ENODE *mkueexp(); struct ENODE *mkeexp(); struct ENODE *mkbaexp(); struct ENODE
*mkbeexp(); struct ENODE *mkboexp(); struct ENODE *mklaexp(); struct ENODE
*mkloexp(); struct ENODE *mkcexp();
*/

struct ENODE *mkasexp();
struct ENODE *mkexp();

/*Function st */

/*
void compound_st(int blab, int clab);
void if_st(int blab, int clab);
void while_st();
void do_st();
void for_st();
void switch_st(int clab);
void break_cont_st(int lab);
void return_st();
void goto_st();
void labeled_st(int blab, int clab);
void exp_st();
*/

void st(int blab, int clab);

/* Function decl */

struct TYPE *get_tnode(int tt, int ts, int as, struct TYPE *tp);
struct ID *get_inode(char *id, struct TYPE *tp, int at, int ad);

/*
void push_scope();
void pop_scope();
void sub_insert_id(struct ID **root, char *id, struct TYPE *tp, int at,int ad);
void insert_id(char *id, struct TYPE *tp, int at, int ad);
void para_decls();
*/

struct ID **sub_search_id(char *id, struct ID **root);
struct ID *search_id(char *id);
void size_check(struct TYPE *tp);
struct TYPE *tpspec();
struct TYPE *subdecls(struct TYPE *tp, char **id);
void init_decls();
void local_decls();
void end_block();
void ext_decls();

/* Function ssclib */

int get_inlabel();
void error(char *mes);
void init_label();
int search_label(char *idp, int f);
void check_label();
void igen(char *name);
void egen();

void gen_code0(char *format);
void gen_code0i(char *format, int d1);
void gen_code0ii(char *format, int d1, int d2);
void gen_code0p(char *format, char *d1);
void gen_code0pi(char *format, char *d1, int d2);
void gen_code0pii(char *format, char *d1, int d2, int d3);
/* void gen_code(char *format,int d1,int d2,int d3,int d4,int d5,int d6,int
 * d7,int d8); */

void mark();
char *mmalloc(int size);
void release();

/* Function scan */

/*
int which_keyword();
void cscan();
void uncscan()
void bsscan();
void in_scan();
*/
void iscan(char *np);
void escan();
int nscan();
int scan();

/* Global var. */
extern int tok;
extern char *scan_str;
extern int scan_const;

extern int local_size;
extern struct TYPE *charp, *intp;
