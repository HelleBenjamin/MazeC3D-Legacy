#all:
#	gcc -o main src/main.c src/glad.c -lGL -lglfw -Iinclude -lm

CC = gcc
CFLAGS = -Wall -lm -Iinclude

ifeq ($(OS), Windows_NT)
  LIBS = -Lwin32 -lglfw3 -lopengl32 -lgdi32 -Lwin32
  INCLUDES = -Iwin32
else
  LIBS = -Llinux -lglfw -lGL
  INCLUDES = -Ilinux
endif

all:
	$(CC) $(INCLUDES) src/main.c src/glad.c -o main $(LIBS) $(CFLAGS)
