CC = gcc
LDFLAGS = -L/usr/local/lib -lfltk -lstdc++ -lXext -lX11 -lm
CLFAGS = -pthread
SRC = src/Box.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = program

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(CFLAGS)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

