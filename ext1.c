#include "backend.h"   // for ARR_COUNT, NEW_ROOM, Room
#include "ext1.h"

const struct Room Ext1TestRoom = NEW_ROOM(1, 0, "Extension Room!");

const struct Room *const Ext1Rooms[] = { &Ext1TestRoom };
const size_t Ext1RoomCount = ARR_COUNT(Ext1Rooms);
