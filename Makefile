CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread
LDFLAGS = -pthread -lrt

TARGET = campus_compute
SRCS = main.c scheduler.c logger.c ipc.c banker.c process_mgmt.c
OBJS = $(SRCS:.c=.o)
LOG_FILE = campus_compute.log
ZIP_NAME = $(SRN)_CampusCompute.zip

.PHONY: all clean run zip help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c campus_compute.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(LOG_FILE) $(ZIP_NAME)
	@# Remove any residual POSIX shared memory or semaphores
	rm -f /dev/shm/campus_compute_* 2>/dev/null || true

run: $(TARGET)
	./$(TARGET)

zip: clean
	@if [ -z "$(SRN)" ]; then \
		echo "[ERROR] SRN environment variable is not set. Run: export SRN=YOUR_SRN"; \
		exit 1; \
	fi
	zip -r $(ZIP_NAME) *.c *.h Makefile README.md
	@echo "Created submission archive: $(ZIP_NAME)"

help:
	@echo "Available targets:"
	@echo "  all     : Build the campus_compute executable"
	@echo "  clean   : Remove binary, object files, and logs"
	@echo "  run     : Build and start interactive CLI"
	@echo "  zip     : Create assignment submission ZIP using \$$SRN"