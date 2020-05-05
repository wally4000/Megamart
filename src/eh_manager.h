#ifndef __EH_MANAGER_H__
#define __EH_MANAGER_H__


#define EH_INFO            1
#define EH_WARN            2
#define EH_ERROR           3
#define EH_SEVERE          4
#define EH_DEBUG           5

// Public
void EH_Init();
void EH_Error(int severity, const char *format, ...);
void EH_DrawErrors();

#endif
