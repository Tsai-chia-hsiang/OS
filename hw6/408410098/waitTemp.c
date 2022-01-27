#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h> 
#include <assert.h>
#include <string.h>
#include <stdbool.h>


int numCPU = -1;
int temp[4];
//很爛的寫法，應該寫成陣列
FILE* temp1, *temp2, *temp3, *temp4;
//底下這程式碼會印出所有CPU的溫度
int printTemp() {

    int eatReturn;
    int tempArray[2];

    temp1 = fopen("/sys/class/hwmon/hwmon6/temp2_input", "r");
    assert(temp1 != NULL);
    temp2 = fopen("/sys/class/hwmon/hwmon6/temp3_input", "r");
    assert(temp2 != NULL);


    for (int i=0; i<2; i++){
        tempArray[i] = -1;
    }

    fseek(temp1, 0, SEEK_SET);
    //eatReturn是明確的把「回傳值」吃掉，讓編譯器不要出現警告訊息
    eatReturn=fscanf(temp1, "%d", &tempArray[0]);
    fseek(temp2, 0, SEEK_SET);
    eatReturn=fscanf(temp2, "%d", &tempArray[1]);

    fclose(temp1); fclose(temp2);

    int totalTemp=0;
    for (int i=0; i<2; i++){
        totalTemp += tempArray[i];
    }
    int avgTemp = totalTemp/2;

    return avgTemp;
}

int main(int argc, char **argv) {
    //預設值等到CPU的溫度降到35度，tempture是目標溫度
    int tempture=35;
    //使用者可以在參數列輸入溫度
    if (argc == 2) {
        sscanf(argv[1], "%d", &tempture);
    }
    //授課老師自己稍微看一下，這個溫度要乘上1000才是真正的溫度
    tempture = tempture * 1000;

    //一個迴圈，每隔一秒讀取CPU的溫度，直到溫度比設定的還要低
    int temp=printTemp();
    int try=1;
 
    
    while((temp = printTemp()) > tempture) {
        fprintf(stderr, "#=%02d sec , tempurture =  %.2f ℃\n", try, ((float)temp)/1000);
        try++;
        sleep(1);
    }
    printf("temp %.2f℃, \n", ((float)temp)/1000);
    exit(0);
}
