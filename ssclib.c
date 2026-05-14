#include "sscdef.h"

int inlab_num = 0;

int get_inlabel() { return (++inlab_num); }

/* -------------------------------------------------------- */

void error(char *mes) {
  printf("ERROR : %s\n", mes);
  exit(0);
}

/* -------------------------------------------------------- */

struct GLABEL {
  char *gidp;
  int ginlab, glabf;
  struct GLABEL *gnextp;
} *glab_root;

void init_label() { glab_root = NULL; }

int search_label(char *idp, int f) {
  struct GLABEL *p;

  for (p = glab_root; p != NULL && strcmp(idp, p->gidp) != 0; p = p->gnextp)
    ;
  if (p == NULL) {
    p = (struct GLABEL *)malloc(sizeof(struct GLABEL));
    p->gidp = malloc(strlen(idp) + 1);
    strcpy(p->gidp, idp);
    p->ginlab = get_inlabel();
    p->glabf = f;
    p->gnextp = glab_root;
    glab_root = p;
  } else if (f == DEF && p->glabf == DEF)
    error(" Double defined label");
  else if (f == DEF)
    p->glabf = DEF;
  return (p->ginlab);
}

void check_label() {
  struct GLABEL *p, *q;

  for (p = glab_root; p != NULL; p = q) {
    if (p->glabf == REF)
      error("Undefined label");
    free(p->gidp);
    q = p->gnextp;
    free((char *)p);
  }
}

/* -------------------------------------------------------- */

FILE *outfp;

void igen(char *name) {
  if ((outfp = fopen(name, "w")) == NULL)
    error("output file does not open");
}

void egen() { fclose(outfp); }

void gen_code0(char *format) {
  fprintf(outfp, format);
  fprintf(outfp, "\n");
}

void gen_code0i(char *format, int d1) {
  fprintf(outfp, format, d1);
  fprintf(outfp, "\n");
}

void gen_code0ii(char *format, int d1, int d2) {
  fprintf(outfp, format, d1, d2);
  fprintf(outfp, "\n");
}

void gen_code0p(char *format, char *d1) {
  fprintf(outfp, format, d1);
  fprintf(outfp, "\n");
}

void gen_code0pi(char *format, char *d1, int d2) {
  fprintf(outfp, format, d1, d2);
  fprintf(outfp, "\n");
}

void gen_code0pii(char *format, char *d1, int d2, int d3) {
  fprintf(outfp, format, d1, d2, d3);
  fprintf(outfp, "\n");
}

/*
void gen_code(char *format,int d1,int d2,int d3,int d4,int d5,int d6,int d7,int
d8)
 {
  fprintf(outfp,format,d1,d2,d3,d4,d5,d6,d7,d8);
  fprintf(outfp,"\n");
 }
 */

/* -------------------------------------------------------- */

struct DALLOC_DATA {
  struct DALLOC_DATA *ndp;
  char *thisp;
};

struct DALLOC_MARK {
  struct DALLOC_DATA *ndp;
  struct DALLOC_MARK *nmp;
} *root_mark = NULL;

void mark() {
  struct DALLOC_MARK *p;

  p = (struct DALLOC_MARK *)malloc(sizeof(struct DALLOC_MARK));
  p->ndp = NULL;
  p->nmp = root_mark;
  root_mark = p;
}

char *mmalloc(int size) {
  struct DALLOC_DATA *dp;

  dp = (struct DALLOC_DATA *)malloc(sizeof(struct DALLOC_DATA));
  dp->thisp = malloc(size);
  dp->ndp = root_mark->ndp;
  root_mark->ndp = dp;
  return (dp->thisp);
}

void release() {
  struct DALLOC_MARK *mp;
  struct DALLOC_DATA *dp, *dnp;

  if ((mp = root_mark) != NULL) {
    root_mark = mp->nmp;
    for (dp = mp->ndp; dp != NULL; dp = dnp) {
      dnp = dp->ndp;
      free(dp->thisp);
      free((char *)dp);
    }
    free((char *)mp);
  }
}
