#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static bool su = false;
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

WP *new_wp() {
    // 没有初始化
    if (su == true) {
        init_wp_pool();
        su = false;
    }
    if (free_ == NULL) {
        printf("Error,free is null.\n");
        assert(0);
    }
    WP *new = NULL;
    // free指针移动
    new = free_;
    free_ = free_->next;
    new->is_used = true;
    new->next = head;
    head = new;
    return new;
}

int free_wp(int no) {
    if (head == NULL) {
        printf("监视点列表为空\n");
        return -1;
    } else {
        WP *cur = head;
        WP *pre = NULL;
        while (cur != NULL && cur->NO != no) {
            pre = cur;
            cur = cur->next;
        }
        if (cur == NULL) {
            return -1;
        }
        //头部节点
        if (pre == NULL) {
            head = cur->next;
        } else {
            pre->next = cur->next;
        }
        cur->is_used = false;
        cur->value = 0;
        cur->next = free_;
        free_ = cur;
    }
    return 0;
}

void print_wp() {
    WP *p = head;
    if (p == NULL) {
        printf("监视点为空\n");
        return;
    } else {
        while (p != NULL) {
            printf("%-8d%-9s%u(%#x)\n", p->NO, p->expr, p->value, p->value);
            if (p->hit > 0)
                printf("        breakpoint already hit %d time\n", p->hit);
            p = p->next;
        }
    }
    return;
}

bool check_wp() {
    bool changed = false;
    WP *wp = head;
    while (wp != NULL) {
        bool success = true;
        uint32_t new_val = expr(wp->expr, &success);
        Assert(success, "watchpoint expr must be success.\n");
        if (new_val != wp->value) {
            printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
            printf("Old value = %u\n", wp->value);
            printf("New value = %u\n", new_val);
            Log("");
            changed = true;
            wp->value = new_val;
            wp->hit++;
        }
        wp = wp->next;
    }
    return changed;
}


