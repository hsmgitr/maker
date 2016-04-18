
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mparser.h"


const char *delim = "/";


FILE *in, *out;
char cbuf[4096];


void fprintLine() {
    fputs("\n", out);
}

char* getDirectory(char *src) {
    static char buf[256];

    int n = strlen(src)-1;
    while(n--) {
        if(src[n]=='/') break;
    }
    if(n>=0) {
        strncpy(buf,src,n);
        buf[n] = 0;
        return buf;
    }
    return NULL;
}

void fprintStrs(const char *args[]) {

    int i=0;
    while(1) {
        if(args[i] == NULL) break;
        fprintf(out,"%s ",args[i++] );
    }

}



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


int getToken( ) {

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

int getLine( ) {

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
        if((ch=='\r') || (ch=='\n')) break;
        cbuf[ind++] = ch;
    } while(1);

    cbuf[ind] = '\0';

    return 1;
}

int getArgument(char *buf) {

    if( !getToken() ) return 0;

    int n = strlen(cbuf);
    if(n == 0) return 0;
    if(cbuf[0]==':') return 0;

    strncpy(buf,cbuf,n);

    return 1;
}

char targetStr[64]={'\0'};
char bldDirStr[1024]={'\0'};
char compilerStr[1024]={'\0'};
char linkerStr[1024]={'\0'};


int numSources;

typedef struct SrcPtr_ {

    char *ptr;
    SrcPtr_ *next;

} SrcPtr, *pSrcPtr;

SrcPtr baseSrc = {NULL, NULL};
pSrcPtr lastSrcPtr=&baseSrc;

SrcPtr incSrc = {NULL, NULL};
pSrcPtr lastIncPtr = &incSrc;

SrcPtr libsSrc = {NULL, NULL};
pSrcPtr lastLibsPtr = &libsSrc;

SrcPtr cmplSrc = {NULL, NULL};
pSrcPtr lastCmplPtr = &cmplSrc;

SrcPtr lnkSrc = {NULL, NULL};
pSrcPtr lastLnkPtr = &lnkSrc;

SrcPtr envSrc = {NULL, NULL};
pSrcPtr lastEnvPtr = &envSrc;


int getNextToken(char *ptr) {

    int n = fscanf(in, "%s",ptr);
    if( n<=0) return 0;
    if(ptr[0]==':') return 0;
    return 1;

}


void putSrc(pSrcPtr *psPtr, char *str) {

    pSrcPtr ps=*psPtr;

    int n = strlen(str);
    char *nStr = new char [n+1];
    strncpy(nStr, str, n);

    ps->ptr = nStr;
    ps->next = new SrcPtr;
    *psPtr = ps->next;

}



int getAllSources() {
    int r = 0;
    char src[256];

    while(1) {
        if(getNextToken(src) == 0)  break;
        putSrc(&lastSrcPtr, src);
        numSources++;
    }

    pSrcPtr ps = &baseSrc;
    for(int i=0;i<numSources;i++) {
        printf("Src[%d]=%s\n", i, ps->ptr);
        ps = ps->next;
    }

    return r;
}

int getAllLines(pSrcPtr *ptr) {
    int r = 0;
    char src[256];

    pSrcPtr ps= *ptr;
    while(1) {
        if(getLine() == 0)  break;

        int n = strlen(cbuf);
        if(n == 0) return 0;
        if(cbuf[0]==':') return 0;

        putSrc(&ps, cbuf);
    }

    return r;
}

int getAllArguments(pSrcPtr *ptr) {
    int r = 0;
    char src[256];

    pSrcPtr ps= *ptr;
    while(1) {
        if(getNextToken(src) == 0)  break;
        putSrc(&ps, src);
    }

    return r;

}

