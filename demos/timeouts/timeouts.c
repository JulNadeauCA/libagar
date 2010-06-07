/*	Public domain	*/
/*
 * This application tests the AG_Timeout(3) interface.
 */
#include <agar/core.h>
#include <agar/gui.h>

AG_Timeout to1, to2;

Uint32
Timeout1(void *obj, Uint32 ival, void *arg)
{
  printf("timeout 1\n");
  return 0;
}

Uint32
Timeout2(void *obj, Uint32 ival, void *arg)
{
  printf("timeout 2\n");
  return 0;
}

void
ScheduleTimeouts(AG_Event *event)
{
  AG_Object *ob;
  AG_Timeout *to;

  printf("schedule timeout1 ival=1000\n");
  AG_ScheduleTimeout(NULL, &to1, 1000);
  printf("schedule timeout2 ival=2000\n");
  AG_ScheduleTimeout(NULL, &to2, 2000);

  printf("timeout queue:\n");
  /* print the timeout tailqueue */
  AG_TAILQ_FOREACH(ob, &agTimeoutObjQ, tobjs) {
    printf("---- obj %s : ", ob->name);
    AG_TAILQ_FOREACH(to, &ob->timeouts, timeouts) {
      char *name;
      if (to == &to1)
	name = "timeout1";
      else if (to == &to2)
	name = "timeout2";
      else
	name = "unknown timeout";
      printf("-- timeout %s at %d ticks ", name, to->ticks);
    }
    printf("\n");
  }
}

int
main(int argc, char **argv)
{
  AG_Window *win;
  AG_Button *btn;

  AG_InitCore("timeouts", 0);
  AG_InitVideo(320, 200, 32, 0);

  win = AG_WindowNew(AG_WINDOW_PLAIN|AG_WINDOW_NOMOVE);
  btn = AG_ButtonNewFn(win, 0, "Schedule timeouts", ScheduleTimeouts, "");

  AG_Expand(btn);
  AG_WindowShow(win);
 
  AG_SetTimeout(&to1, Timeout1, NULL, 0);
  AG_SetTimeout(&to2, Timeout2, NULL, 0);
 
  AG_EventLoop();
  return 0;
}
