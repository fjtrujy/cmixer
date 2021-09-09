#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sbv_patches.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <loadfile.h>
#include <audsrv.h>

#include "cmixer.h"

extern unsigned char libsd_irx;
extern unsigned int size_libsd_irx;

extern unsigned char audsrv_irx;
extern unsigned int size_audsrv_irx;

#define STACK_SIZE 16384
#define BUFFER_SIZE 2048

static int ee_threadID = 0;
static char chunk[BUFFER_SIZE];
static int fillbuffer_sema;

static void load_modules() {
   /* Audio */
   SifExecModuleBuffer(&libsd_irx, size_libsd_irx, 0, NULL, NULL);
   SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, NULL);

   /* Initializes audsrv library */
   if (audsrv_init()) {
      printf("audsrv library not initalizated\n");
   }
}

static void reset_IOP() {
   SifInitRpc(0);
   while(!SifIopReset(NULL, 0)){};

   while(!SifIopSync()){};
   SifInitRpc(0);
   sbv_patch_enable_lmb();
   sbv_patch_disable_prefix_check();
}

static void lock_handler(cm_Event *e) {
  if (e->type == CM_EVENT_LOCK) {
    WaitSema(fillbuffer_sema);
  }
  if (e->type == CM_EVENT_UNLOCK) {
    iSignalSema(fillbuffer_sema);
  }
}

void audioThread(void *data) {
  while (1) {
    cm_process((short int *)chunk, BUFFER_SIZE / 2);
    audsrv_wait_audio(BUFFER_SIZE);
    audsrv_play_audio(chunk, BUFFER_SIZE);
  }
}


static void configureSemaphore() {
  /* Semaphore */
  ee_sema_t sema;
  sema.init_count = 1;
	sema.max_count = 1;
	sema.option = 0;
	fillbuffer_sema = CreateSema(&sema);
}

static int createAudioThread() {
  /* Create thread */
  ee_thread_t th_attr;
  th_attr.stack_size 	 = STACK_SIZE;
	th_attr.gp_reg 		 = (void *)&_gp;
	th_attr.initial_priority = 64;
	th_attr.option 		 = 0;

  th_attr.func = (void *)audioThread;
	th_attr.stack = (void *)malloc(STACK_SIZE);
	if (th_attr.stack == NULL) {
	  printf("Error: Creating audio thread, No memory\n");
    return -1;
	}

	ee_threadID = CreateThread(&th_attr);
	if (ee_threadID < 0) {
		printf("Error: Creating audio thread, No thread\n");
		return -1;
	}

	StartThread(ee_threadID, 0);
  return ee_threadID > 0 ? 0 : -1;
}

int main(int argc, char **argv) {
  cm_Source *src;

  reset_IOP();
  load_modules();

  configureSemaphore();

  /* Audio Config */
  struct audsrv_fmt_t ps2_format;
  ps2_format.bits     = 16;
  ps2_format.freq     = 44100;
  ps2_format.channels = 2;
  audsrv_set_format(&ps2_format);
  audsrv_set_volume(MAX_VOLUME);

  if (createAudioThread()){
    fprintf(stderr, "Error: Creating audio thread\n");
    exit(EXIT_FAILURE);
  }

  // /* Init library */
  cm_init(ps2_format.freq);
  cm_set_lock(lock_handler);
  cm_set_master_gain(0.5);

  // /* Create source and play */
  src = cm_new_source_from_file("loop.wav");
  if (!src) {
    fprintf(stderr, "Error: failed to create source '%s'\n", cm_get_error());
    exit(EXIT_FAILURE);
  }

  cm_set_loop(src, 1);
  cm_play(src);
  
  // /* Wait for [return] */
  printf("Press [return] to exit\n");
  getchar();
  /* Doing SleepThread because getChar doesn't work in PS2 */
  SleepThread();

  // /* Clean up */
  cm_destroy_source(src);
  audsrv_quit();

  return EXIT_SUCCESS;
}
