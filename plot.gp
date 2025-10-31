set encoding utf8
set datafile separator ";"

# this works, but print image on top left connner, and rewrite all content in terminal.
# set term sixel enhanced font "Arial,14"
set terminal pngcairo size 1600,900 enhanced font "Arial,14"
set output "result.png"

set title "function 'sort' results"
set xlabel ""
set ylabel "time (ms)"

set yrange [0:*]

set style data histogram
set style histogram clustered gap 1
set style fill solid 0.8 border -1
set boxwidth 0.8

# set logscale y

set style line 1 lc rgb "#666666" lt 1 lw 1
set grid ytics linestyle 1

set key outside right top vertical

set xtics rotate by -90
# set xtic rotate by 0 scale 0

plot 'result.dat' using 2:xtic(1) title columnheader(2), \
                ''         using 3        title columnheader(3), \
                ''         using 4        title columnheader(4), \
                ''         using 5        title columnheader(5), \
                ''         using 6        title columnheader(6), \
                ''         using 7        title columnheader(7), \
                ''         using 8        title columnheader(8), \
                ''         using 9        title columnheader(9), \
                ''         using 10        title columnheader(10), \
                ''         using 11        title columnheader(11), \
                ''         using 12        title columnheader(12), \
                ''         using 13        title columnheader(13), \
                ''         using 14        title columnheader(14), \
                ''         using 15        title columnheader(15), \
                ''         using 16        title columnheader(16), \
                ''         using 17        title columnheader(17), \
                ''         using 18        title columnheader(18), \
                ''         using 19        title columnheader(19)
                
set key off
set format y "%g"
ncols = 2
delta = 0.18
