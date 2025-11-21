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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("WASD Serial Line Discipline");

struct swd_dev {
    struct tty_struct *tty;
    struct input_dev *input;
    spinlock_t lock;
    char buf[64];
    int buf_len;
};

/* ==================== Key Event Parser ==================== */
static void swd_parse_line(struct swd_dev *swd, const char *line)
{
    if (!swd || !swd->input) return;

    // Ignore the heartbeat dot
    if (line[0] == '.') return;

    char key = line[0];
    char val = line[1]; 
    int press = (val == '1');
    unsigned int code = 0;

    switch (key) {
        case 'W': code = KEY_W; break;
        case 'A': code = KEY_A; break;
        case 'S': code = KEY_S; break;
        case 'D': code = KEY_D; break;
        default: return;
    }
    input_report_key(swd->input, code, press);
    input_sync(swd->input);
}

/* ==================== LDISC Open ==================== */
static int swd_open(struct tty_struct *tty)
{
    struct swd_dev *swd;
    struct ktermios old_termios;
    int error;

    swd = kzalloc(sizeof(*swd), GFP_KERNEL);
    if (!swd) return -ENOMEM;

    spin_lock_init(&swd->lock);
    swd->tty = tty;

    swd->input = input_allocate_device();
    if (!swd->input) {
        kfree(swd);
        return -ENOMEM;
    }

    swd->input->name = "serial-wasd";
    swd->input->id.bustype = BUS_RS232;
    __set_bit(EV_KEY, swd->input->evbit);
    __set_bit(KEY_W, swd->input->keybit);
    __set_bit(KEY_A, swd->input->keybit);
    __set_bit(KEY_S, swd->input->keybit);
    __set_bit(KEY_D, swd->input->keybit);

    error = input_register_device(swd->input);
    if (error) {
        input_free_device(swd->input);
        kfree(swd);
        return error;
    }

    tty->disc_data = swd;
    
    // --- THE MISSING LINE ---
    // Tell the TTY layer we can accept data!
    tty->receive_room = 65536; 

    // --- FORCE HARDWARE SETTINGS ---
    old_termios = tty->termios;
    tty->termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty->termios.c_oflag &= ~OPOST;
    tty->termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty->termios.c_cflag &= ~(CSIZE | PARENB);
    tty->termios.c_cflag |= (CS8 | CLOCAL | CREAD);
    
    tty_set_termios(tty, &old_termios);

    pr_info("swd: open successful. cflag=0x%x\n", tty->termios.c_cflag);
    return 0;
}

/* ==================== LDISC Close ==================== */
static void swd_close(struct tty_struct *tty)
{
    // Retrieve the structure from the TTY
    struct swd_dev *swd = (struct swd_dev *)tty->disc_data;

    if (swd) {
        if (swd->input)
            input_unregister_device(swd->input);
        kfree(swd);
    }
    
    tty->disc_data = NULL; // Prevent use-after-free
    pr_info("swd: close\n");
}

/* ==================== Receive Serial Data ==================== */
static void swd_receive_buf(struct tty_struct *tty, const u8 *cp, const u8 *fp, size_t count)
{
    struct swd_dev *swd = (struct swd_dev *)tty->disc_data;
    size_t i;

    // Safety check: If data comes in after close, ignore it
    if (!swd) return;

    spin_lock(&swd->lock);
    for (i = 0; i < count; ++i) {
        char c = cp[i];
        
        // DEBUG PRINT: Show every character received
        pr_info("swd: recv '%c' (0x%02x)\n", (c >= 32 && c <= 126) ? c : '.', c);

        if (c == '\r') continue;
        if (c != '\n') {
            if (swd->buf_len < sizeof(swd->buf) - 1)
                swd->buf[swd->buf_len++] = c;
        } else {
            swd->buf[swd->buf_len] = '\0';
            if (swd->buf_len >= 1) swd_parse_line(swd, swd->buf);
            swd->buf_len = 0;
        }
    }
    spin_unlock(&swd->lock);
}

/* ==================== Register LDISC ==================== */
static struct tty_ldisc_ops swd_ldisc = {
    .owner = THIS_MODULE,
    .num = MY_LDISC,
    .name = "serial-wasd",
    .open = swd_open,
    .close = swd_close,
    .receive_buf = swd_receive_buf,
};

static int __init swd_init(void)
{
    // NEW API: Pass only the struct pointer
    int ret = tty_register_ldisc(&swd_ldisc);
    
    if (ret) {
        pr_err("swd: failed to register ldisc (%d)\n", ret);
        return ret;
    }
    pr_info("swd: ldisc %d registered\n", MY_LDISC);
    return 0;
}

static void __exit swd_exit(void)
{
    // NEW API: Pass only the struct pointer
    tty_unregister_ldisc(&swd_ldisc);
    pr_info("swd: exit\n");
}

module_init(swd_init);
module_exit(swd_exit);
