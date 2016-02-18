
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mparser.h"

FILE *in, *out;
char cbuf[4096];


int getString(const char *str) {

    char buf[1024];

    rewind(in);

    while(1) {
        int n = fscanf(in,"%s", buf);
        if(n <= 0) return 0;

        if(strcmp(buf, str) == 0) return 1;
    }

    return 0;
}

int nonspace(char ch) {

    switch(ch) {
    case ' ':
    case '\r':
    case '\n':
    case '\t':
        return 0;

    default:
        return 1;

    }

}

int getLine() {

    int ind = 0;
    char ch;

    do {
        ch = fgetc(in);
        if(feof(in)) return 0;
        if(nonspace(ch)) break;
    } while(1);

    cbuf[ind++] = ch;

    do {
        ch = fgetc(in);
        if(feof(in)) return 0;
        if(!nonspace(ch)) break;
        cbuf[ind++] = ch;
    } while(1);

    cbuf[ind] = '\0';

    return 1;
}

int getArgument(char *buf) {

    if( !getLine() ) return 0;

    int n = strlen(cbuf);
    if(n == 0) return 0;
    if(cbuf[0]==':') return 0;

    strncpy(buf,cbuf,n);

    return 1;
}

char targetStr[64]={'\0'};
char bldDirStr[1024]={'\0'};
char compilerStr[1024]={'\0'};
char compilerArgsStr[2048]={'\0'};
char linkerStr[1024]={'\0'};
char linkerArgsStr[2048]={'\0'};
char includesStr[2048]={'\0'};


int numSources;

typedef struct SrcPtr_ {

    char *ptr;
    SrcPtr_ *next;

} SrcPtr, *pSrcPtr;

SrcPtr baseSrc = {NULL, NULL};
pSrcPtr lastSrcPtr=&baseSrc;

int getNextToken(char *ptr) {

    int n = fscanf(in, "%s",ptr);
    if( n<=0) return 0;
    if(ptr[0]==':') return 0;
    return 1;

}


void putSrc(char *str) {

    int n = strlen(str);
    char *nStr = new char [n+1];
    strncpy(nStr, str, n);

    lastSrcPtr->ptr = nStr;
    lastSrcPtr->next = new SrcPtr;
    lastSrcPtr = lastSrcPtr->next;

    numSources++;

}



int getAllSources() {
    int r = 0;
    char src[256];

    while(1) {
        if(getNextToken(src) == 0)  break;
        putSrc(src);
    }

    pSrcPtr ps = &baseSrc;
    for(int i=0;i<numSources;i++) {
        printf("Src[%d]=%s\n", i, ps->ptr);
        ps = ps->next;
    }

    return r;
}


int getPrimitives() {


    int r = 1;
    while(1) {
        if( getString(":Target:") == 0) break;
        if( getArgument(targetStr) == 0) break;

        if( getString(":BuildDirectory:") == 0) break;
        if( getArgument(bldDirStr) == 0) break;

        if( getString(":Compiler:") == 0) break;
        if( getArgument(compilerStr) == 0) break;

        if( getString(":CompilerArguments:") == 0) break;
        getArgument(compilerArgsStr);

        if( getString(":Linker:") == 0) break;
        if( getArgument(linkerStr) == 0) break;

        if( getString(":LinkerArguments:") == 0) break;
        getArgument(linkerArgsStr);

        if( getString(":Includes:") == 0) break;
        getArgument(includesStr);

        fprintf(out,"TARGET=%s\n", targetStr);
        fprintf(out,"BUILDDIR=%s\n", bldDirStr);
        fprintf(out,"COMPILER=%s\n", compilerStr);
        fprintf(out,"CMPLARGS=%s\n", compilerArgsStr);
        fprintf(out,"INCLUDES=%s\n", includesStr);
        fprintf(out,"LINKER=%s\n", linkerStr);
        fprintf(out,"LNKARGS=%s\n", linkerArgsStr);


        if( getString(":Sources:") == 0) break;

        getAllSources();


        r = 0;
        break;
    }


    if(r) {
        printf("Error in getting primitives\n");
    }
    return r;
}

int mparser(FILE *cf, FILE *of) {

    int res=1;

    in = cf;
    out = of;

    while(1) {
        if( getPrimitives() ) break;


        res = 0;
        break;
    }

    if(res) {
        printf("Error in mparser\n");
    }
    return res;
}
