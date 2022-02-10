MPICXX = mpic++
SRC= $(wildcard *.cpp)
OBJ= $(SRC:.cpp=.o)
EXEC= Hello

%.o :%.cpp
	$(MPICXX) -o $@ -c $< -fopenmp

Hello : $(OBJ) 
	$(MPICXX) -o $@ $^ -fopenmp

.PHONY: clean mrproper

clean:
	@rm -rf *.o

mrproper: clean
	@rm -rf $(EXEC)
