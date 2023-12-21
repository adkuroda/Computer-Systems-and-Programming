#include <stdio.h>
#include <stdlib.h>

int main(){
    int i = 0;
    float x = 0.0001;
    for (i = 0; i < 5; i ++){
        x = x/2;
        printf("%d %.100f\n",i, x);
    }
    return 1;
}