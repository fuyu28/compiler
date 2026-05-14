#include "sscdef.h"

char ctable[257] = {
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SLNOT,   SDQUOTE, SNULL,   SNULL,  SMOD,    SBAND,  SSQUOTE,
    SLPAREN, SRPAREN, SSTAR,   SPLUS,   SCOMMA, SMINUS,  SDOT,   SDIV,
    SDIGIT,  SDIGIT,  SDIGIT,  SDIGIT,  SDIGIT, SDIGIT,  SDIGIT, SDIGIT,
    SDIGIT,  SDIGIT,  SCOLON,  SSEMI,   SLESS,  SASSIGN, SGREAT, SQUEST,
    SNULL,   SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SLSQP,   SNULL,  SRSQP,   SBEOR,  SALPHA,
    SNULL,   SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SLBRACE, SBOR,   SRBRACE, SBNOT,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SALPHA,  SALPHA,  SALPHA,  SALPHA,  SALPHA, SALPHA,  SALPHA, SALPHA,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SNULL,   SNULL,   SNULL,   SNULL,   SNULL,  SNULL,   SNULL,  SNULL,
    SEOF};

FILE *infp;
int cbuf = '\0'; /* character buffer */
int next_token, in_const;
char in_str[INSTRSIZE];
char *scan_str;
int scan_const;

struct KEY {
  char *keyword;
  int symbol;
} key[KEY_SIZE] = {
    {"break", SBREAK},       {"case", SCASE},       {"char", SCHAR},
    {"continue", SCONTINUE}, {"default", SDEFAULT}, {"do", SDO},
    {"else", SELSE},         {"extern", SEXTERN},   {"for", SFOR},
    {"goto", SGOTO},         {"if", SIF},           {"int", SINT},
    {"repeat", SREPEAT},     {"return", SRETURN},   {"sizeof", SSIZEOF},
    {"struct", SSTRUCT},     {"switch", SSWITCH},   {"until", SUNTIL},
    {"while", SWHILE}};

int which_keyword() {
  int l, r, c, i;

  l = 0;
  r = KEY_SIZE - 1;
  while (l <= r) {
    c = (l + r) >> 1;
    i = strcmp(in_str, key[c].keyword);
    if (i < 0)
      r = c - 1;
    else if (i > 0)
      l = c + 1;
    else
      return (key[c].symbol);
  }
  return (SIDENT);
}

void cscan() {
  cbuf = getc(infp);
  if (cbuf < 0 || 255 < cbuf)
    cbuf = 256;
}

void uncscan() { ungetc(cbuf, infp); }

void bsscan() {
  int i;

  cscan();
  if (ctable[cbuf] == SDIGIT) {
    i = cbuf - '0';
    cscan();
    if (ctable[cbuf] == SDIGIT) {
      i = (i << 3) + (cbuf - '0');
      cscan();
      if (ctable[cbuf] == SDIGIT)
        i = (i << 3) + (cbuf - '0');
      else
        uncscan();
    } else
      uncscan();
    cbuf = i;
  } else {
    switch (cbuf) {
    case 'n':
      cbuf = '\n';
      break;
    case 't':
      cbuf = '\t';
      break;
    case 'b':
      cbuf = '\\';
      break;
    case 'r':
      cbuf = '\r';
      break;
    case 'f':
      cbuf = '\f';
      break;
    }
  }
}

