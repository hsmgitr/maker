#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mparser.h"

const char *defFile = "Target: \nBuildDirectory: \nCompiler: \nCompilerArguments: \nIncludes: \nLinker: \nLinkerArguments: \nSources: \n";

int main(int argc, char *argv[]) {

    if(argc < 2) {
        printf("usage: maker maker-input-file\n");
        FILE *f = fopen("sample.template","wt");
        if(f) {
            fprintf(f,"%s",defFile);
            printf("A sample template is generated as sample.template\n");
            fclose(f);
        }
        return 1;
    }

    FILE *cf = fopen(argv[1],"rt");
    if(!cf) {
        printf("Cannot find %s\n",argv[1]);
        return 1;
    }

    static char buf[128];
    sprintf(buf,"%s.mak",argv[1]);

    FILE *of = fopen(buf,"wt");
    if(!of) {
        printf("Cannot find %s\n",buf);
        return 1;
    }

    fprintf(of,"\n\n#Make file %s generated by maker\n#\n#\n", buf);
    if( mparser(cf, of) == 0) {
        printf("Make file %s succcessfully generated\n", buf);
    }
    else {
        char bufbuf[150];
        sprintf(bufbuf, "rm %s", buf);
        system(bufbuf);
        printf("Make file is %s not generated\n", buf);
    }
}