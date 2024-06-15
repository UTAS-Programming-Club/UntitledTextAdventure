#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "../frontends/frontend.h"
#include "fileloading.h"

#define UNUSED(x) (void)(x)

bool LoadFile(char *path, size_t *size, void **data, uint16_t resourceID, void *resourceType) {
#if defined(_WIN32) && defined(FRONTEND) && !defined(_DEBUG)
  void *resource = MAKEINTRESOURCEW(resourceID);
  HRSRC hResource = FindResourceW(NULL, resource, resourceType);
  if (hResource) {
    HGLOBAL hData = LoadResource(NULL, hResource);
    if (!hData) {
      return false;
    }

    *size = SizeofResource(NULL, hResource);
    *data = LockResource(hData);
    return true;
  }
#else
  UNUSED(resourceID);
  UNUSED(resourceType);
#endif
  bool success = false;

  if (!data) {
    return false;
  }

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    PrintError("Unable to load %s, file could not be opened. Check if it exists and if you have permission to open it", path);
    goto cleanup;
  }

  struct stat st;
  if (fstat(fd, &st)) {
    goto cleanup;
  }

  static_assert(sizeof(off_t) <= sizeof(size_t));
  *size = st.st_size;
  *data = malloc(*size);

  // If this file gets large then switch to mmap and possibly a streaming json library
  if (read(fd, *data, *size) == -1) {
    goto cleanup;
  }

  success = true;

cleanup:
  close(fd);
  return success;
}

void UnloadFile(void *data, uint16_t resourceID, void *resourceType) {
#if defined(_WIN32) && defined(FRONTEND) && !defined(_DEBUG)
  void *resource = MAKEINTRESOURCEW(resourceID);
  HRSRC hResource = FindResourceW(NULL, resource, resourceType);
  if (hResource) {
    // Only relevant on very old versions of windows
    // On such versions I have no clue if FindResourceW/LoadResource work after LockResource
    // HGLOBAL hData = LoadResource(NULL, hResource);
    // if (!hData) {
    //   return false;
    // }
    // UnlockResource(hData);
    FreeResource(hResource);
    return;
  }
#else
  UNUSED(resourceID);
  UNUSED(resourceType);
#endif
  free(data);
}
