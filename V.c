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
  hashItem T; /* The table */
};

typedef struct hash *hash;

hash H;

struct stackItem{
  int v; /* The value of the item. Copied from A */
  int ufi; /* Representing the set index for this value. */
};

typedef struct stackItem *stackItem;

struct stack{
  int a; /* Number of positions alloced */
  int top; /* Last element on the stack */
  int stub; /* Boolean for last call was to stub */
  stackItem M; /* Point to the actual stack */
};

typedef struct stack* stack;

stack S; /* Algorithms stack */

/* A union find array */
/* Negative numbers are ranks. Positive numbers are pointers */
typedef int *UF;

UF T; /* Array for UF data structure */
int *T2S; /* Map from UF data structure to stack position */

void Push(stack S)
{ /* Pushes element into the stack */
  S->top++;
  S->stub = 0;
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
  h->T = calloc(h->a, sizeof(struct hashItem));

  return h;
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
        && h->T[i].key != 1+key){
    i++;
    i %= h->a;
  }

  return i;
}

int get(hash h,
        int key
        )
{
  return h->T[findPosition(h, key)].value;
}

void
insert(hash h,
       int key,
       int value
       )
{
  int i = findPosition(h, key);

  h->T[i].key = key+1;
  h->T[i].value = value;
}

int
contains(hash h,
         int key
         )
{
  return h->T[findPosition(h, key)].key == key;
}

void
delete(hash h,
       int key /* A pointer to the point */
       )
{
  int i;

  i = findPosition(h, key);
  h->T[i].key = 0; /* Emptied position */

  i++;
  i %= h->a;
  while(0 != h->T[i].key){
    struct hashItem t;
    t.key = h->T[i].key-1;
    t.value = h->T[i].value;
    h->T[i].key = 0; /* Emptied position */
    insert(h, t.key, t.value);
    i++;
    i %= h->a;
  }
}

stack makeStack(int n)
{
  stack S = NULL;

  S = malloc(sizeof(struct stack));
  S->a = n+2;
  S->top = 0;
  S->stub = 0; /* means false */
  S->M = malloc(S->a*sizeof(struct stackItem));
  S->M[0].v = INT_MIN;
  Push(S);

  return S;
}

void freeStack(stack S)
{
  free(S->M);
  free(S);
}

stackItem
STop(stack S)
{
  S->stub = 0; /* means false */
  return &(S->M[S->top - 2]);
}

stackItem
Top(stack S)
{
  S->stub = 0; /* means false */
  return &(S->M[S->top - 1]);
}

int
wasStub(stack S)
{
  return S->stub;
}

stackItem
getStub(stack S)
{
  S->stub = 1; /* means true */
  return &(S->M[S->top]);
}

void
Pop(stack S)
{
  S->stub = 0; /* means false */
  S->top--;
}

UF makeUF(int n)
{
  UF A = NULL;
  int i; /* Counter */

  A = malloc(n*sizeof(int));
  i = 0;
  while(i < n){
    A[i] = -1; /* Initial rank */
    i++;
  }

  return A;
}

int Find(UF A, int q)
{
  static int LA[35]; /* Iterative find */
  int i;
  int p = q;

  i = 0;
  while(0 <= A[p]){
    LA[i] = p;
    i++;
    p = A[p]; /* Move up */
  }

  assert(i < 35 && "Hit limit");

  while(i > 0){
    i--;
    A[LA[i]] = p;
  }

  return p;
}

void Union(UF A, int p, int q)
{
  /* printf("Uniting %d %d\n", p, q); */

  int rp = Find(A, p);
  int rq = Find(A, q);

  if(A[rp] < A[rq])
    A[rq] = rp;
  else {
    if(A[rp] == A[rq])
      A[rq]--;
    A[rp] = rq;
  }
}

int
main(int argc, char** argv){

  int q;

  q = getInt();

  S = makeStack(q);
  H = makeHash(q);
  T = makeUF(q);
  T2S = malloc(q*sizeof(int));

  int ufc = 0; /* Counter for the UF structure */
  int pos; /* The position in the array */
  int c; /* Character being read. */
  int v; /* A value for the array */
  int qi; /* Query index. */
  stackItem sti; /* Stack item */

  pos = -1;
  c = getInt();
  while(0 <= load){ /* There is file to read */
    switch(c){
    case value:
      v = getInt();

      sti = Top(S);
      if(sti->v < v){ /* Element is larger put in new space */
        sti = getStub(S); /* Puts an empty item into the stack */
        sti->v = v;
      } else { /* Element is smaller contract stack */
        int pufi = -1; /* Previous UFi */
        if(sti->v > v){
          sti->v = v;
          pufi = sti->ufi;
        }
        sti = STop(S); /* Second to top */
        while(sti->v >= v){
          sti->v = v;
          Union(T, sti->ufi, pufi);
          T2S[Find(T, sti->ufi)] = S->top-2;
          pufi = sti->ufi;
          Pop(S); /* Remove top from stack */
          sti = STop(S);
        }
      }

      pos++; /* Increment position */
      break;

    case mark:
      /* ufc Is the index for the new set */
      insert(H, pos, ufc); /* Insert to hash */

      if(wasStub(S)){ /* Put on stub */
        getStub(S)->ufi = ufc;
        Push(S); /* Puts stub into the stack. */
      } else { /* Unite with Top */
        sti = Top(S);
        Union(T, ufc, sti->ufi);
      }

      T2S[Find(T, ufc)] = S->top-1;
      ufc++;

      break;

    case query: case closeQ: /* Queries */
      qi = getInt();
      qi--;

      /* In this version closing has almost no effect. */

      vout = S->M[T2S[Find(T, get(H, qi))]].v;

      printf("%d ", 1+qi);
      printf("%d ", pos);
      printf("%d\n", vout);

      if(closeQ == c) /* Close marking */
        delete(H, qi);
      break;
    default:
      break;
    }
    c = getInt();
  }

  free(T2S);
  free(T);
  free(H->T);
  free(H);
  freeStack(S);

  return 0;
}
