/* MIT License */

/* Copyright (c) 2021 Luís M. S. Russo */

/* Permission is hereby granted, free of charge, to any person obtaining a copy */
/* of this software and associated documentation files (the "Software"), to deal */
/* in the Software without restriction, including without limitation the rights */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell */
/* copies of the Software, and to permit persons to whom the Software is */
/* furnished to do so, subject to the following conditions: */

/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE */
/* SOFTWARE. */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h> /* For double buffering */
#include "commands.h"

#define sureInline(X) __inline X __attribute__((__gnu_inline__, __always_inline__, __artificial__))

volatile int vout;

char buffer[BUFSIZ];
int *iBuffer;
int load = 0; /* Current buffer load for Consumer */
int bufferIdx = 0;


/* The main thread actually is the consumer */
static sureInline(int) getInt(void)
{
  if(0 == load){
    int rSize = read(0, &(buffer[0]), BUFSIZ);
    assert(0 == (rSize % 4) && "Broken integer read.");
    load = rSize;

    iBuffer = (int*)&(buffer[0]);
    bufferIdx = 0;
    if(0 == load) /* Prepare end of file */
      iBuffer[bufferIdx] = EOF;
  }

  load -= 4;
  return iBuffer[bufferIdx++];
}

static int primes[] = {
  3, 5, 7, 11, 17, 29, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
  49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,
  12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457,
  1610612741
};

struct hashItem {
  int key; /* Position over array A */
  int value; /* UF number */
};

typedef struct hashItem *hashItem;

struct hash{
  int a; /* Size of alloced Table */
  int n; /* Number of elements in the hash */
  hashItem T; /* The table */
};

typedef struct hash *hash;

struct stackItem{
  int v; /* The value of the item. Copied from A */
  int idx; /* Representing the position index of this value. */
};

typedef struct stackItem *stackItem;

struct stack{
  int a; /* Number of positions alloced */
  int stub; /* Last element on the stack */
  int stubQ; /* Boolean for last call was to stub */
  stackItem M; /* Point to the actual stack */
};

typedef struct stack* stack;

struct UFItem{
  int seti; /* Set index value */
  int stacki; /* Stack index */
};

typedef struct UFItem *UFItem;

struct UF{
  int a; /* Number of alloced positions */
  int lst; /* Last position index */
  UFItem L; /* List of sets */
};
/* A union find array */
/* Negative numbers are ranks. Positive numbers are pointers */
typedef struct UF *UF;

struct fastRMQ{
  int pos; /* Current position in array */
  stack S;
  hash H;
  UF T;
};

typedef struct fastRMQ *fastRMQ;

void Push(stack S)
{ /* Pushes element into the stack */
  S->stub++;
  S->stubQ = 0;
}

hash
makeHash(int n
        )
{
  hash h = NULL;
  int i;
  for(i = 0; primes[i] < n; i++)
    ;

  h = malloc(sizeof(struct hash));
  h->a = primes[i];
  h->n = 0;
  h->T = calloc(h->a, sizeof(struct hashItem));

  return h;
}

void
freeHash(hash *H
         )
{
  free((*H)->T);
  (*H)->T=NULL;
  free(*H);
  *H = NULL;
}

static unsigned int
hashFun(int key,
        int M /* Use size as modulus */
        )
{
  unsigned int r = 0;
  unsigned int a = 31415;
  const unsigned int b = 27183;

  unsigned char *S = (unsigned char *)&key;

  for(int i = 0; i < 4 ; i++){
    r = (a*r + *S) % M;
    S++;
    a = (a*b) % M;
  }

  return r;
}

static int
findPosition(hash h,
             int key
             )
{
  int i = hashFun(key, h->a);

  while(0 != h->T[i].key
        && h->T[i].key != key){
    i++;
    i %= h->a;
  }

  return i;
}

int
get(hash h,
        int key
        )
{
  /* Bypass negative signs */
  return abs(h->T[findPosition(h, key)].value);
}