void fprintList(pSrcPtr p) {
    while(1) {
        if(p->ptr == NULL) break;
        fprintf(out,"%s ", p->ptr);
        p = p->next;
    }
    fprintLine();
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
        getAllArguments(&lastCmplPtr);

        if( getString(":Linker:") == 0) break;
        if( getArgument(linkerStr) == 0) break;

        if( getString(":LinkerArguments:") == 0) break;
        getAllArguments(&lastLnkPtr);

        if( getString(":Includes:") == 0) break;
        getAllArguments(&lastIncPtr);

        if( getString(":Libraries:") == 0) break;
        getAllArguments(&lastLibsPtr);

        if( getString(":Environment:") == 0) {
            printf("Info: No additional environment setup\n");
        }
        else {
            getAllLines(&lastEnvPtr);
            printf("Info: Adding environment setup \n");

        }

        fprintf(out,"TARGET=%s\n", targetStr);
        fprintf(out,"BUILDDIR=%s\n", bldDirStr);
        fprintf(out,"COMPILER=%s\n", compilerStr);
        fprintf(out,"LINKER=%s\n", linkerStr);

        fprintf(out,"CMPLARGS= "); fprintList(&cmplSrc);
        fprintf(out,"INCLUDES= "); fprintList(&incSrc);
        fprintf(out,"LNKARGS= "); fprintList(&lnkSrc);
        fprintf(out,"LIBRARIES= "); fprintList(&libsSrc);



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


char* getIncludeFileFrom(FILE *fp) {

    static char buf[128];
    int ind = 0;
    char ch;


    while(1) {
        ch = fgetc(fp);
        if(feof(fp)) {
            printf("Abort: EOF while scanning\n");
            exit(0);
        }

        if( (ch == '"')) break;
        if( (ch == '<')) return NULL;
    }

    do {
        ch = fgetc(fp);
        if(feof(fp)) return 0;
        if(nonspace(ch)) break;
    } while(1);

    buf[ind++] = ch;
    do {
        ch = fgetc(fp);
        if(feof(fp)) {
            printf("Abort: EOF while scanning\n");
            exit(0);
        }
        if( (ch == '"')) break;
        if(!nonspace(ch)) continue;
        buf[ind++] = ch;

    } while(1);
    buf[ind++] = 0;
    return buf;
}


int scanDependencies(char *src) {

    printf("Scanning dependencies in %s\n", src);

    FILE *fp = fopen(src, "rt");
    if(!fp) {
        printf("Warning: cannot find %s, dependency will be skipped\n",src );
        return 1;
    }



    while(1) {
        char buf[1024];
        int n = fscanf(fp,"%s",buf);
        if( n <= 0) break;

        if(strcmp(buf,"#include") == 0) {
            char *ptr = getIncludeFileFrom(fp);
            if( ptr == NULL) continue;
            char *dirPtr = getDirectory(ptr);

            if(dirPtr==NULL) {
                dirPtr = getDirectory(src);
                sprintf(buf,"%s%s%s", dirPtr,delim,ptr);
            }
            else {
                sprintf(buf,"%s", ptr);
            }


            //dirPtr = getDirectory(src);
            //sprintf(buf,"%s%s%s", dirPtr,delim,ptr);

            if( scanDependencies(buf) == 0)
                fprintf(out," %s ",buf);
        }

    }

    fclose(fp);
    return 0;
}





int addSrcDependencyAction(char *src) {

    fprintLine();
    fprintf(out, "%s%s%s.o: %s", bldDirStr,delim,src,src);

    scanDependencies(src);

    fprintLine();
    char *ptr = getDirectory(src);
    if(ptr) fprintf(out, "\t@mkdir -p %s%s%s\n",bldDirStr,delim, ptr );
    fprintf(out, "\t$(COMPILER) $(CMPLARGS) $(INCLUDES) -c %s -o %s%s%s.o\n\n", src, bldDirStr,delim,src);

    return 1;
}



int addTargets() {

    pSrcPtr ps;

    fputs("\n", out);

    fprintf(out,".PHONY: startup %s%s%s\n\n", bldDirStr,delim,targetStr);

    fprintf(out,"%s/%s: startup %s ",bldDirStr, targetStr, bldDirStr);


    ps = &baseSrc;
    for(int i=0;i<numSources;i++) {
        char buf[256];
        sprintf(buf, "%s%s%s.o ", bldDirStr,delim,ps->ptr);
        fputs(buf,out);
        ps = ps->next;
    }

    fprintLine();




    fprintf(out, "\t$(LINKER) ");
    ps = &baseSrc;
    for(int i=0;i<numSources;i++) {
        char buf[256];
        sprintf(buf, " %s%s%s.o ", bldDirStr,delim,ps->ptr);
        ps=ps->next;
        fputs(buf,out);
    }

    fprintf(out, " -o %s%s%s $(LNKARGS) $(LIBRARIES) ",bldDirStr,delim,targetStr);

    fprintLine(); fprintLine();


    fprintf(out,"%s: \n", bldDirStr);
    const char *args[] = {"\tmkdir"," -p ", bldDirStr, NULL};
    fprintStrs(args);
    fprintLine(); fprintLine();

    fprintf(out,"startup:\n");
    ps = &envSrc;
    while(ps->ptr != NULL) {
        fprintf(out,"\t%s\n", ps->ptr);
        ps = ps->next;
    }
    fprintLine();
    fprintLine();

    ps = &baseSrc;
    for(int i=0;i<numSources;i++) {
        if( addSrcDependencyAction(ps->ptr) == 0) return 1;
        ps=ps->next;
    }

    return 0;
}

int mparser(FILE *cf, FILE *of) {

    int res=1;

    in = cf;
    out = of;

    while(1) {
        if( getPrimitives() ) break;

        if( addTargets()) break;

        fprintLine();
        fprintf(out, "clean:\n\trm -rf %s\n\n", bldDirStr);
        res = 0;
        break;
    }

    fprintLine();

    if(res) {
        printf("Error in mparser\n");
    }
    return res;
}
