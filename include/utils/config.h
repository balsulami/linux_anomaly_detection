//
// Created by root on 12/8/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_CONFIG_H_H
#define HIGHPERFORMANCELINUXSENSORS_CONFIG_H_H
#include <string>

std::string FTRACE_PATH = "/sys/kernel/debug/tracing";
std::string SYSCALLS_PATH = "/sys/kernel/debug/tracing/events/raw_syscalls";
std::string SYS_ENTRY = "sys_enter";
std::string SYS_EXIT = "sys_exit";

#endif //HIGHPERFORMANCELINUXSENSORS_CONFIG_H_H
