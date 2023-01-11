
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <stdlib.h>
#include <math.h>

#define UART_ID         (uart0)
#define BAUD_RATE       (9600)
#define GET_INDEX       (0x7f)

void check_input_start(void);
void program_display(void);
void store_command(void);
void fetch(void);
void decode(bool* flag);

void mov();
void sub();
void mul();
void cmp();
void blt();
void end(bool* flag);
void b();
void out();


static uint sProgram_counter = 0;
static int sGeneral_register[5] = {0, 0, 0, 0, 0};
static uint sStatus_register = 0b00; // 　1bit: LESS_THAN_FLAG / 0bit: EQUAL_FLAG
struct Register
{
    uint opcode;
    uint oprand_1;
    uint oprand_2;
};
static struct Register sInstruction_register;
uint sProgram_memory[48];
// uint sProgram_memory[48] = {0b11000000, 0b10000001, 0b00000101, 0b11000000, 0b10000010, 0b00000001, 0b11000000, 0b10000011, 0b00000001, 0b11000000, 0b10000100, 0b00000001, 0b11000100, 0b10000011, 0b10000001, 0b11000110, 0b00010101, 0b11001001, 0b10000010, 0b11001000, 0b00011101, 0b11000011, 0b10000010, 0b10000001, 0b11000010, 0b10000001, 0b10000100, 0b11001000, 0b00001100, 0b11000111, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0, 0b0};


int main(){

    // UART初期化、設定
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // プログラム終了フラグ
    bool end_flag = false;

    // program_memory初期化
    
    for(int i = 0; i < 48; i++){
        sProgram_memory[i] = 0;
    }
    
    check_input_start();
    store_command();
    program_display();
    while(!end_flag){
        fetch();
        decode(&end_flag);
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

    
    char from_terminal;
    // コマンド格納 & 10進数化
    int i = 0; // <48
    int j = 0; // <2
    while(true){
        if(uart_is_readable(UART_ID)){
            if(i < 48){
                from_terminal = uart_getc(UART_ID);
                if(from_terminal == 0x0d){
                    uart_puts(UART_ID, "\n\r");
                    break;
                }else if(('0' <= from_terminal) && (from_terminal <= '9')){
                    uart_putc(UART_ID, from_terminal); // callback
                    from_terminal -= '0';
                    if(j == 0){
                        sProgram_memory[i] |= (from_terminal << 4);
                        j++;
                    }else{
                        sProgram_memory[i] |= from_terminal;
                        j--;
                        i++;
                    }
                }else if(('a' <= from_terminal) && (from_terminal <= 'f')){
                    uart_putc(UART_ID, from_terminal);
                    from_terminal = from_terminal - 'a' + 10;
                    if(j == 0){
                        sProgram_memory[i] |= (from_terminal << 4);
                        j++;
                    }else{
                        sProgram_memory[i] |= from_terminal;
                        j--;
                        i++;
                    }
                }else{
                    uart_puts(UART_ID, "Input Error\n\r");
                    break;
                }
            }else{
                break;
            }
        }
        sleep_ms(10);
    }
    uart_puts(UART_ID, "Store complete\n\r");
}

// 入力されたアセンブリ16進数を10進数に変換して表示
void program_display(void){
    printf("Program (integer) display start\n");
    for(int i=0; i < 48; i++){
        printf("%x", sProgram_memory[i]);
        printf("\n");
    }
    printf("Program (integer) display end\n");
}

void fetch(void){
    sInstruction_register.opcode = sProgram_memory[sProgram_counter];
    sInstruction_register.oprand_1 = sProgram_memory[sProgram_counter + 1];
    sInstruction_register.oprand_2 = sProgram_memory[sProgram_counter + 2];
}



 void mov(){
    printf("mov..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX] = sInstruction_register.oprand_2;
    sProgram_counter += 3;
    printf("\tregister[%d] = %d\n", (sInstruction_register.oprand_1 & GET_INDEX), sInstruction_register.oprand_2);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("mov done\n");
 }

 void sub(){
    printf("sub..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX] -= sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX];
    sProgram_counter += 3;
    printf("\tregister[%d]%d -= register[%d]%d\n", sInstruction_register.oprand_1 & GET_INDEX, sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX], sInstruction_register.oprand_2 & GET_INDEX, sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX]);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("sub done\n");
 }

 void mul(){
    printf("sub..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX] *= sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX];
    sProgram_counter += 3;
    printf("\tregister[%d]%d *= register[%d]%d\n", sInstruction_register.oprand_1 & GET_INDEX, sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX], sInstruction_register.oprand_2 & GET_INDEX, sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX]);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("sub done\n");
 }

 void cmp(){
    printf("cmp..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    if((sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX] - sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX]) < 0){
        sStatus_register = 0b10;
    }else if((sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX] - sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX]) == 0){
        sStatus_register = 0b01;
    }else{
        sStatus_register = 0b00;
    }
    sProgram_counter += 3;
    printf("\tregister[%d]%d - register[%d]%d\n", sInstruction_register.oprand_1 & GET_INDEX, sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX], sInstruction_register.oprand_2 & GET_INDEX, sGeneral_register[sInstruction_register.oprand_2 & GET_INDEX]);
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("cmp done\n");
 }

 void blt(){
    printf("blt..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    if((sStatus_register & 0b10) != 0){
        sProgram_counter = sInstruction_register.oprand_1;
    }else{
        sProgram_counter += 2;
    }
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("blt done\n");
 }

 void end(bool* flag){
    printf("end..\n");
    *flag = true;
    printf("end done\n");
 }

 void b(){
    printf("b..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    sProgram_counter = sInstruction_register.oprand_1;
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("b done\n");
 }

 void out(){
    printf("out..\n");
    printf("\tprogram_counter:%d\n", sProgram_counter);
    char ans[5]; 
    sprintf(ans, "%d", sGeneral_register[sInstruction_register.oprand_1 & GET_INDEX]);
    uart_puts(UART_ID, ans);
    uart_puts(UART_ID, "\n\r");
    sProgram_counter += 2;
    printf("\tprogram_counter:%d\n", sProgram_counter);
    printf("out done\n");
 }

 void decode(bool* flag){
    switch (sInstruction_register.opcode & 0b00111111){
        case 0:
            mov();
            break;
        case 1: //add // 未使用

            break;
        case 2:
            sub();
            break;
        case 3:
            mul();
            break;
        case 4:
            cmp();
            break;
        case 5: //bne // 未使用
            
            break;
        case 6:
            blt();
            break;
        case 7:
            end(flag);
            break;
        case 8:
            b();
            break;
        case 9:
            out();
            break;
    }
 }
 