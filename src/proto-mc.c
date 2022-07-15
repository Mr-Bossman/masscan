#include "proto-mc.h"
#include "proto-banner1.h"
#include "unusedparm.h"
#include "masscan-app.h"
#include "proto-interactive.h"
#include <ctype.h>
#include <string.h>

static unsigned char hand_shake[] = {0x14, 0x00, 0x00, 0x0e, 0x6d, 0x63, 0x2e, 0x68, 0x79, 0x70, 0x69, 0x78, 0x65, 0x6c, 0x2e, 0x6e, 0x65, 0x74, 0xdd, 0x63, 0x01};

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
mc_clean(struct ProtocolState *stream_state){
}
/***************************************************************************
 ***************************************************************************/
struct ProtocolParserStream banner_mc = {
    "mc", 25565, hand_shake, sizeof(hand_shake), 0,
    mc_selftest,
    mc_init,
    mc_parse,
    mc_clean,
};
