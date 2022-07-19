#include "proto-mc.h"
#include "proto-banner1.h"
#include "unusedparm.h"
#include "masscan-app.h"
#include "proto-interactive.h"
#include "output.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static unsigned char hand_shake_ptr[128];
static unsigned char statusQ[] = {1,0};

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
    struct MCSTUFF *mc = &pstate->sub.mc;

    if(mc->imgstart&&mc->imgend) { // we already found and removed image data
        banout_append(banout, PROTO_MC,px,length);
    } else {
        mc->banmem = realloc(mc->banmem,mc->totalLen+length+1); // expand to add new memory for added paket
        memcpy(mc->banmem+mc->totalLen,px,length); // copy in new packet
        mc->banmem[mc->totalLen] = 0; // add ending 0 for str
        mc->totalLen+=length;
        if(!mc->imgstart) { // dont search again if we found start
            mc->imgstart = (size_t)memstr(mc->banmem,mc->totalLen,"data:image/png;base64");
            if(mc->imgstart)
                mc->imgstart-=(size_t)mc->banmem;
        } else { // we found start but not the end
            if((mc->imgend = (size_t)memchr(mc->banmem+mc->imgstart,'\"',mc->totalLen-mc->imgstart))){ // we found the end
                mc->imgend-=(size_t)mc->banmem;
                memcpy(mc->banmem+mc->imgstart,mc->banmem+mc->imgend,(mc->totalLen-mc->imgend)+1); // copy data after B64
                mc->totalLen=mc->imgstart+(mc->totalLen-mc->imgend); // shrink length to subtract B64 image
                banout_append(banout, PROTO_MC,mc->banmem,mc->totalLen); // print out banner minus image data
                free(mc->banmem); // we dont need to keep track of this any more.
            }
        }
    }
    for(size_t i = 0; i < length; i++){
        if(px[i] == '{')
            mc->brackcount++;
        if(px[i] == '}')
            mc->brackcount--;
    }
    if(mc->brackcount <= 0)
        more->is_closing = 1;
}

/***************************************************************************
 ***************************************************************************/
static void *
mc_init(struct Banner1 *banner1)
{
    unsigned char * tmp = hand_shake(25565,"localhost",9);
    memcpy(hand_shake_ptr,tmp,tmp[0]+1);
    free(tmp);
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
mc_callback(const struct Banner1 *banner1, struct InteractiveData *more)
{
    more->m_length = 2;
    more->m_payload = (void *)statusQ;
    more->is_payload_dynamic = 0;
}

/***************************************************************************
 ***************************************************************************/
struct ProtocolParserStream banner_mc = {
    "mc", 25565, 0, 0, 0,
    mc_selftest,
    mc_init,
    mc_parse,
    0,
    0,
    mc_callback
};
