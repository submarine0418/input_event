// serial_wasd_ldisc.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_ldisc.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define MY_LDISC 29

struct swd_dev {
    struct tty_struct *tty;
    struct input_dev *input;
    char buf[64];
    size_t buf_len;
    spinlock_t lock;
};

static struct swd_dev *swd;

/* ==================== Key Event Parser ==================== */
static void swd_parse_line(struct swd_dev *swd, const char *line)
{
    if (!swd || !swd->input) return;

    char key = line[0];
    char val = line[1];   // '1' or '0'
    int press = (val == '1');

    unsigned int code = 0;

    switch (key) {
        case 'W': code = KEY_W; break;
        case 'A': code = KEY_A; break;
        case 'S': code = KEY_S; break;
        case 'D': code = KEY_D; break;
        default:
            return; // Unknown line
    }

    input_report_key(swd->input, code, press);
    input_sync(swd->input);
}

/* ==================== LDISC Open ==================== */
static int swd_open(struct tty_struct *tty)
{
    if (!swd) {
        swd = kzalloc(sizeof(*swd), GFP_KERNEL);
        if (!swd) return -ENOMEM;

        spin_lock_init(&swd->lock);

        swd->tty = tty;

        swd->input = input_allocate_device();
        if (!swd->input) {
            kfree(swd);
            swd = NULL;
            return -ENOMEM;
        }

        swd->input->name = "serial-wasd";
        swd->input->id.bustype = BUS_RS232;

        __set_bit(EV_KEY, swd->input->evbit);
        __set_bit(KEY_W, swd->input->keybit);
        __set_bit(KEY_A, swd->input->keybit);
        __set_bit(KEY_S, swd->input->keybit);
        __set_bit(KEY_D, swd->input->keybit);

        input_register_device(swd->input);
    }

    tty->disc_data = swd;
    return 0;
}

/* ==================== LDISC Close ==================== */
static void swd_close(struct tty_struct *tty)
{
    struct swd_dev *s = tty->disc_data;

    if (!s) return;

    if (s->input) {
        input_unregister_device(s->input);
        s->input = NULL;
    }

    kfree(s);
    swd = NULL;
}

/* needed for 6.12 kernel */
static ssize_t swd_write(struct tty_struct *tty, struct file *file,
                         const u8 *buf, size_t count)
{
    return count;
}

/* ==================== Receive Serial Data ==================== */
static void swd_receive_buf(struct tty_struct *tty, const u8 *cp,
                            const u8 *fp, size_t count)
{
    struct swd_dev *s = tty->disc_data;
    size_t i;

    if (!s) return;

    spin_lock(&s->lock);

    for (i = 0; i < count; ++i) {
        char c = cp[i];

        if (c == '\r')
            continue;

        if (c != '\n') {
            if (s->buf_len < sizeof(s->buf) - 1)
                s->buf[s->buf_len++] = c;
        } else {
            s->buf[s->buf_len] = '\0';

            if (s->buf_len >= 2)     // e.g. "W1"
                swd_parse_line(s, s->buf);

            s->buf_len = 0;
        }
    }

    spin_unlock(&s->lock);
}

/* ==================== Register LDISC ==================== */
static struct tty_ldisc_ops swd_ldisc = {
    .owner = THIS_MODULE,
    .name = "swd_ldisc",
    .num  = MY_LDISC,
    .open = swd_open,
    .close = swd_close,
    .write = swd_write,
    .receive_buf = swd_receive_buf,
};

static int __init swd_init(void)
{
    int ret = tty_register_ldisc(&swd_ldisc);
    if (ret) {
        pr_err("swd: failed to register ldisc %d: %d\n", MY_LDISC, ret);
        return ret;
    }
    pr_info("swd: ldisc %d registered\n", MY_LDISC);
    return 0;
}

static void __exit swd_exit(void)
{
    tty_unregister_ldisc(&swd_ldisc);
    pr_info("swd: exit\n");
}

module_init(swd_init);
module_exit(swd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Serial WASD Line Discipline Driver");
