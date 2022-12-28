
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <stdlib.h>
#include <math.h>

#define UART_ID         (uart0)
#define BAUD_RATE       (9600)
#define BOARD_LED       (25)
#define EQUAL_TO_FLAG   (0)
#define LESS_THAN_FLAG  (1)

void check_input_start(void);
void program_display(void);
void store_command(void);
void fetch(void);
void decode(int* index_1, int* index_2, int* store, bool* flag);

void mov(int* index, int* store);
void sub(int* index_1, int* index_2);
void mul(int* index_1, int* index_2);
void cmp(int* index_1, int* index_2);
void blt(int* store);
void end(bool* flag);
void b(int* store);
void out(int* index_1);

void get_register_index(int* index, int n);
void get_imediate(int* store_itob, int n);

static int sProgram_counter = 0;
static int sGeneral_register[5] = {0, 0, 0, 0, 0};
static int sStatus_register[2] = {0, 0};
struct Register
{
    int opcode[2];
    int oprand_1[2];
    int oprand_2[2];
};
static struct Register sInstruction_register;
int sProgram_memory[48][2] = {{12, 0}, {8, 1}, {0, 5}, {12, 0}, {8, 2}, {0, 1}, {12, 0}, {8, 3}, {0, 1}, {12, 0}, {8, 4}, {0, 1}, {12, 4}, {8, 3}, {8, 1}, {12, 6}, {1, 5}, {12, 9}, {8, 2}, {12, 8}, {1, 13}, {12, 3}, {8, 2}, {8, 1}, {12, 2}, {8, 1}, {8, 4}, {12, 8}, {0, 12}, {12, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};


int main(){

    // UART初期化、設定
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    
    // GPIO初期化、設定
    gpio_init(BOARD_LED);
    gpio_set_dir(BOARD_LED, GPIO_OUT);

    int register_index_1 = 0;
    int register_index_2 = 0;

    // プログラム終了フラグ
    bool end_flag = false;

    // 2進数格納用
    int store_itob = 0;

    // program_memory初期化
    /*入力自動化につきコメントアウト
    for(int i = 0; i < 48; i++){
        for(int j = 0; j < 2; j++){
            program_memory[i][j] = 0;
        }
    }
    */
    check_input_start();
    store_command();
    program_display();
    while(!end_flag){
        fetch();
        decode(&register_index_1, &register_index_2, &store_itob, &end_flag);
    }
}



// 's'が入力されたら先に進む
void check_input_start(void){
    char from_terminal;
    uart_puts(UART_ID, "Type 's' to start\n\r");
    while(true){
        if(uart_is_readable(UART_ID)){
            from_terminal = uart_getc(UART_ID);
            if(from_terminal == 's'){
                uart_puts(UART_ID, "Input assembly command\n\r");
                break;
            }else{
                continue;
            }
        }
    }
}

// 16進数のアセンブリ後を入力するモード。enterで終了。
void store_command(void){

    /*入力自動化
    char from_terminal;
    // コマンド格納 & 10進数化
    int i = 0; // <48
    int j = 0; // <2
    while(true){
        if(uart_is_readable(UART_ID)){
            if(i < 48){
                if(j < 2){
                    from_terminal = uart_getc(UART_ID);
                    if(from_terminal == 0x0d){
                        uart_puts(UART_ID, "\n\r");
                        break;
                    }else if(('0' <= from_terminal) && (from_terminal <= '9')){
                        uart_putc(UART_ID, from_terminal); // callback
                        from_terminal -= '0';
                        sProgram_memory[i][j] = from_terminal;
                        j++;
                    }else if(('a' <= from_terminal) && (from_terminal <= 'f')){
                        uart_putc(UART_ID, from_terminal);
                        from_terminal = from_terminal - 'a' + 10;
                        sProgram_memory[i][j] = from_terminal;
                        j++;
                    }
                }else{
                    i++;
                    j = 0;
                }
            }else{
                break;
            }
        }
        sleep_ms(10);
    }
    入力自動化*/
    uart_puts(UART_ID, "Store complete\n\r");
}

// 入力されたアセンブリ16進数を10進数に変換して表示
void program_display(void){
    printf("Program (integer) display start\n");
    for(int i=0; i < 48; i++){
        for(int j = 0; j < 2; j++){
            printf("%d", sProgram_memory[i][j]);
            printf("\t");
            if(j == 1){
                printf("\n\r");
            }
        }
    }
    printf("Program (integer) display end\n");
}

void fetch(void){
    for(int i = 0; i < 2; i++){
        sInstruction_register.opcode[i] = sProgram_memory[sProgram_counter][i];
    }
    for(int i = 0; i < 2; i++){
        sInstruction_register.oprand_1[i] = sProgram_memory[sProgram_counter + 1][i];
    }
    for(int i = 0; i < 2; i++){
        sInstruction_register.oprand_2[i] = sProgram_memory[sProgram_counter + 2][i];
    }

}

void get_register_index(int* index, int n){
    int i, j;
    if(n == 1){
        i = sInstruction_register.oprand_1[0];
        j = sInstruction_register.oprand_1[1];
    }else if(n == 2){
        i = sInstruction_register.oprand_2[0];
        j = sInstruction_register.oprand_2[1];
    }
    int binary[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int binary_index = 4;
    while(i != 0){
        binary[binary_index] = i % 2;
        i = i / 2;
        binary_index++;
    }
    binary_index = 0;
    while(j != 0){
        binary[binary_index] = j % 2;
        j = j / 2;
        binary_index++;
    }
    binary[7] = 0; //レジスタフラグ削除
    // レジスタのアドレス値10進数化
    for(int p = 0; p < 8; p++){
        *index += (binary[p] * pow(2, p));
    }
}

 void get_imediate(int* store_itob, int n){
    int i, j;
    if(n == 1){
        i = sInstruction_register.oprand_1[0];
        j = sInstruction_register.oprand_1[1];
    }else if(n == 2){
        i = sInstruction_register.oprand_2[0];
        j = sInstruction_register.oprand_2[1];
    }
    int binary[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int binary_index = 4;
    while(i != 0){
        binary[binary_index] = i % 2;
        i = i / 2;
        binary_index++;
    }
    binary_index = 0;
    while(j != 0){
        binary[binary_index] = j % 2;
        j = j / 2;
        binary_index++;
    }
    // imediate値の10進数化
    for(int p = 0; p < 8; p++){
        *store_itob += (binary[p] * pow(2, p));
    }
 }

 void mov(int* index, int* store){
    // 必要であればindex、storeはここで宣言
    printf("mov..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    // 第一引数の2進数化
    get_register_index(index, 1);  //ここにオペランドを入れる。そうすれば第二引数の1はいらない。returnで返してもいい。
    // 第二引数の2進数化
    get_imediate(store, 2);
    sGeneral_register[*index] = *store;
    sProgram_counter += 3;
    printf("\tregister[%d] = %d\n", *index, *store);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("mov done\n");
    *store = 0;
    *index = 0;
 }

 void sub(int* index_1, int* index_2){
    printf("sub..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    // 第一引数の2進数化
    get_register_index(index_1, 1);
    // 第二引数の2進数化
    get_register_index(index_2, 2);
    sGeneral_register[*index_1] -= sGeneral_register[*index_2];
    sProgram_counter += 3;
    printf("\tregister[%d]%d -= register[%d]%d\n", *index_1, sGeneral_register[*index_1], *index_2, sGeneral_register[*index_2]);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("sub done\n");
    *index_1 = 0;
    *index_2 = 0;
 }

 void mul(int* index_1, int* index_2){
    printf("sub..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    // 第一引数の2進数化
    get_register_index(index_1, 1);
    // 第二引数の2進数化
    get_register_index(index_2, 2);
    sGeneral_register[*index_1] *= sGeneral_register[*index_2];
    sProgram_counter += 3;
    printf("\tregister[%d]%d *= register[%d]%d\n", *index_1, sGeneral_register[*index_1], *index_2, sGeneral_register[*index_2]);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("sub done\n");
    *index_1 = 0;
    *index_2 = 0;
 }

 void cmp(int* index_1, int* index_2){
    printf("cmp..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    // 第一引数の2進数化
    get_register_index(index_1, 1);
    // 第二引数の2進数化
    get_register_index(index_2, 2);
    if((sGeneral_register[*index_1] - sGeneral_register[*index_2]) < 0){
        sStatus_register[EQUAL_TO_FLAG] = 0;
        sStatus_register[LESS_THAN_FLAG] = 1;
    }else if((sGeneral_register[*index_1] - sGeneral_register[*index_2]) == 0){
        sStatus_register[EQUAL_TO_FLAG] = 1;
        sStatus_register[LESS_THAN_FLAG] = 0;
    }else{
        sStatus_register[EQUAL_TO_FLAG] = 0;
        sStatus_register[LESS_THAN_FLAG] = 0;
    }
    sProgram_counter += 3;
    printf("\tregister[%d]%d - register[%d]%d\n", *index_1, sGeneral_register[*index_1], *index_2, sGeneral_register[*index_2]);
    printf("\tstatus_register%d%d\n", sStatus_register[EQUAL_TO_FLAG], sStatus_register[LESS_THAN_FLAG]);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("cmp done\n");
    *index_1 = 0;
    *index_2 = 0;
 }

 void blt(int* store){
    printf("blt..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    get_imediate(store, 1);
    if(sStatus_register[LESS_THAN_FLAG] == 1){
        sProgram_counter = *store;
    }else{
        sProgram_counter += 2;
    }
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("blt done\n");
    *store = 0;
 }

 void end(bool* flag){
    printf("end..\n");
    *flag = true;
    printf("end done\n");
 }

 void b(int* store){
    printf("b..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    get_imediate(store, 1);
    sProgram_counter = *store;
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("b done\n");
    *store = 0;
 }

 void out(int* index_1){
    printf("out..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    get_register_index(index_1, 1);
    char ans[5]; 
    sprintf(ans, "%d", sGeneral_register[*index_1]);
    uart_puts(UART_ID, ans);
    uart_puts(UART_ID, "\n\r");
    sProgram_counter += 2;
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("out done\n");
    *index_1 = 0;
 }

 void decode(int* index_1, int* index_2, int* store, bool* flag){
    switch (sInstruction_register.opcode[1]){
        case 0:
            mov(index_1, store);
            break;
        case 1: //add // 未使用

            break;
        case 2:
            sub(index_1, index_2);
            break;
        case 3:
            mul(index_1, index_2);
            break;
        case 4:
            cmp(index_1, index_2);
            break;
        case 5: //bne // 未使用
            
            break;
        case 6:
            blt(store);
            break;
        case 7:
            end(flag);
            break;
        case 8:
            b(store);
            break;
        case 9:
            out(index_1);
            break;
    }
 }
 