#include <AL/AL.h>

long
ALinit(void)
{
  initialize_ale_error_table();
  initialize_alw_error_table();
  return 0L;
}

long
ALinitAL(ALsession session, ALflag_t initial_flags)
{
  int ret;

  ret = ALinit();
  if (ret) return ret;

  ret = ALinitSession(session);
  if (ret) return ret;

  ret = ALinitUtmp(session);
  if (ret) return ret;

  ret = ALinitUser(session, initial_flags);
  return ret;
}

long
ALinitSession(ALsession session)
{
  memset(session, 0, sizeof(*session));
  return 0L;
}
