#HEADERS = buffer.h
OBJECTS = buffer.o

default: buffer

%.o: %.c
#$(HEADERS)
	gcc -c $< -o $@ -lm

buffer: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -lm

clean:
	-rm -f $(OBJECTS)
	-rm -f buffer
