#include "proto-mc.h"
#include "proto-banner1.h"
#include "unusedparm.h"
#include "masscan-app.h"
#include "proto-interactive.h"
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

    pstate->state = length;

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
