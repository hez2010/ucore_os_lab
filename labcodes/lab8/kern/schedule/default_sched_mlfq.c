#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include <default_sched.h>

#define QUEUE_NUM 6

static list_entry_t queue[QUEUE_NUM];
static int factor[QUEUE_NUM];

static void
mlfq_init(struct run_queue *rq) {
    for (int i = 0; i < QUEUE_NUM; i++) {
        list_init(&queue[i]);
        factor[i] = i == 0 ? 1 : factor[i - 1] * 2;
    }
    rq->proc_num = 0;
}

static void
mlfq_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    // add process to queue
    list_add_before(&queue[proc->mlfq_queue], &(proc->run_link));
    int max_slice = rq->max_time_slice * factor[proc->mlfq_queue];
    if (proc->time_slice == 0 || proc->time_slice > max_slice) {
        // reset
        proc->time_slice = max_slice;
    }
    proc->rq = rq;
    rq->proc_num++;
}

static void
mlfq_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    list_del_init(&(proc->run_link));
    rq->proc_num--;
}

static struct proc_struct *
mlfq_pick_next(struct run_queue *rq) {
    for (int i = 0; i < QUEUE_NUM; i++) {
        if (list_empty(&queue[i])) continue;
        list_entry_t *le = list_next(&queue[i]);
        struct proc_struct *p = le2proc(le, run_link);
        if (p->mlfq_queue < QUEUE_NUM - 1) {
            p->mlfq_queue++;
        }
        return p;
    }
    return NULL;
}

static void
mlfq_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice--;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;
    }
}

struct sched_class default_sched_class = {
     .name = "mlfq_scheduler",
     .init = mlfq_init,
     .enqueue = mlfq_enqueue,
     .dequeue = mlfq_dequeue,
     .pick_next = mlfq_pick_next,
     .proc_tick = mlfq_proc_tick,
};
