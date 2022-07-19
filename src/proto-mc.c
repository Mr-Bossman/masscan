#include "proto-mc.h"
#include "proto-banner1.h"
#include "unusedparm.h"
#include "masscan-app.h"
#include "proto-interactive.h"
#include "output.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static unsigned char * hand_shake_ptr = NULL;

unsigned char* hand_shake(uint16_t port, const char* ip,size_t ip_len)
{
  size_t tlen = 7+ip_len;
  unsigned char * ret = (unsigned char *)calloc(1,tlen);
  ret[0] = 6+ip_len;
  ret[3] = ip_len;
  memcpy(ret+4,ip,ip_len);
  ret[tlen-3] = (unsigned char)(port&0xff);
  ret[tlen-2] = (unsigned char)(port>>8);
  ret[tlen-1] = 1;
  return ret;
}

char * banmem;
size_t totalLen=0;
size_t imgstart = 0;
size_t imgend = 0;
void* memstr(void * mem, size_t len, char * str){
    size_t stlen = strlen(str);
    if(len < stlen)
        return 0;
    for(size_t i = 0; i < len-stlen; i++){
        if(!memcmp(mem+i,str,stlen))
            return mem+i;
    }
    return 0;
}
/***************************************************************************
 ***************************************************************************/
static void
mc_parse(  const struct Banner1 *banner1,
          void *banner1_private,
          struct ProtocolState *pstate,
          const unsigned char *px, size_t length,
          struct BannerOutput *banout,
          struct InteractiveData *more)
{
    if(imgstart&&imgend) { // we already found and removed image data
        banout_append(banout, PROTO_MC,px,length);
    } else {
        banmem = realloc(banmem,totalLen+length+1); // expand to add new memory for added paket
        memcpy(banmem+totalLen,px,length); // copy in new packet
        banmem[totalLen] = 0; // add ending 0 for str
        totalLen+=length;
        if(!imgstart) { // dont search again if we found start
            imgstart = (size_t)memstr(banmem,totalLen,"data:image/png;base64");
            if(imgstart)
                imgstart-=(size_t)banmem;
        } else { // we found start but not the end
            if((imgend = (size_t)memchr(banmem+imgstart,'\"',totalLen-imgstart))){ // we found the end
                imgend-=(size_t)banmem;
                memcpy(banmem+imgstart,banmem+imgend,(totalLen-imgend)+1); // copy data after B64
                totalLen=imgstart+(totalLen-imgend); // shrink length to subtract B64 image
                banout_append(banout, PROTO_MC,banmem,totalLen); // print out banner minus image data
                free(banmem); // we dont need to keep track of this any more.
            }
        }
    }

}

/***************************************************************************
 ***************************************************************************/
static void *
mc_init(struct Banner1 *banner1)
{
    hand_shake_ptr = hand_shake(25565,"localhost",9);
    banner_mc.hello = hand_shake_ptr;
    banner_mc.hello_length = hand_shake_ptr[0]+1;
    banner1->payloads.tcp[25565] = (void*)&banner_mc;
    return 0;
}

/***************************************************************************
 ***************************************************************************/
static int
mc_selftest(void)
{
    return 0;
}

/***************************************************************************
 ***************************************************************************/
static void
mc_clean(struct ProtocolState *stream_state)
{
    free(hand_shake_ptr);
}

/***************************************************************************
 ***************************************************************************/
static void
mc_callback(const struct Banner1 *banner1, struct InteractiveData *more)
{
    char *statusQ = malloc(2);
    statusQ[0] = 1;
    statusQ[1] = 0;
    more->m_length = 2;
    more->m_payload = (void *)statusQ;
    more->is_payload_dynamic = 1;
}

/***************************************************************************
 ***************************************************************************/
struct ProtocolParserStream banner_mc = {
    "mc", 25565, 0, 0, 0,
    mc_selftest,
    mc_init,
    mc_parse,
    mc_clean,
    0,
    mc_callback
};
