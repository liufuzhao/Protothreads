#include "pt_port.h"
#include "pt_box_extern.h"
#include "pt_list.h"

AOS_SLIST_HEAD(boxs_head);

void pt_box_init(pt_box_t *box)
{
    box->state = PT_BOX_EMPTY_STATE;
    box->p_addr = NULL;
    box->p_msg = NULL;
    box->fir_accept_pt = NULL;
    slist_add(&box->list, &boxs_head);
}

void pt_box_get_target(pt_box_t *box, void **target)
{
    *target = box->p_addr;
}

void pt_box_get_msg(pt_box_t *box, void **msg)
{
    *msg = box->p_msg;
}

void pt_box_get_state(pt_box_t *box, uint32_t *state)
{
    *state = box->state;
}

int pt_boxs_handled(pt_box_t *box)
{
    if (box->state == PT_BOX_ACCEPT_STATE)
    {
        box->state = PT_BOX_MAN_HANDLED_STATE;
        return PT_TRUE;
    }
    else
    {
        return PT_FALSE;
    }
}
// 每个线程运行前需要执行一次
void pt_boxs_auto_handled(struct pt *pt)
{
    pt_box_t *node = NULL;
    slist_t *tmp = NULL;
    slist_for_each_entry_safe(&boxs_head, tmp, node, pt_box_t, list)
    {
        if (node)
        {
            if (pt == node->fir_accept_pt)
            {
                node->state = PT_BOX_AUTO_HANDLED_STATE;
                node->fir_accept_pt = NULL;
                node->p_addr = NULL;
                node->p_msg = NULL;
            }
        }
    }
}
