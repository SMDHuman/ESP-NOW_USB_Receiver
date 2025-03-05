//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "slip_decoder.h"
#include "serial_com.h"
#include "utils.h"


static uint8_t slip_package_buffer[SLIP_BUFFER_SIZE] = {};
static size_t slip_buffer_index;
static uint8_t slip_package_ready; 

static uint64_t checksum = 0;

//-----------------------------------------------------------------------------
void slip_init(){
}
//-----------------------------------------------------------------------------
void slip_task(){

}
//-----------------------------------------------------------------------------
void slip_push(uint8_t data){
    static uint8_t esc_flag = false;
    if(slip_package_ready){
        slip_reset();
    }
    if(SLIP_BUFFER_SIZE == slip_buffer_index){
        slip_reset();
        serial_send_debug("! SLIP BUFFER OVERFLOW !");
    }
    if(esc_flag){
        if(data == S_ESC_END){
            slip_package_buffer[slip_buffer_index++] = S_END;
            checksum += S_END;
        }
        else if(data == S_ESC_ESC){
            slip_package_buffer[slip_buffer_index++] = S_ESC;
            checksum += S_ESC;
        }
        esc_flag = false;
    }
    else if(data == S_ESC){
        esc_flag = true;
    }
    else if(data == S_END){
        slip_package_ready = true;
        #ifdef CHECKSUM_ENABLE
            slip_buffer_index -= 4;
        #endif
    }
    else{
        slip_package_buffer[slip_buffer_index++] = data;
        checksum += data;
    }
}
//-----------------------------------------------------------------------------
uint8_t slip_is_ready(){
    if(slip_package_ready){
        size_t chsm = 0;
        //...
        #ifdef CHECKSUM_ENABLE
            for(size_t i = 0; i < 4; i++){
                size_t d = slip_package_buffer[i+slip_buffer_index];
                checksum -= d;
                chsm += (d << (i*8));
            }
            if(checksum == chsm){
                return(true);
            }else{
                slip_reset();
            }
            return(false);
        #endif
        //...
        #ifndef CHECKSUM_ENABLE
            return(true);
        #endif
    }
    return(false);
}
//-----------------------------------------------------------------------------
void slip_get_package(uint8_t **buf, size_t *len){
    *buf = &slip_package_buffer[0];
    *len = slip_buffer_index;
}

//-----------------------------------------------------------------------------
void slip_reset(){
    memset(slip_package_buffer, 0, SLIP_BUFFER_SIZE);
    slip_buffer_index = 0;
    slip_package_ready = false;
    checksum = 0;
}