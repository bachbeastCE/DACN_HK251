# ===== Compiler =====
CC = gcc
CXX = g++

# ===== Flags =====
CFLAGS = -Wall -Wextra -O2
CXXFLAGS = -Wall -O2

# ===== Libs =====
LIBS = -lboost_system -lpthread -lcurl

# ===== Target =====
TARGET = firebase

# ===== Source =====
C_SRCS = lora_api.c micro_aes.c
CPP_SRCS = main.cpp

SRCS = $(C_SRCS) $(CPP_SRCS)

# ===== Object =====
OBJS = $(C_SRCS:.c=.o) $(CPP_SRCS:.cpp=.o)

# ===== Build =====
all: $(TARGET)

# compile C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# compile C++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# link
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

# run
run: all
	@echo "Killing old process..."
	fuser -k 9001/tcp 2>/dev/null || true
	@echo "Starting app..."
	sudo ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run

#curl -X DELETE "https://binnocular-default-rtdb.asia-southeast1.firebasedatabase.app/tracking_history.json"
#ls /proc/129913/task | wc -l
#ps aux | grep firebase