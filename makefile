#HEADERS = buffer.h
OBJECTS = buffer.o

default: buffer

%.o: %.c
#$(HEADERS)
	gcc -c $< -o $@

buffer: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f buffer
