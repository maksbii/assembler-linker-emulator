#include <stdio.h>

extern int yyparse();
extern FILE* yyin;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Upotreba: ./asembler <ulaz.s>\n");
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        printf("Greska: ne mogu da otvorim %s\n", argv[1]);
        return 1;
    }

    printf("Pocinje parsiranje...\n");
    yyparse();
    printf("Parsiranje zavrseno!\n");

    fclose(yyin);
    return 0;
}