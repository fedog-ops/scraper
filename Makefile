CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lcurl -lxml2

SRCS = scraper.c
OBJS = $(SRCS:.c=.o)
EXECUTABLE = executable

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
    $(CC) $(CFLAGS) $(OBJS) -o $(EXECUTABLE) $(LIBS)

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(OBJS) $(EXECUTABLE)