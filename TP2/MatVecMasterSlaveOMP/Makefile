MPICXX = mpic++
SRC= $(wildcard *.cpp)
OBJ= $(SRC:.cpp=.o)


all: maitre esclave

%.o: %.cpp
	$(MPICXX) -o $@ -c $< 

maitre: maitre.o fonctions.o
	$(MPICXX) -o $@ $^

esclave: esclave.o fonctions.o
	$(MPICXX) -o $@ $^

clean:
	@rm -rf *.o

cleanall: clean
	@rm -rf maitre esclave
