CC = gcc

CFLAGS = -Wall -Wextra -O2

TARGET = lora_test_receive

SRCS = lora_test.c lora_api.c micro_aes.c

OBJS = $(SRCS:.c=.o)

DEPS = lora_api.h micro_aes.h

all: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean