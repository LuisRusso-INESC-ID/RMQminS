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


#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "commands.h"

char buffer[BUFSIZ];
int *iBuffer = (int *)&(buffer[0]);
int load = 0;
int bufferIdx = 0;

static void pushInt(int w)
{
  ssize_t wriRet;

  if(BUFSIZ == load){
    wriRet = write(1, buffer, BUFSIZ);
    assert(BUFSIZ == wriRet && "Broken integer write.");
    bufferIdx = 0;
    load = 0;
  }

  load += 4;
  iBuffer[bufferIdx++] = w;
}

int
main(int argc, char **argv)
{

  FILE *input = fopen(argv[1], "r");

  int c;
  int q = 0;

  while((c = getc(input))!= EOF){
    if('M' == c)
      q++;
  }

  pushInt(q);
  fclose(input);

  input = fopen(argv[1], "r");

  while((c = getc(input))!= EOF){
    int nv;
    switch(c){
    case 'V':
      pushInt(value);
      fscanf(input, "%d", &nv);
      pushInt(nv);
      break;
    case 'Q':
      pushInt(query);
      fscanf(input, "%d", &nv);
      pushInt(nv);
      break;
    case 'M':
      pushInt(mark);
      break;
    case 'C':
      pushInt(closeQ);
      fscanf(input, "%d", &nv);
      pushInt(nv);
      break;
    }
  }

  fclose(input);
  write(1, buffer, load);

  return 0;
}
