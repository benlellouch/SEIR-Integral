set datafile separator ','
set key autotitle columnhead
plot 'output.csv' using 1:2 with lines, 'output.csv' using 1:3 with lines, 'output.csv' using 1:4 with lines