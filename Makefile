CPP := g++
CPPFLAGS := -g -std=c++11 -c
LFLAGS :=
REMOVE := rm

OBJ := main.o Atlas.o Targa.o
HEADER := Atlas.h Texture.h Targa.h

atlas: $(OBJ)
	$(CPP) -o $@ $(OBJ) $(LFLAGS)

%.o:%.cpp Atlas.h $(HEADER)
	$(CPP) $(CPPFLAGS) $<

clean:
	$(REMOVE) $(OBJ)

.PHONY: clean
