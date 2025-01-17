#include <stdio.h>
#include <string.h>

#include "semtech_loramac.h"
#include "xtimer.h"
#include "fmt.h"



#define DEVEUI "CAFEBABEABCC87DA"
#define APPEUI "CAFEBABE00000000"
#define APPKEY "05B599C06DAC46FE1EDADFE6C765EB2F"

#define MAX_MESSAGE_LENGTH 128

extern semtech_loramac_t loramac;

static uint8_t deveui[LORAMAC_DEVEUI_LEN] ;
static uint8_t appeui[LORAMAC_APPEUI_LEN] ;
static uint8_t appkey[LORAMAC_APPKEY_LEN] ;


int lora_init(void){
	puts("Start init lora\n");
	/* Convert identifiers and application key */
	fmt_hex_bytes(deveui, DEVEUI);
	fmt_hex_bytes(appeui, APPEUI);
	fmt_hex_bytes(appkey, APPKEY);

	/* set the LoRaWAN keys */
	semtech_loramac_set_deveui(&loramac, deveui);
	semtech_loramac_set_appeui(&loramac, appeui);
	semtech_loramac_set_appkey(&loramac, appkey);
	semtech_loramac_set_adr(&loramac, true);
	semtech_loramac_set_dr(&loramac, 0);
	
	switch (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA)) {
            case SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED:
                puts("Cannot join: dutycycle restriction\n");
                return 1;
            case SEMTECH_LORAMAC_BUSY:
                puts("Cannot join: mac is busy\n");
                return 1;
            case SEMTECH_LORAMAC_JOIN_FAILED:
                puts("Join procedure failed!\n");
                return 1;
            case SEMTECH_LORAMAC_ALREADY_JOINED:
                puts("Warning: already joined!\n");
                return 1;
            case SEMTECH_LORAMAC_JOIN_SUCCEEDED:
                puts("Join procedure succeeded!\n");
                break;
            default: /* should not happen */
                puts("Critical Lora Init\n");
                break;
        }
        semtech_loramac_set_tx_mode(&loramac, LORAMAC_TX_UNCNF);
        semtech_loramac_set_tx_port(&loramac, 2);
        return 0;
}


int main(void){
    
    if (lora_init() == 0){
        puts("Lora Initialized\n");
        char payload[50];
        int duration = 60;

        int data1=0, data2=0, data3=0;
        bool flag_data = true;

        while (1){
            /**** Generate Data ****/
            if (data1 >= 50){
                flag_data = false;
            }
            if (data1 <= -50){
                flag_data = true;
            }
            if (flag_data){
                data1 += 5;
                data2 += 10;
                data3 -= 5;
            }else{
                data1 -= 5;
                data2 -= 10;
                data3 += 5;
            }

            /**** Build payload ****/
			sprintf(payload,"o=%03d ,v=%03d ,p=%03d",(int)data1, data2, data3);
			
			/**** Send payload via LoRaWAN ****/
			switch (semtech_loramac_send(&loramac,  (uint8_t *)payload, strlen(payload))) {
			    case SEMTECH_LORAMAC_NOT_JOINED:
				    puts("Cannot send: not joined\n");
				    break;
				case SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED:
				    puts("Cannot send: dutycycle restriction\n");
				    break;
			    case SEMTECH_LORAMAC_BUSY:
				    puts("Cannot send: MAC is busy\n");
				    break;

			    case SEMTECH_LORAMAC_TX_ERROR:
				    puts("Cannot send: error\n");
				    break;

			    case SEMTECH_LORAMAC_TX_CNF_FAILED:
				    puts("Fail to send: no ACK received\n");
                    break;
				}
            xtimer_sleep(duration);
            puts("Looping...\n");
        }
    }else{
        puts("Erreur : Init Lora\n");
    }
    return 0;
}