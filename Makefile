CPP      := g++
CPPFLAGS := -std=c++11 -O2 -c
LFLAGS   := -lz -s
REMOVE   := rm

OBJ      := main.o Atlas.o Targa.o
HEADER   := Atlas.h Texture.h Targa.h

atlas: $(OBJ)
	$(CPP) -o $@ $(OBJ) $(LFLAGS)

%.o:%.cpp Atlas.h $(HEADER)
	$(CPP) $< $(CPPFLAGS)

clean:
	$(REMOVE) $(OBJ)

.PHONY: clean
