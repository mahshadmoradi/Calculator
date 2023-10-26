#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/lcd.h"
#include "stdio.h"
///////////////////////////////////////////////////////////////////////////
int index = 0, inputIndex=-1;
int r = 0;
bool error=0, lock=0;

char input[31]; //chars displayed 
int paranthesisNum;

void clr();
void errorr();
float calculate(float finalResult,int op);
float chartonum(float f,int point);
void display(int n);

#define menumode 0
#define calculatormode 1
#define displaymode 2

/////////////////////////////////////////////////////////////////////////// 
int main(void)
{
    int Mode = menumode;
    // initialize the device
    // Use ctrl+click to see inside of the functions
    SYSTEM_Initialize();
    LCD_Initialize();
    // remember to clear lcd before first usage
    clr();
    inputIndex++;
    LCDGoto(0,0);
    LCDPutStr("Select Mode 1||2"); 
    int i=0,j=0;
    int pressed=0;

while (true)
    {
        //reading the keyboard :
        LATA = 1;
        while(true){
            LATA = 1 << i;
            for(j = 0;j < 4 ; j++){ //scanning B12 to B15 :
                if ((PORTB & 0xf000) == 0x1000 << j){
                pressed = 1;
                break;
              }
            }
            if(pressed){
                pressed = 0;
                break;
            }
            i++;
            i = i%5;
        } //the keyboard is fully scanned 
        
        //mode is selected :
        switch(Mode){
            case menumode:
                if(j==0){
                    if(i==0){ 
                        Mode = calculatormode;
                       LCDGoto(0,0);
                       LCDPutStr("calculator mode "); 
                        __delay_ms(200);
                    }else if(i==1){ //Mode2
                        Mode = displaymode;
                       LCDGoto(0,0);
                       LCDPutStr("display mode    "); 
                        __delay_ms(100);
                    }
                }
                break;
            default:    
            if(!lock || (i==3 && j==3) ){ //screen is not locked or is locked and is to get cleared 
                LCDPutCmd(LCD_HOME);

                switch (i){ //scanning the pressed button : 
                case 0: //column 1 
                    switch (j){
                        case 3:
                            input[inputIndex]='('; 
                            break;
                        default:
                        input[inputIndex]='0'+3*j+i+1;
                        display(inputIndex);            
                    }            
                    break; 
                case 1: //column 2
                    switch (j){
                        case 3:
                        input[inputIndex]='0'; break;
                        default:  
                        input[inputIndex]='0'+3*j+i+1;
                        display(inputIndex);             
                    }            
                    break;
                case 2: //column 3
                    switch (j){
                        case 3:
                        input[inputIndex]=')'; break;
                        default:  
                        input[inputIndex]='0'+3*j+i+1;
                        display(inputIndex);           
                    }            
                    break; 
                case 3: //column 4
                    switch (j){
                        case 0:
                        input[inputIndex]='+';
                        break;
                        case 1:
                        input[inputIndex]='*';
                        break; 
                        case 2:
                        input[inputIndex]='.';
                        break; 
                        case 3:
                            (Mode==displaymode)? input[inputIndex]='a': clr(); //clearing in calculate mode 
                        break;
                    }
                    break;
                case 4:
                    switch (j){
                        case 0:
                            input[inputIndex]='-';  
                        break;
                        case 1:
                            input[inputIndex]='/';  
                        break; 
                        case 2: //delete 
                            inputIndex-=2;
                            if(inputIndex<-1)
                                inputIndex=-1;
                            input[inputIndex+1]=' ';
                        break;

                        case 3: //equals 
                            if(Mode==calculatormode){
                            lock = 1;
                            input[inputIndex]='=';
                            display(inputIndex+1);
                            index = 0;
                            r = 0;
                            paranthesisNum=0;  
                            calculate(1,0);
                            display(inputIndex+2);
                            }
                            else 
                                input[inputIndex]='=';    
                            
                        break;
                    }            
                    break;                 
                }
            }
            //display on the screen 
            display(inputIndex);
            inputIndex++;
            __delay_ms(50);
            break;
        }
    }
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void clr(){
    lock=0;
    error=0;
    index = 0;
    r = 0;  
    inputIndex=-1;
    for(int k = 0; k<31; k++)
        input[k]=' ';
    for(int clr=5; clr>=0; clr--){
    LCDClear();
    LCDGoto(clr,0);
    }

}
void errorr(){
    error=1;
    LCDGoto(0,1);
    LCDPutStr("Error"); 
}
void display(int n){ 
    n-=15;
    if(n<0)
        n=0;
    for(int k=0; k<=15; k++){
        LCDGoto(k,0);
        LCDPutChar(input[k+n]); 
    }
}
float calculate(float finalResult,int op){ //op::  0:none, 1:add, 2:sub, 3:mult, 4:div
    r++;
    float result = 0;
    index++;
    while(input[index-1] != '='){
        switch(input[index-1])
        {
            case '(':
                paranthesisNum++;
                if(op == 1)
                    result =  finalResult + calculate(1,0);
                else if(op == 2)
                    result =  finalResult - calculate(1,0);
                else if(op == 3)
                    result =  finalResult * calculate(1,0);
                else if(op == 4){
                    float d =  calculate(1, 0);
                    if(d == 0){
                        errorr();
                    }
                    result =  finalResult / d;
                }
                else if(input[index-2]>=48){
                    result =  result * calculate( 1, 0);
                }
                else 
                    result = calculate( 1, 0);
                break;

            case ')': //continuing the recursion in calculateulation 
                paranthesisNum--;
                index++;
                if(input[index-1] == '*'){
                    r--;
                    return calculate(result,3);
                }else if(input[index-1] == '/'){
                    r--;
                    return calculate(result,4);
                }else
                    r--;
                    return result;
                //returning of func is done 
            case '+':
                result = calculate(result,1);
                break;
            case '-':
                result = calculate(result,2);
                break;
            case '*':
                result = calculate(result,3);
                break;
            case '/':
                result = calculate(result,4);
            case '=':
                break;
                
            default:
                index--;
                result = chartonum(0,0);
                if(op!=0) r--;
                if(op == 3){
                    if(input[index-1] == '*')
                        return calculate( finalResult * result,3);
                    else if(input[index-1] == '/')
                        return calculate( finalResult * result,4);
                    else
                        return result * finalResult;
                } else if(op == 4){
                    if(result == 0){
                        errorr(); // zero division
                    }
                    if(input[index-1] == '*')
                        return calculate( finalResult / result,3);
                    else if(input[index-1] == '/')
                        return calculate( finalResult / result,4);
                    else
                        return finalResult / result;
                } else if(op == 1){

                    if(input[index-1] == '*')
                        return finalResult + calculate(result,3);
                    else if(input[index-1] == '/')
                        return finalResult + calculate(result,4);
                    else
                        return finalResult + result;
                } else if(op == 2){
                    if(input[index-1] == '*')
                        return finalResult - calculate(result,3);
                    else if(input[index-1] == '/')
                        return finalResult - calculate(result,4);
                    else
                        return finalResult - result;
                }
        }

    }

    if(r == 1 ){

        if(paranthesisNum!=0) { errorr(); return 0;}

        if(!error){
        //handling float numbers :
        char number[16] = "0000000000000000";
        float f = result;
        int point = 7;
        LCDGoto(0,1);
        //for negative numbers 
        if(f<0){
            LCDPutChar('-');
            f*=-1;
        }else LCDPutChar(' ');
        
        int intf = (int) f;
        int ff = (int)(((int)(f*100000))-(int)(intf * 100000));
        int ii = 0;
        while(intf>=1){
            number[16-ii-point] = intf%10 + '0';
            intf/=10;
            ii++;
        }
        number[17-point] = '.';
        int ff2 = ff;
        ii=5;
        while(ff>=1 && ii >=0){
            number[10+ii] = ff%10 + '0';
            ff/=10;
            ii--;
        }
        //num correctly in char array 
        
        bool putzero = 0;
        for(int k=1; k<=15; k++){
            LCDGoto(k,1);
            if(number[k] == '0' && !putzero && k!=16-point){
            LCDPutChar(' '); 
            }
            else{
            LCDPutChar(number[k]); 
            putzero = 1;
        }}
        }
    }
    r--;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

float chartonum(float f, int point){
    index++;
    float i = input[index-1] - '0';
    
    if(i >= 0 && i < 10){
        return chartonum( 10 * f + i, point)/(1+9*point);
    }
    else if(input[index-1] == '.'){
        return chartonum( f, 1);
    }
    else return f;
}