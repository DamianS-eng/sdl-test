#include <stdio.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

static const char *music_file = "music.wav";

static const char *name = "Audio Demo";
static const char *version = "1";
static const char *appid = "io.damians-eng.demo";

static SDL_AudioDeviceID audio_device = 0;
static SDL_AudioStream *stream = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {

  if(!SDL_Init(SDL_INIT_AUDIO) || !SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); 
    return SDL_APP_FAILURE;
  }

  if(!SDL_SetAppMetadata(name, version, appid)) {
    fprintf(stderr, "Some problem setting metadata: %s\n", SDL_GetError()); 
    return SDL_APP_FAILURE;
  }

  SDL_AudioSpec wavespec;
  Uint8 *wavebuff = NULL;
  Uint32 wavelen = 0;
  //FIXME create a switch statement in a function that returns a string depending on which step failed, then do printf, free buff, and SDL_APP_FAILURE
  if (!SDL_LoadWAV(music_file, &wavespec, &wavebuff, &wavelen)) {
    fprintf(stderr, "Cannot load wav file! %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_AudioSpec desired;
  SDL_zero(desired);
  desired.freq = 48000;             // Hz
  desired.format = SDL_AUDIO_F32;       // floating point
  desired.channels = 2;             // stereo
  // desired doesn't use samples or callback in SDL3
  // desired.samples = 4096;           // magic 
  // desired.callback = NULL;          // if no data, play silence
  
  // for audio_device, at least in SDL2
  // name of device (we don't know)  -> NULL
  // output device                   -> 0
  // specify how it's wanted (shrug) -> memset all to 0
  // don't change the format         -> NULL
  // return what is asked or fail    -> 0
  // audio_device = SDL_OpenAudioDevice(NULL, 0, &wavespec, NULL, 0);
  //SDL_QueueAudio(audio_device, wavebuff, wavelen);
  
  audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
  if(!audio_device) {
    fprintf(stderr, "Audio device error: %s\n", SDL_GetError()); 
    SDL_free(wavebuff); 
    return SDL_APP_FAILURE; 
  }

  stream = SDL_CreateAudioStream(&wavespec, &desired);
  if(!stream) {
    fprintf(stderr, "Stream Problem. %s\n", SDL_GetError());
    SDL_free(wavebuff);
    return SDL_APP_FAILURE;
  }

  if (SDL_PutAudioStreamData(stream, wavebuff, wavelen) < 0) { 
    fprintf(stderr, "Stream put error: %s\n", SDL_GetError()); 
    SDL_free(wavebuff); 
    return SDL_APP_FAILURE; 
  }

  SDL_free(wavebuff);

  if (SDL_BindAudioStream(audio_device, stream) < 0) { 
    fprintf(stderr, "Failed to bind stream to device: %s\n", SDL_GetError()); 
    return SDL_APP_FAILURE; 
  }

  // SDL_Window *win = SDL_CreateWindow(title, width, height, 0))
  if (!SDL_CreateWindow("Demo", 800, 600, 0)) {
    fprintf(stderr, "Cannot create window...  %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_ResumeAudioDevice(audio_device);
  return SDL_APP_CONTINUE;
};

SDL_AppResult SDL_AppIterate(void *appstate) {
  if (SDL_GetAudioStreamAvailable(stream) == 0 && SDL_AudioDevicePaused(audio_device)) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
};

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
    {
      return SDL_APP_SUCCESS;
    }
  return SDL_APP_CONTINUE;
};

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (audio_device) {
    SDL_CloseAudioDevice(audio_device);
    audio_device = 0;
  }
  if (stream) {
    SDL_DestroyAudioStream(stream);
    stream = NULL;
  }
  //SDL_DestroyGPUDevice(device);
  //SDL_DestroyWindow(window);
};
