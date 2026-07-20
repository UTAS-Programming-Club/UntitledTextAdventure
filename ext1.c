#include <stdint.h>

#include "backend.h"
#include "coregame.h"
#include "ext1.h"

const struct Screen Ext1TestScreen = NEW_SCREEN(0, 1, "Extension Screen!", 
                                                USE_ACTION(CoreGoNorthAction), USE_ACTION(CoreGoEastAction), USE_ACTION(CoreGoSouthAction), USE_ACTION(CoreGoWestAction)
);

const struct Screen *const Ext1Screens[] = { &Ext1TestScreen };
const size_t Ext1ScreenCount = ARR_COUNT(Ext1Screens);
