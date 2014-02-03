#!/bin/bash
for file in examples/example*; do
  ./sudoku < $file;
  echo --;
done
