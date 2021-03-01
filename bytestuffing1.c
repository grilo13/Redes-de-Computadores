#include <stdio.h>
#include <string.h>
#include <math.h>

void bytestuff(char *input_string, char* output_string){


    for(int i = 0, j = 0; i < strlen(input_string); i++, j++){

        if(input_string[i] == 'H' && input_string[i+1] == 'H'){

            printf("Existem dois Hs juntos");

            output_string[j] = 'E';
            output_string[j+1] = 'H';
            output_string[j+2] = 'H';

            j = j+1;
            

        } if(input_string[i] == 'E' && input_string[i+1] == 'H' && input_string[i+2] == 'H'){
            output_string[j] = 'E';
            output_string[j+1] = 'E';
            output_string[j+2] = 'E';

            j = j+1;

        } else {
            output_string[j] = input_string[i];
        }
    }

}

int main(){

    char x[8], y[8];

    strcpy(x, "aEHHHa");

    bytestuff(x,y);

    printf("\n x(input) %s\ny(output) %s\n", x,y);

    return 0;

}