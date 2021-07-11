/*****************************************************************************/
/*                      Covid Bluetooth Trace Simulator                      */
/*****************************************************************************/

/*
 *  A Covid trace simulator via Bluetooth and Mac Addresses.
 *
 *  It simulates checking for a Bluetooth device near us every 0.1 (sec10) for 7 hours and 12 minutes (duration)(30 days)
 *
 *  If the Covid test, which takes place every 144 secs (hour4,) is true it notifies the close contacts 
 *  which are the same mac addresses found between 4 and 20 minutes, after the first contact with them.
 * 
 *  Every 12 secs (min20), non close mac addresses are deleted. Close contacts are kept for 14 days.
 * 
 *  Author : Dimosthenis Iliadis-Apostolidis 
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

//I take everything in microseconds afterwards, because sleep() had problems. I'll use usleep instead.
//10 secs in 30 days is 0.1 secs in 7 hours and 12 minutes
#define sec10 0.1

//4 mins in 30 days is 2.4 secs in 7 hours and 12 minutes
#define min4 2.4

//20 mins in 30 days is 12 secs in 7 hours and 12 minutes
#define min20 12

//14 days in 30 days is 12069 secs in 7 hours and 12 minutes
#define day14 12069

//4 hours in 30 days is 144 secs in 7 hours and 12 minutes
#define hour4 144

//Duration is still in seconds for the comparison.
//7 hours and 12 minutes is 25920 secs.
#define duration 25920
//We have already lost some accuracy due to the above calculations, but it is neglectable.

//Declare variables to calculate time.
float BILLION =1000000000;
struct timespec start, stop;
float elapsed;

//Could also be volatile int, but no real change in this scenario.
int running_p2 = 0;
int running_p3 = 0;
int running_p4 = 0;
int running_p5 = 0;
int wait4m = 0;
int wait20m = 0;
int wait14days = 0;

//Declare struct
struct macaddress{
    void (*BTnearme)();
    uint8_t arr[259200][6];
    int close[259200]; //could be bool to save ram.
    float timestamp[259200];
    uint8_t server[259200][6];
};

struct macaddress mac;

void BTnearme();
bool testCOVID();
void deleteMac(int m);
void uploadContacts(int l);
void *s10(void *arg);
void *m4(void *arg);
void *m20(void *arg);
void *h4(void *arg);
void *d14(void *arg);
FILE *trace;

int main(int argc, char **argv)
{
    //Make seed random for rand().
    srand(time(NULL));

    //Mac addresses are of 48bits -> 6 bytes and each byte can hold the value from 0 to 255.
    //We need 6 "numbers" from 0 to 255. This makes a total of 6 bytes for each mac address (48-bit).
    //Initialization
    for (int i = 0; i<259200; i++){
        mac.timestamp[i] = 0;
        mac.close[i] = 0;
        for (int j = 0; j<6; j++){
            mac.arr[i][j] = 0;
        }
    }

    //Open file. Could also use "wb" for writing binary. I'll use the traditional one instead!
    trace = fopen("trace.txt", "w");

    //start clock
    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    //Run the program for "30" days. It can run "forever" without the last if and break argument.
    while(1){
        pthread_t p1;
        //Search for a mac address every 10 secs.
        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        elapsed = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;
        pthread_create(&p1, NULL, s10, NULL);
        //Call h4 and wait for it.
        if (running_p2 == 0){
            running_p2 = 1;
            pthread_t p2;
            pthread_create(&p2, NULL, h4, NULL);
            if (elapsed>=duration){
                pthread_cancel(p2); 
            }            
        }        
        //Call m4 and wait for it.
        if (running_p3 == 0){
            running_p3 = 1;
            pthread_t p3;
            pthread_create(&p3, NULL, m4, NULL);
            if (elapsed>=duration){
                pthread_cancel(p3); 
            }            
        }        
        //Call m20 and wait for it.
        if (running_p4 == 0){
            running_p4 = 1;
            pthread_t p4;
            pthread_create(&p4, NULL, m20, NULL);  
            if (elapsed>=duration){
                pthread_cancel(p4); 
            }
        }         
        //Call d14 and wait for it
        if (running_p5 == 0){
            running_p5 = 1;
            pthread_t p5;
            pthread_create(&p5, NULL, d14, NULL); 
            if (elapsed>=duration){
                pthread_cancel(p5); 
            }
        }       
        pthread_join(p1, NULL);

        //Have "30" days passed?
        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        elapsed = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;
        if (elapsed>=duration){
            fclose(trace);
            break;
        }
    }
    printf( "Covid took %lf seconds to execute.\n", elapsed);
    return 0;
}

//Add ONE macaddress to the array.
void BTnearme(){
    //Sometimes we don't have people around us (or with BT on)!
    //But we suppose that we find a BT device every time.
    //Enter a mac address into the struct to the first empty spot.
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    float t = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;

    for (int i = 0; i<259200; i++){
        //I take as "empty" the 00:00:00:00:00:00 mac address.
        int sum = 0;
        for(int k = 0; k<6; k++){
            sum = sum + mac.arr[i][k];
            //printf("Sum is %d\n", sum);
        }
        //if sum is 0 and i mod 71 != 0 or if sum is 0 and i is 0, 2, .... 34, .. 68,...,  
        //The same values get set only in pairs and not infinetely, in order to get multiple close contacts. 
        if ((sum == 0 && i % 71 !=0) || (sum == 0 && (i % 2 == 0) )){
            for (int j = 0; j < 6; j++){
                mac.arr[i][j] = rand() % 255;
            }
            mac.timestamp[i] = t; 
            if (trace == NULL){
                printf("Error while opening file.\n");
                exit(-1);
            } 
            fprintf(trace, "%d, %f\n", i, mac.timestamp[i]);
            //printf("i = %d, mac.arr[i][5] = %d\n",i, mac.arr[i][5]);
            break;            
        }
        //Every 71th Spot I have close contact in pairs ( [0]-[71], [71]-[34], etc) Note that i = 0 is set above.
        else if (sum == 0 && i % 71 == 0){
            for (int j = 0; j < 6; j++){
                mac.arr[i][j] = mac.arr[i-71][j];
            }
            mac.timestamp[i] = t;             
            if (trace == NULL){
                printf("Error while opening file.\n");
                exit(-1);
            } 
            fprintf(trace, "%d, %f\n", i, mac.timestamp[i]);
            //printf("i-71 = %d, mac.arr[i][5] = %d\n",(i-71), mac.arr[i-71][5]);
            //printf("i = %d, mac.arr[i][5] = %d\n",i, mac.arr[i][5]);
            break;

        }
    }
}

bool testCOVID(){
    bool result;
    int r = rand() % 100;
    //Choose to be false with a probability of 95/100.
    if (r >= 95){
        result = true; 
    }
    else {
        result = false;
    }
    return result;
}

void deleteMac(int m){
    mac.timestamp[m] = -1;
    mac.close[m] = -1;
    for (int j = 0; j < 6; j++){
        //An all-255 mac address is going to be the "deleted" one (since we use uint8_t).
        mac.arr[m][j] = 255;
        //printf("Deleting close...\n");
    } 
}

void uploadContacts(int l){
    //Test was positive, so we have to find the mac addresses which were close contacts up to l.
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    float upload_t = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;
    for (int i = 0; i < l; i++){
        if (mac.close[i] == 1 && upload_t - mac.timestamp[i]<=day14){
            for (int j = 0; j < 6; j++){
                mac.server[i][j] = mac.arr[i][j];
                //printf("Uploading...\n");
            }
        }
    }
}

//Functions to pass to the threads.
void *s10(void *arg) {
    BTnearme(); 
    usleep(sec10*1000000); // wait for "10" seconds after we run so that we take the first @t = 0s
}

void *h4(void *arg) {
    //printf("got here\n");
    usleep(hour4*1000000); // wait for "4" hours
    //After 4 hours take a covid test. If positive, upload Contacts of at most the last 14 days.
    //Get time as a timestamp the Covid Test.
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    if (testCOVID()==true){
        float test_time = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;
        int l = 0;
        //Find the first 0 (empty mac), in order to make a for loop < l.
        while (l<259200 && mac.timestamp[l]<=test_time)
        {
           if (mac.timestamp[l]==0){
               break;
           }
           else
           {
               l = l + 1;
           }
        }
        //printf("Covid test came back positive...%f\n", test_time);
        //I could save some time by not calling uploadContacts(int l) when I don't have any close contacts.
        //I do call it anyway, since when the test comes back as positive, we have to update the server.
        uploadContacts(l);
    }
    running_p2 = 0;
}

void *m4(void *arg) {
    //Only the first time wait, afterwards it's 1.
    if (wait4m == 0){
        usleep(min4*1000000); // wait for the first "14" days
        wait4m = 1;
    }
    //check for close contact
    for(int i = 0; i<259200; i++)
    {   
        int sum = 0;
        for(int k = 0; k<6; k++){
            sum = sum + mac.arr[i][k];
        }
        //This means it is not empty (all zeros), nor deleted (all 255).
        if (sum != 0 && sum != 1530){
            for(int j = i+1; j<259200; j++){
                if (mac.arr[i][0] == mac.arr[j][0]){
                    if (mac.arr[i][1] == mac.arr[j][1]){       
                        if (mac.arr[i][2] == mac.arr[j][2]){         
                            if (mac.arr[i][3] == mac.arr[j][3]){    
                                if (mac.arr[i][4] == mac.arr[j][4]){  
                                    if (mac.arr[i][5] == mac.arr[j][5]){  
                                        if (mac.timestamp[j]-mac.timestamp[i]>= min4 && mac.timestamp[j]-mac.timestamp[i]<= min20){
                                            //We make the last contact as a close one, so no duplicates.
                                            mac.close[j] = 1;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    running_p3 = 0;   
}

void *m20(void *arg) {
    if (wait20m == 0){
        usleep(min20*1000000); // wait for the first "20" minutes
        wait20m = 1;
    }
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    float delete_nonclose_t = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;
    //After 20 minutes delete MAC addresses which weren't a close contact.
    for (int i = 0; i < 259200; i++){
        int sum = 0;
        for(int k = 0; k<6; k++){
            sum = sum + mac.arr[i][k];
        }
        if (sum != 0 && sum !=1530 && mac.close[i]==0 && delete_nonclose_t - mac.timestamp[i]>=min20){
            mac.timestamp[i] = -1;
            mac.close[i] = -1;
            for (int j = 0; j < 6; j++){
                mac.arr[i][j] = 255;
                //printf("Deleting non-close......\n");
            }
        }
    }
    running_p4 = 0;  
}

void *d14(void *arg) {
    //Only the first time wait, afterwards it's 1.
    if (wait14days == 0){
        sleep(day14); // wait for the first "14" days (integer overflow with usleep(day14*1000000))
        wait14days = 1;
    }
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    float delete_close_t = (float)(( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION) ;
    for (int m = 0; m < 259200; m++)
    {
        if (delete_close_t-mac.timestamp[m] >= day14 && mac.timestamp[m] != -1 && mac.close[m] == 1){
            deleteMac(m);
            break;
        }
    }
    running_p5 = 0;
}
