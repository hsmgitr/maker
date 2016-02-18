
#include <stdlib.h>
#include <string.h>

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

int getArgument(char *buf) {
    int n = fscanf(in,"%s", buf);
    if(n<=0) return 0;
    return 1;
}

char targetStr[64];
char bldDirStr[1024];
char compilerStr[1024];
char compilerArgsStr[2048];
char linkerStr[1024];
char linkerArgsStr[2048];
char includesStr[2048];



int getPrimitives() {

    int r = 1;
    while(1) {
        if( getString("Target:") == 0) break;
        if( getArgument(targetStr) == 0) break;
        if( getString("BuildDirectory:") == 0) break;
        if( getArgument(bldDirStr) == 0) break;
        if( getString("Compiler:") == 0) break;
        if( getArgument(compilerStr) == 0) break;
        if( getString("CompilerArguments:") == 0) break;
        if( getArgument(compilerArgsStr) == 0) break;
        if( getString("Linker:") == 0) break;
        if( getArgument(linkerStr) == 0) break;
        if( getString("LinkerArguments:") == 0) break;
        if( getArgument(linkerArgsStr) == 0) break;
        if( getString("Includes:") == 0) break;
        if( getArgument(includesStr) == 0) break;

        fprintf(out,"TARGET\t\t\t=%s\n", targetStr);
        fprintf(out,"BUILDDIR\t\t\t=%s\n", bldDirStr);
        fprintf(out,"COMPILER\t\t\t=%s\n", compilerStr);
        fprintf(out,"CMPLARGS\t\t\t=%s\n", compilerArgsStr);
        fprintf(out,"INCLUDES\t\t\t=%s\n", includesStr);
        fprintf(out,"LINKER\t\t\t=%s\n", linkerStr);
        fprintf(out,"LNKARGS\t\t\t=%s\n", linkerArgsStr);


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
