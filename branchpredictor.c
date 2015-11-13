#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<assert.h>
#include<ctype.h>

#define MAX 3
#define MIN 0 //saturating values of 2 bit counter

int DEBUG = 1;

int main(int argc, char *argv[])
{
        if(argc <= 3)
        {
                printf("Usage: ./sim bimodal <M2> <trace file>\n");
                printf("Usage: ./sim gshare <M1> <N> <Trace file>\n");
                printf("Usage: ./sim hybrid <K> <M1> <N> <M2> <Trace file>\n");
                exit(-1);
        }

        FILE *trace = fopen(argv[3],"r");
        if(trace == NULL)
        {
                printf("FATAL: Could not open argv[3]\n");
                exit(-1);
        }

        unsigned short bimodal_prediction_table[262143];
        unsigned short gshare_prediction_table[262143];
        unsigned long long global_branch_history;
        unsigned short hybrid_chooser_table[262143];
        char str[12];

        unsigned long long num_accesses = 0, branch_mispredicts = 0;
        double mispredict_rate;
        
        if(!strcmp(argv[1],"bimodal"))
        {
            unsigned long long i, num_addr, branch_addr, address_bits = atoi(argv[2]);
            char t;
            
            //Initialize prediction counters
            for(i=0;i<pow(2,address_bits);i++)
                bimodal_prediction_table[i] = 2;

            while(fgets(str,12,trace) != NULL)
            {
                num_accesses++;

                for(i=0;i<strlen(str);i++)
                   if(str[i] == '\n')
                      str[i] = '\0';
               
                char addr[8];
                for(i=0;i<strlen(str);i++)
                {
                    if(str[1] != ' ')
                        addr[i] = str[i];
                    else
                    {
                        addr[i] = '\0';
                        break;
                    }
                }
                
                for(i=0;i<strlen(str);i++)
                {
                    if(str[i] == 't')
                        t = 1;
                    else
                        t = 0;
                }

                branch_addr = 0;
                num_addr = (unsigned long long)strtoul(addr, NULL, 16);
                num_addr = num_addr>>2;

                for(i=0;i<address_bits;i++)
                    branch_addr = branch_addr + (num_addr&(1<<i));

                //Mis-Prediction Count
                if(bimodal_prediction_table[branch_addr] > 1 && t == 0)
                    branch_mispredicts++;
                else if(bimodal_prediction_table[branch_addr] < 2 && t == 1)
                    branch_mispredicts++;

                //Update Prediction Table
                if(t == 1)
                {
                   if(bimodal_prediction_table[branch_addr] != 3)
                       bimodal_prediction_table[branch_addr]++;
                }
                else if (t == 0)
                {
                   if(bimodal_prediction_table[branch_addr] != 0)
                       bimodal_prediction_table[branch_addr]--;
                }
            }

            //Output
            printf("COMMAND\n");
            printf(" ./sim bimodal %d %s\n",address_bits,argv[3]);
            printf("OUTPUT\n");
            printf(" number of predictions:    %llu\n",num_accesses);
            printf(" number of mispredictions: %llu\n",branch_mispredicts);
            printf(" misprediction rate:       %0.2f%\n",(double)100*branch_mispredicts/num_accesses);
            printf("FINAL BIMODAL CONTENTS\n");
            
            for(i=0;i<pow(2,address_bits);i++)
                printf(" %d  %d\n",i,bimodal_prediction_table[i]);
        }
        else if (!strcmp(argv[1],"gshare"))
        {
        }
        else if(!strcmp(argv[1],"hybrid"))
        {
        }
     
    return 0;
}