void
insert(hash h,
       int key,
       int value
       )
{
  assert(0 < key && "Inserting invalid key.");

  int i = findPosition(h, key);

  h->T[i].key = key;
  h->T[i].value = value;
  h->n++;
}

int
contains(hash h,
         int key
         )
{
  return h->T[findPosition(h, key)].key == key;
}

/* Do not really remove elements, just mark. */
void
markDelete(hash h,
       int key /* A pointer to the point */
       )
{
  int i;

  i = findPosition(h, key);
  h->T[i].value *= -1; /* Swap sign */
  h->n--;
}

stack makeStack(int n)
{
  stack S = NULL;

  S = malloc(sizeof(struct stack));
  S->a = n+2; /*  */
  S->stub = 0;
  S->stubQ = 0; /* means false */
  S->M = malloc(S->a*sizeof(struct stackItem));
  S->M[0].v = INT_MIN;
  S->M[0].idx = 0; /* Simple clean value */
  Push(S);

  return S;
}

void freeStack(stack *S)
{
  free((*S)->M);
  (*S)->M = NULL;
  free(*S);
  *S = NULL;
}

stackItem
STop(stack S)
{
  S->stubQ = 0; /* means false */
  return &(S->M[S->stub - 2]);
}

stackItem
Top(stack S)
{
  S->stubQ = 0; /* means false */
  return &(S->M[S->stub - 1]);
}

stackItem
getStub(stack S)
{
  S->stubQ = 1; /* means true */
  return &(S->M[S->stub]);
}

int
wasStubQ(stack S)
{
  return S->stubQ;
}

void
Pop(stack S)
{
  S->stubQ = 0; /* means false */
  S->stub--;
}

UF makeUF(int n)
{
  UF T = NULL;

  T = malloc(sizeof(struct UF));
  T->a = n+1;
  T->lst = 1; /* Need to waste position 0 for hash value consistency */
  T->L = (UFItem) malloc((T->a)*sizeof(struct UFItem));

  int i; /* Counter */
  i = 0;
  while(i < T->a){
    T->L[i].seti = -1; /* Initial rank */
    i++;
  }

  return T;
}

void freeUF(UF *T)
{
  free((*T)->L);
  (*T)->L = NULL;
  free(*T);
  *T = NULL;
}

int Find(UF T, int q)
{
  UFItem A = T->L;

  static int LA[35]; /* Iterative find */
  int i;
  int p = q;

  i = 0;
  while(0 <= A[p].seti){
    LA[i] = p;
    i++;
    p = A[p].seti; /* Move up */
  }

  assert(i < 35 && "Hit limit");

  while(i > 0){
    i--;
    A[LA[i]].seti = p;
  }

  return p;
}

void Union(UF T, int p, int q)
{
  int rp = Find(T, p);
  int rq = Find(T, q);

  if(rp != rq){
    UFItem A = T->L;

    if(A[rp].seti < A[rq].seti)
      A[rq].seti = rp;
    else {
      if(A[rp].seti == A[rq].seti)
        A[rq].seti--;
      A[rp].seti = rq;
    }

    if(A[rp].stacki > A[rq].stacki)
      A[rp].stacki = A[rq].stacki;
    else
      A[rq].stacki = A[rp].stacki;
  }
}

fastRMQ
makeRMQ(int a /* Alloc size */
	)
{
  fastRMQ R = NULL;

  R = malloc(sizeof(struct fastRMQ));
  R->S = makeStack(a);
  R->H = makeHash(2*a);
  R->T = makeUF(a);
  R->pos = 1; /* 0 has no sign */

  return R;
}

