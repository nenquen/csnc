#ifndef SDL_filesystem_h_
#define SDL_filesystem_h_

#include "SDL_stdinc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Minimal compatibility header.
 * Some parts of the engine expect SDL_filesystem.h + SDL_GetBasePath.
 * The bundled SDL headers in this repo do not ship this header.
 */

extern DECLSPEC char * SDLCALL SDL_GetBasePath(void);

#ifdef __cplusplus
}
#endif

#endif /* SDL_filesystem_h_ */
