#include "backend.h"   // for USE_ACTION, ARR_COUNT, NEW_SCREEN, Screen
#include "coregame.h"  // for CoreGoEastAction, CoreGoNorthAction, CoreGoSouthAction, CoreGoWestAction
#include "ext1.h"

const struct Screen Ext1TestScreen = NEW_SCREEN(1, 0, "Extension Screen!", 
                                                USE_ACTION(CoreGoNorthAction), USE_ACTION(CoreGoEastAction), USE_ACTION(CoreGoSouthAction), USE_ACTION(CoreGoWestAction)
);

const struct Screen *const Ext1Screens[] = { &Ext1TestScreen };
const size_t Ext1ScreenCount = ARR_COUNT(Ext1Screens);
