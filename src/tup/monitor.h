#ifndef tup_monitor_h
#define tup_monitor_h

#define AUTOUPDATE_PID "autoupdate pid"
#define MONITOR_PID_FILE ".tup/monitor.pid"

int monitor_supported(void);
int monitor(int argc, char **argv);
int stop_monitor(int restarting);
int monitor_get_pid(void);

#endif