void in_scan() {
  char *p;

START:
  while ((next_token = ctable[cbuf]) == SNULL)
    cscan();
  switch (next_token) {
  case SALPHA:
    p = in_str;
    do {
      *p++ = cbuf;
      cscan();
    } while (ctable[cbuf] == SALPHA || ctable[cbuf] == SDIGIT);
    *p = '\0';
    next_token = which_keyword();
    break;
  case SDIGIT:
    in_const = cbuf - '0';
    cscan();
    if (in_const == 0) {
      if (cbuf == 'X' || cbuf == 'x') {
        cscan();
        while ('0' <= cbuf && cbuf <= '9' || 'a' <= cbuf && cbuf <= 'f' ||
               'A' <= cbuf && cbuf <= 'F') {
          if ('0' <= cbuf && cbuf <= '9')
            in_const = (in_const << 4) + (cbuf - '0');
          else if ('a' <= cbuf && cbuf <= 'f')
            in_const = (in_const << 4) + (cbuf - 'a' + 10);
          else if ('A' <= cbuf && cbuf <= 'F')
            in_const = (in_const << 4) + (cbuf - 'A' + 10);
          cscan();
        }
      } else {
        while ('0' <= cbuf && cbuf <= '9') {
          in_const = (in_const << 3) + (cbuf - '0');
          cscan();
        }
      }
    } else {
      while ('0' <= cbuf && cbuf <= '9') {
        in_const = (in_const * 10) + (cbuf - '0');
        cscan();
      }
    }
    next_token = SCONST;
    break;
  case SSQUOTE:
    cscan();
    if (cbuf == '\\')
      bsscan();
    if (ctable[cbuf] == SEOF)
      error("Illegal character constant");
    in_const = cbuf;
    next_token = SCONST;
    cscan();
    if (ctable[cbuf] != SSQUOTE)
      error(" ''' Expected");
    cscan();
    break;
  case SDQUOTE:
    cscan();
    p = in_str;
    while (ctable[cbuf] != SDQUOTE) {
      if (cbuf == '\\')
        bsscan();
      if (ctable[cbuf] == SEOF)
        error("Illegal string");
      *p++ = cbuf;
      cscan();
    }
    *p = '\0';
    next_token = SSTR;
    cscan();
    break;
  case SBAND:
    cscan();
    if (ctable[cbuf] == SBAND) {
      next_token = SLAND;
      cscan();
    }
    break;
  case SBOR:
    cscan();
    if (ctable[cbuf] == SBOR) {
      next_token = SLOR;
      cscan();
    }
    break;
  case SASSIGN:
    cscan();
    if (ctable[cbuf] == SASSIGN) {
      next_token = SEQUAL;
      cscan();
    }
    break;
  case SPLUS:
    cscan();
    if (ctable[cbuf] == SPLUS) {
      next_token = SINC;
      cscan();
    }
    break;
  case SMINUS:
    cscan();
    if (ctable[cbuf] == SMINUS) {
      next_token = SDEC;
      cscan();
    } else if (ctable[cbuf] == SGREAT) {
      next_token = SPOINTTO;
      cscan();
    }
    break;
  case SLESS:
    cscan();
    if (ctable[cbuf] == SLESS) {
      next_token = SLSHIFT;
      cscan();
    } else if (ctable[cbuf] == SASSIGN) {
      next_token = SLESSEQ;
      cscan();
    }
    break;
  case SGREAT:
    cscan();
    if (ctable[cbuf] == SGREAT) {
      next_token = SRSHIFT;
      cscan();
    } else if (ctable[cbuf] == SASSIGN) {
      next_token = SGREATEQ;
      cscan();
    }
    break;
  case SLNOT:
    cscan();
    if (ctable[cbuf] == SASSIGN) {
      next_token = SNOTEQ;
      cscan();
    }
    break;
  case SDIV:
    cscan();
    if (ctable[cbuf] == SSTAR) {
      cscan();
      do {
        while (ctable[cbuf] != SSTAR && ctable[cbuf] != SEOF)
          cscan();
        if (ctable[cbuf] == SEOF)
          error("Illegal comment");
        cscan();
      } while (ctable[cbuf] != SDIV);
      cscan();
      goto START;
    }
    break;
  default:
    cscan();
    break;
  }
}

void iscan(char *np) {
  if ((infp = fopen(np, "r")) == NULL)
    error("Input file does not open");
  cscan();
  in_scan();
}

void escan() { fclose(infp); }

int nscan() { return (next_token); }

int scan() {
  int token;

  switch (next_token) {
  case SIDENT:
  case SSTR:
    scan_str = mmalloc(strlen(in_str) + 1);
    strcpy(scan_str, in_str);
    break;
  case SCONST:
    scan_const = in_const;
    break;
  }
  token = next_token;
  in_scan();
  printf("Token = %d\n", token);
  return (token);
}
