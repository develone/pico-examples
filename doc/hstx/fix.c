#include <stdio.h>
#include <stdlib.h>

int main(void) 
{
 		
		int i, j=250000;
		char buf[80];
		FILE *file_pointer_rd = fopen("dvi-1.csv","r");
    if (file_pointer_rd == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
		}
		FILE *file_pointer_wr = fopen("test.csv","w");
    if (file_pointer_wr == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
		}
		for(i=0;i<250000;i++)
		{
				fgets(buf, sizeof(buf),file_pointer_rd);
				 
				 
				printf("%d ",i);
				printf("%s\n",buf);
		}
		
		return 1;
}
