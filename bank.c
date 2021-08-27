#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "account.h"
#include "string_parser.h"
#include <string.h>

struct process{
    command_line* n;
    int num;
    int length;
};

account* accounts;
pthread_mutex_t mutex;
pthread_mutex_t sum_1;
pthread_mutex_t thread_wait;
pthread_cond_t condTrans;
pthread_barrier_t barrier;


int threads_waiting = 0;
int starting = 0;

int bad = 0;


int sum(){
    pthread_mutex_lock(&sum_1);
    int total;
    for(int i = 0; i < 10; i++){
        total += accounts[i].transactions_processed;
    }
    pthread_mutex_unlock(&sum_1);
    return total;

}


void processor(int initial, double amount){
    pthread_mutex_lock(&accounts[initial].ac_lock);
    accounts[initial].transaction_tracter += amount;
    accounts[initial].transactions_processed += 1;
    pthread_mutex_unlock(&accounts[initial].ac_lock);
}

void deposit(int initial, double amount){
    //pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&accounts[initial].ac_lock);
    accounts[initial].balance += amount;
    pthread_mutex_unlock(&accounts[initial].ac_lock);
    //pthread_mutex_unlock(&mutex);
}

void withdraw(int initial, double amount){
    //pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&accounts[initial].ac_lock);
    accounts[initial].balance -= amount;
    pthread_mutex_unlock(&accounts[initial].ac_lock);
    //pthread_mutex_unlock(&mutex);
}


void* update_balance(void *args){
    while(starting != 1){
        pthread_mutex_lock(&mutex);
        int count = 0;
        while(sum() < 5000){
            pthread_cond_wait(&condTrans, &mutex);
            pthread_barrier_wait(&barrier);
        }
        for(int i = 0; i < 10; i++){
            accounts[i].balance += accounts[i].reward_rate * accounts[i].transaction_tracter;
            accounts[i].transactions_processed = 0;
            printf("%d balance: %.2lf\n", i, accounts[i].balance);
        }
        count++;
        printf("Count: %d\n", count);
        printf("Break.........................................\n");   
        pthread_barrier_wait(&barrier);
        pthread_mutex_unlock(&mutex);
    }
}



void* process_transaction(void *args){
    struct process *proc = args;
    for(int u = 0; u < proc->length; u++){
        int index;
        
        for(int i = 0; i < proc->num; i++){
            if(strcmp(accounts[i].account_number, proc->n[u].command_list[1]) == 0){
                index = i;
            }
        }

        if(strcmp(proc->n[u].command_list[0],"T") == 0){
            int dest_account = proc->num+1;
            for(int i = 0; i < proc->num; i++){
                if(strcmp(accounts[i].account_number, proc->n[u].command_list[3]) == 0){
                    dest_account = i;
                }
            }
            if(dest_account == proc->num+1){
                continue;
            }
            if(strcmp(proc->n[u].command_list[2], accounts[index].password) == 0){
                double transfer_amount;
                char *y;
                transfer_amount = strtod(proc->n[u].command_list[4], &y);
                withdraw(index, transfer_amount);
                deposit(dest_account, transfer_amount);
                processor(index, transfer_amount);
            }else{
                //printf("Incorrect Password: %s\n", proc->n.command_list[2]);
                bad += 1;
                continue;
            }
        }else if(strcmp(proc->n[u].command_list[0], "C") == 0){
            if(strcmp(accounts[index].password,proc->n[u].command_list[2]) == 0){
                bad += 1;
            }
            else{
                //printf("Incorrect Password: %s\n", proc->n.command_list[2]);
                bad += 1;
                continue;
            }
        }else if(strcmp(proc->n[u].command_list[0], "D") == 0){
            if(strcmp(accounts[index].password, proc->n[u].command_list[2]) == 0){
                double deposit_amount;
                char *r;
                deposit_amount = strtod(proc->n[u].command_list[3], &r);
                deposit(index, deposit_amount);
                processor(index, deposit_amount);
            }
            else{
                //printf("Incorrect Password: %s\n", proc->n.command_list[2]);
                bad += 1;
                continue;
            }
        }else if(strcmp(proc->n[u].command_list[0], "W") == 0){
            if(strcmp(accounts[index].password, proc->n[u].command_list[2]) == 0){
                double withdraw_amount;
                char *q;
                withdraw_amount = strtod(proc->n[u].command_list[3], &q);
                //printf("%.2lf\n", withdraw_amount);
                withdraw(index, withdraw_amount);
                processor(index, withdraw_amount);
            }else{
                //printf("Incorrect Password: %s\n", proc->n.command_list[2]);
                bad += 1;
                continue;
            }
        }else{
            printf("Transaction not supported\n");
            continue;
        }
        pthread_mutex_lock(&thread_wait);
        threads_waiting += 1;
        pthread_mutex_unlock(&thread_wait);

        if(threads_waiting == 10 && sum() >= 5000){
            pthread_cond_signal(&condTrans);
        }
        pthread_barrier_wait(&barrier);
        threads_waiting = 0;
    }
    starting = 1;
    
}




