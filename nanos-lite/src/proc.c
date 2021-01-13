#include "proc.h"

#define MAX_NR_PROC 4

void naive_uload(PCB *pcb, const char *filename);

void context_kload(PCB *pcb, void *entry);

void context_uload(PCB *pcb, const char *filename);

void register_pcb(PCB *pcb);

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
    current = &pcb_boot;
}

void hello_fun(void *arg) {
    int j = 1;
    while (1) {
        Log("Hello World from Nanos-lite for the %dth time!", j);
        j++;
        _yield();
    }
}

void init_proc() {
    Log("init ..proc\n");
//    context_kload(&pcb[0], (void *) hello_fun);
    context_uload(&pcb[0], "/bin/dummy");

    switch_boot_pcb();

}

_Context *schedule(_Context *prev) {
    current->cp = prev;
    current = &pcb[0];
//    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
//    Log("schedule success. current PCB: 0x%08x", current);
    return current->cp;
}
