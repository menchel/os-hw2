#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/list.h>

asmlinkage long sys_hello(void)
{
	printk("Hello, World\n");
	return 0;
}

asmlinkage long sys_set_weight(int weight)
{
	if(weight<0)
	{
		return -EINVAL;
	}
	current->process_weight=weight;
	return 0;
}

asmlinkage long sys_get_weight(void)
{
	return current->process_weight;
}

asmlinkage long sys_get_path_sum(pid_t target)
{
	long path_sum = 0;
	struct task_struct *current_task;
    struct task_struct *target_task;

	//get the task struct of target
    target_task = pid_task(find_vpid(target), PIDTYPE_PID);
    if (!target_task) 
	{
        return -ECHILD;
    }
	//loop over target parents
    for (current_task = target_task; current_task->pid != 1; current_task = current_task->parent) 
	{
        path_sum += current_task->process_weight; //add weight
        if (current == current_task) // we reached the current process
		{
            return path_sum;
        }
		if (!current_task->parent) //null parent 
		{
			break;
		}
    }
    return -ECHILD;
}

asmlinkage long sys_get_heaviest_sibling(void)
{
	struct task_struct *parent_task;
    struct task_struct *heaviest_weight_sibling_task=current;
	struct task_struct *current_task;
	struct list_head* list_element;
	long heaviest_weight_sibling_weight=current->process_weight;

	parent_task = current->parent;
	if(!parent_task) //no parent, so no siblings
	{
		return current->pid;
	}
	if(list_empty(&parent_task->children)) // no children
	{
       return current->pid;
    }
    list_for_each(list_element, &parent_task->children)
	{
        current_task = list_entry(list_element, struct task_struct, sibling);
		if(heaviest_weight_sibling_weight<current_task->process_weight)
		{
			heaviest_weight_sibling_weight=current_task->process_weight;
			heaviest_weight_sibling_task=current_task;
		}
		if(heaviest_weight_sibling_weight==current_task->process_weight)
		{
			if(heaviest_weight_sibling_task->pid>current_task->pid)
			{
				heaviest_weight_sibling_weight=current_task->process_weight;
				heaviest_weight_sibling_task=current_task;
			}
		}
    }
	return heaviest_weight_sibling_task->pid;
}