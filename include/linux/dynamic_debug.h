#ifndef _DYNAMIC_DEBUG_H
#define _DYNAMIC_DEBUG_H

/*
 * An instance of this structure is created in a special
 * ELF section at every dynamic debug callsite.  At runtime,
 * the special section is treated as an array of these.
 */
struct _ddebug {
	/*
	 * These fields are used to drive the user interface
	 * for selecting and displaying debug callsites.
	 */
	const char *modname;
	const char *function;
	const char *filename;
	const char *format;
	unsigned int lineno:24;
	/*
 	 * The flags field controls the behaviour at the callsite.
 	 * The bits here are changed dynamically when the user
	 * writes commands to <debugfs>/dynamic_debug/control
	 */
#define _DPRINTK_FLAGS_PRINT   (1<<0)  /* printk() a message using the format */
#define _DPRINTK_FLAGS_INCL_MODNAME	(1<<1)
#define _DPRINTK_FLAGS_INCL_FUNCNAME	(1<<2)
#define _DPRINTK_FLAGS_INCL_LINENO	(1<<3)
#define _DPRINTK_FLAGS_INCL_TID		(1<<4)
#define _DPRINTK_FLAGS_DEFAULT 0
	unsigned int flags:8;
	char enabled;
} __attribute__((aligned(8)));


int ddebug_add_module(struct _ddebug *tab, unsigned int n,
				const char *modname);

#if defined(CONFIG_DYNAMIC_DEBUG)
extern int ddebug_remove_module(const char *mod_name);
extern int __dynamic_pr_debug(struct _ddebug *descriptor, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));

struct device;

extern int __dynamic_dev_dbg(struct _ddebug *descriptor,
			     const struct device *dev,
			     const char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));

struct net_device;

extern int __dynamic_netdev_dbg(struct _ddebug *descriptor,
			     const struct net_device *dev,
			     const char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));

#define DEFINE_DYNAMIC_DEBUG_METADATA(name, fmt)		\
	static struct _ddebug __used __aligned(8)		\
	__attribute__((section("__verbose"))) name = {		\
		.modname = KBUILD_MODNAME,			\
		.function = __func__,				\
		.filename = __FILE__,				\
		.format = (fmt),				\
		.lineno = __LINE__,				\
		.flags =  _DPRINTK_FLAGS_DEFAULT,		\
		.enabled = false,				\
	}

#define dynamic_pr_debug(fmt, ...)				\
do {								\
	DEFINE_DYNAMIC_DEBUG_METADATA(descriptor, fmt);		\
	if (unlikely(descriptor.enabled))			\
		__dynamic_pr_debug(&descriptor, pr_fmt(fmt),	\
				   ##__VA_ARGS__);		\
} while (0)

#define dynamic_dev_dbg(dev, fmt, ...)				\
do {								\
	DEFINE_DYNAMIC_DEBUG_METADATA(descriptor, fmt);	\
	if (unlikely(descriptor.enabled))			\
		__dynamic_dev_dbg(&descriptor, dev, fmt,	\
				  ##__VA_ARGS__);		\
} while (0)

#define dynamic_netdev_dbg(dev, fmt, ...)			\
do {								\
	DEFINE_DYNAMIC_DEBUG_METADATA(descriptor, fmt);		\
	if (unlikely(descriptor.enabled))			\
		__dynamic_netdev_dbg(&descriptor, dev, fmt,	\
				     ##__VA_ARGS__);		\
} while (0)

#else

static inline int ddebug_remove_module(const char *mod)
{
	return 0;
}

#define dynamic_pr_debug(fmt, ...)					\
	do { if (0) printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__); } while (0)
#define dynamic_dev_dbg(dev, fmt, ...)					\
	do { if (0) dev_printk(KERN_DEBUG, dev, fmt, ##__VA_ARGS__); } while (0)
#endif

#endif
