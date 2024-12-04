// Based on output by Haxe HLC 4.3.6

#include <arena.h>
#include <backend/GlobalData.h>
#include <backend/Room.h>

#include "game.h"

extern hl_type t$backend_Room;
extern backend__$GlobalData g$_backend_GlobalData;

extern void backend_Room_new(backend__Room, int, int, int);
extern hl__types__ArrayObj hl_types_ArrayObj_alloc(varray *);
void backend_GlobalData_Init(hl__types__ArrayObj);

bool HLInitGameRooms(size_t roomCount, struct CRoomInfo *rooms) {
  varray *rawArray = hl_alloc_array(&t$backend_Room, roomCount);
  if (!rawArray) {
    return false;
  }
  backend__Room *pArray = (backend__Room *)(rawArray + 1);

  printf("Room Count: %i\n", roomCount);
  for (size_t i = 0; i < roomCount; ++i) {
    puts("Test");
    // TODO: Fix hl_alloc_obj crashing some times
    pArray[i] = (backend__Room)hl_alloc_obj(&t$backend_Room);
    if (!pArray[i]) {
      return false;
    }
    backend_Room_new(pArray[i], rooms[i].type, rooms[i].x, rooms[i].y);
  }

  hl__types__ArrayObj array = hl_types_ArrayObj_alloc(rawArray);
  backend_GlobalData_Init(array);
  backend_GlobalData_Log();
  getchar();
  return true;
}

struct CRoomInfo *HLGetGameRoom(struct GameState *state, size_t roomID) {
  hl__types__ArrayObj array = g$_backend_GlobalData->rooms;
  if(!array) {
    hl_null_access();
  }

  backend__Room hlRoom = NULL;
  if(roomID < (size_t)array->length) {
    varray *raw_array = array->array;
    backend__Room *pArray = (backend__Room *)(raw_array + 1);
    hlRoom = pArray[roomID];
  }

  if(!hlRoom) {
    hl_null_access();
  }

  struct CRoomInfo *cRoom = arena_alloc(&(state->arena), sizeof *cRoom);
  if (!cRoom) {
    return NULL;
  }

  cRoom->type = hlRoom->type;
  return cRoom;
}