int main(int argc, char const *argv[]){

    if(argc != 2){
        printf("Usage - ./bank <input file>\n");
        return 0;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&thread_wait, NULL);
    pthread_cond_init(&condTrans, NULL);
    pthread_barrier_init(&barrier, NULL, 10);

    FILE *file = fopen(argv[1], "r");
    int count = 0;
    char* linecount = NULL;
    size_t e = 0;
    while(getline(&linecount, &e, file) != -1){
        count++;
    }
    count -= 51;
    fclose(file);


    FILE *fp = fopen(argv[1], "r");

    //Get number of account    
    size_t len = 128;
    char num[10];
    int n;
    fgets(num, 10, fp);
    n = atoi(num);

    //Initialize Accounts
    char greeting[10] = "hello\n";
    accounts = malloc(n*sizeof(account));
    for(int i = 0; i < 10; i++){
        char *line1 = NULL;
        char *line2 = NULL;
        char *line3 = NULL;
        size_t len1 = 0;
        size_t len2 = 0;
        size_t len3 = 0;
        char *line4 = NULL;
        char *line5 = NULL;
        char *p;
        char *p2;
        double balance;
        double r_rate;

        getline(&line1, &len1, fp);
        //Set Account Number
        getline(&line2, &len2, fp);
        strncpy(accounts[i].account_number, line2, 16);

        //Set Password
        getline(&line3, &len3, fp);
        strncpy(accounts[i].password, line3, 8);

        //Set Initial Balance
        getline(&line4, &len, fp);
        balance = strtod(line4, &p);
        accounts[i].balance = balance;

        //Set Reward Rate
        getline(&line5, &len, fp);
        r_rate = strtod(line5, &p2);
        accounts[i].reward_rate = r_rate;

        accounts[i].transactions_processed = 0;

        //Set tracker amount
        accounts[i].transaction_tracter = 0.0;
        pthread_mutex_init(&accounts[i].ac_lock, NULL);
    }


    pthread_t threads[10];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    struct process proc[10];

    for(int i = 0; i < 10; i++){
        char *line;
        size_t l = 0;
        
        proc[i].num = n;
        proc[i].length = count/10;
        //printf("%d\n", count/10);
        proc[i].n = malloc(proc[i].length * sizeof(command_line));
        for(int j = 0; j < proc[i].length; j++){
            getline(&line, &l, fp);
            command_line transaction = str_filler(line, " ");
            proc[i].n[j] = transaction;
        }
    }

    for(int i = 0; i < 10; i++){
        pthread_create(&threads[i], &attr, &process_transaction, &proc[i]);
    }
    pthread_t t;
    pthread_create(&t, &attr, &update_balance, NULL);

    for(int i = 0; i < 10; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_join(t, NULL);

    for(int i = 0; i < 10; i++){
        free(proc[i].n);
    }
    
    for(int i = 0; i < 10; i++){
        pthread_mutex_destroy(&accounts[i].ac_lock);
    }
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&thread_wait);

    pthread_cond_destroy(&condTrans);


    
   
   
       
    /*
    for(int i = 0; i < 10; i++){
        accounts[i].balance += accounts[i].reward_rate * accounts[i].transaction_tracter;
        printf("%d balance: %.2lf\n", i, accounts[i].balance);
    }   
    */

    //free(accounts);
    int total = 0;

    for(int i = 0; i < 10; i++){
        total += accounts[i].transactions_processed;
    }

    printf("Transactions Processed: %d\n", total);
    printf("Other: %d\n", bad);

    fclose(fp);
    return 1;
}