fastRMQ
makeNewRMQ(fastRMQ old
	   )
{

  int a = 2*old->H->n;


  fastRMQ new = makeRMQ(a);
  new->pos = old->pos;
  new->S->stubQ = old->S->stubQ;

  /* Traverse old stack */
  int i = 1;
  while(i < old->S->stub){
    old->S->M[i].idx = -1; /* Mark inactive */
    i++;
  }

  i = 0;
  while(i < old->H->a){ /* Traverse Hash */
    if(0 != old->H->T[i].key &&
       0 < old->H->T[i].value){ /* Active entries */
      /* Put in new hash */
      insert(new->H, old->H->T[i].key, new->T->lst);
      new->T->lst++; /* For now you do not know where it is going to go in S. */

      int ufi = old->H->T[i].value;
      ufi = Find(old->T, ufi); /* Change to root */

      int stacki = old->T->L[ufi].stacki;
      if(0 > old->S->M[stacki].idx) /* Reactivate stack entry */
        old->S->M[stacki].idx = old->H->T[i].key;
    }
    i++;
  }

  /* Now compact stack S */
  int j = 1; /* New Stack positions */
  i = 1;
  while(i < old->S->stub){
    if( 0 < old->S->M[i].idx){   /*  Only active entries */
      new->S->M[j].v = old->S->M[i].v;
      old->S->M[i].v = j; /* Overwrite value */
      j++;
    }
    i++;
  }
  /* Process stub */
  new->S->stub = j;
  new->S->M[j].v = old->S->M[i].v;
  new->S->M[j].idx = old->S->M[i].idx;

  /* Now go through the Hash again */
  j = 1;
  i = 0;
  while(i < old->H->a){ /* Traverse Hash */
    if(0 != old->H->T[i].key &&
       0 < old->H->T[i].value){ /* Active entries */

      int ufi = old->H->T[i].value;
      ufi = Find(old->T, ufi); /* Change to root */

      int stacki = old->T->L[ufi].stacki;

      /* Put in UFI */
      int sidx = old->S->M[stacki].v; /* Use overwritten values */
      new->T->L[j].stacki = sidx;
      new->S->M[sidx].idx = old->H->T[i].key;
      j++;
    }
    i++;
  }

  /* Finally go for Unions */
  i = 1;
  while(i < j){
    int stacki = new->T->L[i].stacki;
    int idx = new->S->M[stacki].idx;
    int ufi = get(new->H, idx);
    Union(new->T, i, ufi);
    i++;
  }

  return new;
}

void freeRMQ(
             fastRMQ *R
             )
{
  freeUF(&((*R)->T));
  freeHash(&((*R)->H));
  freeStack(&((*R)->S));
  free(*R);
  *R=NULL;
}

void
printRMQ(fastRMQ F)
{
  printf("Printing RMQ\n");
  printf("pos: %d\n", F->pos);
  printf("Stack >> \t");
  printf("stub: %d \t", F->S->stub);
  printf("stubQ: %d \n", F->S->stubQ);
  int i = 0;
  while(i <= F->S->stub){
    printf(">> Idx [%d] ", i);
    printf(">> v: %d \t", F->S->M[i].v);
    printf("idx: %d \n", F->S->M[i].idx);
    i++;
  }
  printf("\n");

  printf("Hash >>\n");
  printf("n: %d \n", F->H->n);
  i = 0;
  while(i < F->H->a){
    if(0 != F->H->T[i].key){
      printf(">> %d -> %d\n", F->H->T[i].key,
	     F->H->T[i].value);
    }
    i++;
  }
  printf("\n");

  printf("UF >>\n");
  i = 0;
  while(i < F->T->lst){
    printf(">> Idx [%d] ", i);
    printf("= %d \t", F->T->L[i].seti);
    printf("stckI: %d\n", F->T->L[i].stacki);
    i++;
  }
}


