#ifndef __TIMER_MODULE_H_
#define __TIMER_MODULE_H_


void init_timer();
bool get_state_timeout();
void clean_state_timeout();
void open_printer_timeout_timer();
void close_printer_timeout_timer();
bool get_printer_timeout_status();

#endif