CC = g++
OBJ = sai.o
TARGET = SAI
EMAIL = matthew.l.allen@gmail.com

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	g++ -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(TARGET) 

run: $(TARGET)
	./$(TARGET)

mail:
	mail -s "SAI source" "$(EMAIL)" < sai.cpp
	mail -s "SAI makefile" "$(EMAIL)" < makefile

gitadd:
	git add sai.cpp