void
process(fastRMQ F, int v)
{ /* Read int c from the input */
  /* printf("Process %d\n", v); */

  stackItem sti = Top(F->S);

  if(sti->v <= v){ /* Element is larger put in new space */
    sti = getStub(F->S); /* Puts an empty item into the stack */
    sti->v = v;
    sti->idx = F->pos;
  } else { /* Element is smaller contract stack */
    int ufi = get(F->H, sti->idx); /* UFindex */
    stackItem ssti = STop(F->S);
    while(ssti->v >= v){
      Union(F->T, get(F->H, ssti->idx), ufi);
      Pop(F->S); /* Remove top from stack */
      ssti = STop(F->S);
    }
    Top(F->S)->v = v;
  }
  F->pos++; /* Increment position */
}

void
markCmd(fastRMQ *PF)
{
  /* printf("Mark\n"); */
  fastRMQ F = *PF;

  if(F->T->lst == F->T->a){ /* UF structure is full. */
    /* printf("Before RMQ transfer\n"); */
    /* printRMQ(F); */

    fastRMQ new = makeNewRMQ(F);
    freeRMQ(PF);
    *PF = new;
    F = *PF;

    /* printf("After RMQ transfer\n"); */
    /* printRMQ(F); */
  }

  /* Add to Hash */
  insert(F->H, F->pos-1, F->T->lst);

 /* Add to UF */
  F->T->L[F->T->lst].stacki = F->S->stub;

  /* Add to Stack, if needed */
  if(wasStubQ(F->S)) /* Last command was stub */
    Push(F->S); /* Put stub on stack */
  else
    Union(F->T, F->T->lst, get(F->H, Top(F->S)->idx));

  F->T->lst++; /* Finish UF add */
}

int
queryCmd(fastRMQ F, int p)
{ /* p is previous position */
  /* printf("Query %d\n", p); */

  int ufi = get(F->H, p); /* UFindex */
  int rootUFI = Find(F->T, ufi);
  int Sidx = F->T->L[rootUFI].stacki; /* Stack Index */

  return F->S->M[Sidx].v;
}

void
RMQAssert(fastRMQ F)
{
  assert(2*F->H->n <= F->H->a && "Hash overflow");
  assert(F->T->lst <= F->T->a && "UF overflow");
  assert(F->S->stub <= F->S->a && "UF overflow");

  int i;
  int j;

  i = 1;
  while(i < F->S->stub){
    if(contains(F->H, F->S->M[i].idx)){
      j = i+1;
      while(j < F->S->stub){
	if(contains(F->H, F->S->M[j].idx)){
	  assert(Find(F->T, get(F->H, F->S->M[i].idx))
		 != Find(F->T, get(F->H, F->S->M[j].idx))
		 && "Mixed sets in stack");
	}
	j++;
      }
    }
    i++;
  }

  i = 1;
  while(i < F->S->stub){
    assert(i == F->T->L[Find(F->T, get(F->H, F->S->M[i].idx))].stacki
           && "Failed Stack index" );
    i++;
  }

  i = 1;
  while(i+1 < F->S->stub){
    assert(F->S->M[i].v < F->S->M[i+1].v
           && "Failed Stack index" );
    i++;
  }
}


int
main(int argc, char** argv){

  int q;

  q = getInt();

  fastRMQ F = makeRMQ(4);
  int c; /* Character being read. */
  int idx;

  /* RMQAssert(F); */

  c = getInt();
  while(0 <= load){ /* There is file to read */

    assert(F->T->L[0].seti == -1 && "touched first set");
    /* printRMQ(F); */
    switch(c){
    case value:
      process(F, getInt());
      break;

    case mark:
      markCmd(&F);
      break;

    case query: case closeQ: /* Queries */
      idx = getInt();
      idx--;
      vout = queryCmd(F, 1+idx);

      printf("%d ", 1+idx);
      printf("%d ", F->pos-2);
      printf("%d\n", vout);

      if(closeQ == c) /* Close marking */
        markDelete(F->H, 1+idx);
      break;
    default:
      break;
    }
    /* RMQAssert(F); */
    c = getInt();
  }

  /* printRMQ(F); */

  freeRMQ(&F);

  return 0;
}
