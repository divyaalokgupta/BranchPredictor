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


        unsigned short bimodal_prediction_table[262143];
        unsigned short gshare_prediction_table[262143];
        unsigned long long bhr;
        unsigned short hybrid_prediction_table[262143];
        char str[12];

        unsigned long long num_accesses = 0, branch_mispredicts = 0;
        double mispredict_rate;
        
        if(!strcmp(argv[1],"bimodal"))
        {
            FILE *trace = fopen(argv[3],"r");
            if(trace == NULL)
            {
                    printf("FATAL: Could not open argv[3]\n");
                    exit(-1);
            }
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
            fclose(trace);
        }
        else if (!strcmp(argv[1],"gshare"))
        {
            FILE *trace = fopen(argv[4],"r");
            if(trace == NULL)
            {
                    printf("FATAL: Could not open argv[4]\n");
                    exit(-1);
            }
            unsigned long long i, num_addr, branch_addr, address_bits = atoi(argv[2]), bhr_bits = atoi(argv[3]);
            bhr = 0;
            char t;
            
            //Initialize prediction counters
            for(i=0;i<pow(2,address_bits);i++)
                gshare_prediction_table[i] = 2;

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

                //XOR operation for branch address
                branch_addr = branch_addr ^ (bhr << (address_bits - bhr_bits));    

                //Mis-Prediction Count
                if(gshare_prediction_table[branch_addr] > 1 && t == 0)
                    branch_mispredicts++;
                else if(gshare_prediction_table[branch_addr] < 2 && t == 1)
                    branch_mispredicts++;

//               printf("GP: %llu\n",branch_addr);
                //Update Prediction Table  & Branch History Register
                if(t == 1)
                {
                   if(gshare_prediction_table[branch_addr] != 3)
                       gshare_prediction_table[branch_addr]++;
//                   printf("Old BHR: %d\n",bhr);
                    bhr = bhr >> 1;
                    bhr = bhr | (1<<(bhr_bits-1));
//                   printf("New BHR: %d\n",bhr);
                }
                else if (t == 0)
                {
                   if(gshare_prediction_table[branch_addr] != 0)
                       gshare_prediction_table[branch_addr]--;
//                   printf("Old BHR: %d\n",bhr);
                    bhr = bhr >> 1;
                    bhr = bhr & (~(1<<bhr_bits));
//                   printf("New BHR: %d\n",bhr);
                }
            }

            //Output
            printf("COMMAND\n");
            printf(" ./sim gshare %d %d %s\n",address_bits,bhr_bits,argv[4]);
            printf("OUTPUT\n");
            printf(" number of predictions:    %llu\n",num_accesses);
            printf(" number of mispredictions: %llu\n",branch_mispredicts);
            printf(" misprediction rate:       %0.2f%\n",(double)100*branch_mispredicts/num_accesses);
            printf("FINAL GSHARE CONTENTS\n");
            
            for(i=0;i<pow(2,address_bits);i++)
                printf(" %d  %d\n",i,gshare_prediction_table[i]);
            fclose(trace);
        }
        else if(!strcmp(argv[1],"hybrid"))
        {
            FILE *trace = fopen(argv[6],"r");
            if(trace == NULL)
            {
                    printf("FATAL: Could not open argv[6]\n");
                    exit(-1);
            }
            unsigned long long i, num_addr, bimodal_branch_addr, gshare_branch_addr, chooser_branch_addr, bimodal_address_bits = atoi(argv[5]), bhr_bits = atoi(argv[4]),gshare_address_bits = atoi(argv[3]), chooser_address_bits = atoi(argv[2]);
            bhr = 0;
            char t;
            
            //Initialize prediction counters
            for(i=0;i<pow(2,bimodal_address_bits);i++)
                bimodal_prediction_table[i] = 2;
            
            for(i=0;i<pow(2,gshare_address_bits);i++)
                gshare_prediction_table[i] = 2;

            for(i=0;i<pow(2,chooser_address_bits);i++)
                hybrid_prediction_table[i] = 1;

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

                bimodal_branch_addr = 0;
                num_addr = (unsigned long long)strtoul(addr, NULL, 16);
                num_addr = num_addr>>2;

                for(i=0;i<bimodal_address_bits;i++)
                    bimodal_branch_addr = bimodal_branch_addr + (num_addr&(1<<i));

                //XOR operation for Gshare branch address
                gshare_branch_addr = 0;
                for(i=0;i<gshare_address_bits;i++)
                    gshare_branch_addr = gshare_branch_addr + (num_addr&(1<<i));
                gshare_branch_addr = gshare_branch_addr ^ (bhr << (gshare_address_bits - bhr_bits));    
//                printf("GP: %d\n",gshare_branch_addr);
//                printf("BP: %d\n",bimodal_branch_addr);

                //Chooser branch address
                chooser_branch_addr = 0;
                for(i=0;i<chooser_address_bits;i++)
                    chooser_branch_addr = chooser_branch_addr + (num_addr&(1<<i));
//                printf("CP: %d\n",chooser_branch_addr);

                //Mis-Prediction Count
                if(hybrid_prediction_table[chooser_branch_addr] > 1)
                {
                    if(gshare_prediction_table[gshare_branch_addr] > 1 && t == 0)
                        branch_mispredicts++;
                    else if(gshare_prediction_table[gshare_branch_addr] < 2 && t == 1)
                        branch_mispredicts++;
                }
                else if (hybrid_prediction_table[chooser_branch_addr] < 2)
                {
                    if(bimodal_prediction_table[bimodal_branch_addr] > 1 && t == 0)
                        branch_mispredicts++;
                    else if(bimodal_prediction_table[bimodal_branch_addr] < 2 && t == 1)
                        branch_mispredicts++;
                }

                //Update Hybrid Prediction Table


                //Update Prediction Tables 
                if(hybrid_prediction_table[chooser_branch_addr] < 2)
                {
                    if(t == 1)
                    {
                        if(gshare_prediction_table[gshare_branch_addr] < 2 && bimodal_prediction_table[bimodal_branch_addr] > 1 && hybrid_prediction_table[chooser_branch_addr] != 0)
                            hybrid_prediction_table[chooser_branch_addr]--;
                        else if(gshare_prediction_table[gshare_branch_addr] > 1 && bimodal_prediction_table[bimodal_branch_addr] < 2 && hybrid_prediction_table[chooser_branch_addr] != 3 )
                            hybrid_prediction_table[chooser_branch_addr]++;
                            
                        if(bimodal_prediction_table[bimodal_branch_addr] != 3)
                            bimodal_prediction_table[bimodal_branch_addr]++;

                        bhr = bhr >> 1;
                        bhr = bhr | (1<<(bhr_bits-1));
                    }
                    else if (t == 0) 
                    {
                        if(gshare_prediction_table[gshare_branch_addr] < 2 && bimodal_prediction_table[bimodal_branch_addr] > 1 && hybrid_prediction_table[chooser_branch_addr] != 3 )
                            hybrid_prediction_table[chooser_branch_addr]++;
                        else if(gshare_prediction_table[gshare_branch_addr] > 1 && bimodal_prediction_table[bimodal_branch_addr] < 2 && hybrid_prediction_table[chooser_branch_addr] != 0 )
                            hybrid_prediction_table[chooser_branch_addr]--;
                        
                        if(bimodal_prediction_table[bimodal_branch_addr] != 0)
                            bimodal_prediction_table[bimodal_branch_addr]--;

                        bhr = bhr >> 1;
                        bhr = bhr & (~(1<<(bhr_bits-1)));
                    }
                }
                else if(hybrid_prediction_table[chooser_branch_addr] > 1)
                {
                    if(t == 1)
                    {
                        if(gshare_prediction_table[gshare_branch_addr] < 2 && bimodal_prediction_table[bimodal_branch_addr] > 1 && hybrid_prediction_table[chooser_branch_addr] != 0)
                            hybrid_prediction_table[chooser_branch_addr]--;
                        else if(gshare_prediction_table[gshare_branch_addr] > 1 && bimodal_prediction_table[bimodal_branch_addr] < 2 && hybrid_prediction_table[chooser_branch_addr] != 3 )
                            hybrid_prediction_table[chooser_branch_addr]++;
                        
                        if(gshare_prediction_table[gshare_branch_addr] != 3)
                            gshare_prediction_table[gshare_branch_addr]++;

                        bhr = bhr >> 1;
                        bhr = bhr | (1<<(bhr_bits-1));
                    }
                    else if (t == 0)
                    {
                        if(gshare_prediction_table[gshare_branch_addr] < 2 && bimodal_prediction_table[bimodal_branch_addr] > 1 && hybrid_prediction_table[chooser_branch_addr] != 3 )
                            hybrid_prediction_table[chooser_branch_addr]++;
                        else if(gshare_prediction_table[gshare_branch_addr] > 1 && bimodal_prediction_table[bimodal_branch_addr] < 2 && hybrid_prediction_table[chooser_branch_addr] != 0 )
                            hybrid_prediction_table[chooser_branch_addr]--;
                        
                        if(gshare_prediction_table[gshare_branch_addr] != 0)
                            gshare_prediction_table[gshare_branch_addr]--;
                            
                        bhr = bhr >> 1;
                        bhr = bhr & (~(1<<(bhr_bits-1)));
                    }
                }


            }

            printf("COMMAND\n");
            printf(" ./sim hybrid %d %d %d %d %s\n",chooser_address_bits,gshare_address_bits,bhr_bits,bimodal_address_bits,argv[6]);
            printf("OUTPUT\n");
            printf(" number of predictions:    %llu\n",num_accesses);
            printf(" number of mispredictions: %llu\n",branch_mispredicts);
            printf(" misprediction rate:       %0.2f%\n",(double)100*branch_mispredicts/num_accesses);
            printf("FINAL CHOOSER CONTENTS\n");
            for(i=0;i<pow(2,chooser_address_bits);i++)
                printf(" %d  %d\n",i,hybrid_prediction_table[i]);
            printf("FINAL GSHARE CONTENTS\n");
            for(i=0;i<pow(2,gshare_address_bits);i++)
                printf(" %d  %d\n",i,gshare_prediction_table[i]);
            printf("FINAL BIMODAL CONTENTS\n");
            for(i=0;i<pow(2,bimodal_address_bits);i++)
                printf(" %d  %d\n",i,bimodal_prediction_table[i]);
            fclose(trace);
        }

    return 0;
}

