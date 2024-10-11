TARGET = battery_icon
OBJS = main.o exports.o include/draw.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX = 1

# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

INCDIR = include
CFLAGS = -O2 -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpsppower libs/libpspsystemctrl_user.a #libs/libpspsystemctrl_kernel.a libs/libpspkubridge.a libs/libpspuart.a
LDFLAGS = -nostartfiles
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak