CC = gcc
LDFLAGS = -L/usr/local/lib -lfltk -lstdc++ -lXext -lX11 -lm

SRC = src/Box.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = program

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

