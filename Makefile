all:
	g++ -o sudoku sudoku.cxx

debug:
	g++ -o sudoku -g sudoku.cxx

clean:
	rm sudoku
